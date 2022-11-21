/* libserial.cpp - Open, read, and write from serial port in C
** inspired by http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
** 30-04-2013

Copyright (C) 2014 Richard Goutorbe.  All right reserved. Email arduinorc at gmail dot com .
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "libserial.h"
#include <sys/time.h>

//#define SERIAL_DEBUG

int serialReadBlockingMode_int = SERIAL_READ_NONBLOCKING; // default read mode

/* Set serial link settings
** The values for speed are B115200, B230400, B9600, B19200, B38400, B57600, B1200, B2400, B4800, etc. 
** The values for parity are 0 (meaning no parity), PARENB|PARODD (enable parity and use odd), 
** 	PARENB (enable parity and use even), PARENB|PARODD|CMSPAR (mark parity), and PARENB|CMSPAR (space parity).
**	CMSPAR is needed only for choosing mark and space parity, which is uncommon. For most applications, it can be omitted.
*/
int serialInitCom(int fh, int speed, int parity) {
        struct termios tty;
        memset(&tty, 0, sizeof tty);
        if (tcgetattr(fh, &tty) != 0) {
                printf("error %d from tcgetattr\n", errno);
                return -1;
        }
        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break as \000 chars
        tty.c_iflag &= ~IGNBRK;	// ignore break signal
        tty.c_lflag = 0;			// no signaling chars, no echo, no canonical processing
        tty.c_oflag = 0;		// no remapping, no delays
	
        tty.c_cc[VMIN]  = 0;		// read doesn't block
        tty.c_cc[VTIME] = 5;	// 0.5 seconds read timeout
	serialReadBlockingMode_int = SERIAL_READ_NONBLOCKING;
	
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD); 		// ignore modem controls, enable reading
        tty.c_cflag &= ~(PARENB | PARODD);	// shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr(fh, TCSANOW, &tty) != 0) {
                printf("error %d from tcsetattr\n", errno);
                return -1;
        }
        return 0;
}

// Set read blocking mode
// should_block : 1=blocking  0=non-blocking with 500ms timeout
// return value: 0=Ok, -1 on error
int serialReadMode(int fh, int block_int) {
	int retval_int = 0;
	if (block_int != serialReadBlockingMode_int) {
		struct termios tty;
		memset(&tty, 0, sizeof tty);
		if (tcgetattr(fh, &tty) == 0) {
			tty.c_cc[VMIN]  = block_int ? 1 : 0;
			tty.c_cc[VTIME] = 5;	// 0.5 seconds read timeout
			if (tcsetattr(fh, TCSANOW, &tty) == 0)
				serialReadBlockingMode_int = block_int;
			else {
				printf("error %d setting term attributes\n", errno);
				retval_int = -1;
			}
		}
		else {
			printf("error %d from tggetattr\n", errno);
			retval_int = -1;
		}
	}
	return retval_int;
}

// Read a line from Serial until the terminating eol_char is read or buffer is full or until timeout, whichever comes first
// Returned out_buff_str does not include the terminating eol_char in the returned buffer
// Arg timeout_int : timeout in seconds
// Return value: length of the null-terminated string returned in buff_str, terminating eol_char NOT included,  -1 on error
int serialReadLine(int fh, char *out_buff_str, int bufflen_int, const char eol_char, unsigned int timeout_int) {
	int retval_int = 0;
	unsigned short eol_byt = 0;
	
	serialReadMode(fh, SERIAL_READ_NONBLOCKING);	// subsequent read() calls will timeout after 0.5s
	
	struct timeval  time_struct;
	gettimeofday(&time_struct, NULL);
	time_t stop_tim = time_struct.tv_sec + timeout_int;
	
	// Repeat until the terminating eol_char is read or the buffer is full or the desired timeout_int is reached
	do {
		int nread_int = read(fh, out_buff_str + retval_int, 1);
		if (nread_int == 1) {
			if (out_buff_str[retval_int] == eol_char) {
				out_buff_str[retval_int] = '\0';
				eol_byt = 1;
			}
			else
				retval_int++;
		}
		else if (nread_int == -1) {
			printf("error %d reading serial: %s\n", errno, strerror(errno));
			retval_int = -1;
			break;
		}
		gettimeofday(&time_struct, NULL);
	} while (retval_int < bufflen_int - 1 && time_struct.tv_sec <= stop_tim && eol_byt == 0);
	
	if (eol_byt == 0 && retval_int != -1) // exited after timeout or out_buff_str full
		out_buff_str[retval_int] = '\0';
#ifdef SERIAL_DEBUG
	printf("serialReadLine(%ds) returns %d, \"%s\"\n", timeout_int, retval_int, out_buff_str);
#endif
	return retval_int;
}

// Read a line from Serial, block until the terminating eol_char is read or buffer is full 
// do not include the terminating eol_char in the returned buffer
// Return value: length of the null-terminated string returned in buff_str, terminating eol_char NOT included, 0 on EOF, -1 on error
int serialReadLine(int fh, char *out_buff_str, int bufflen_int, const char eol_char) {
	int retval_int = 0;
	unsigned short eof_byt = 0;
	unsigned short eol_byt = 0;
	unsigned short ioerr_byt = 0;

	serialReadMode(fh, SERIAL_READ_BLOCKING);
	
	do {
		int nread_int = read(fh, out_buff_str + retval_int, 1);
		switch (nread_int) {
			case 1:
				if (out_buff_str[retval_int] == eol_char) {
					out_buff_str[retval_int] = '\0';
					eol_byt = 1; // this will not happen if out_buff_str gets full before reading the eol
				}
				else
					retval_int++;
				break;
			case 0:
				eof_byt = 1;
				break;
			case -1:
				printf("error %d reading serial: %s\n", errno, strerror(errno));
				ioerr_byt = 1;
				retval_int = -1;
				break;
		}
	} while (retval_int < bufflen_int - 1 && eof_byt == 0 && eol_byt == 0 && ioerr_byt == 0);
	
	if (ioerr_byt == 0 && (eof_byt == 1 || eol_byt == 0)) // exited because EOF or out_buff_str full
		out_buff_str[retval_int] = '\0';
	
#ifdef SERIAL_DEBUG
	printf("serialReadLine() returns %d, \"%s\"\n", retval_int, out_buff_str);
#endif
	return retval_int;
}

// Write a string to Serial
// Return value: 0=Ok, not 0 on error
int serialWrite(int fh, const char *buff_str, int nchars_int) {
	int  retval_int = 0;
#ifdef SERIAL_DEBUG	
	printf("serialWrite(\"%s\", %d)\n", buff_str, nchars_int);
#endif
	if (write(fh, buff_str, nchars_int) != nchars_int) {
		printf("error %d writing serial: %s\n", errno, strerror(errno));
		retval_int = 1;
	}
	return retval_int;
}

