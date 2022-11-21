/* txupload.cpp - Upload a settings file to the Tx
** 30-04-2013
** 03-05-2013 ignore comments in source file
** 08-05-2013 usleep() to prevent Arduino serial input buffer overflow
** 18-06-2013 ignore final \r in source file, if any
** 29-06-2013 added optional arg to resume upload at line number
** 04-07-2013 comments start by '#' instead of ';'
** 30-05-2014 v1.4 lowered speed to 2400 bps to prevent Arduino's buffer overflow

Copyright (C) 2014 Richard Goutorbe.  All right reserved. Email arduinorc at gmail dot com .
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "libserial.h"

#define CMDLINESIZE 32 // same value as in ../arduinotx/arduinotx_command.h
#define REPLYLINESIZE 256 // large enough to store the Tx initialization messages
#define TXUPLOADVERSION "1.4"
#define COMMENT_TOKEN '#' // comments start by '#'

const char CMDEOL = '\n'; // input command line terminator

// Test if given null-terminated string is empty or contains only white-space characters
// return value: 1=string is empty or blank, 0=string contains non-blank characters
int is_blank(const char *line_str) {
	int retval_int = 1;
	const char *c_ptr = line_str;
	while (*c_ptr && retval_int) {
		if (*c_ptr ==' ' || *c_ptr == '\t' || *c_ptr == '\n' || *c_ptr == '\r')
			c_ptr++;
		else
			retval_int = 0;
	}
	return retval_int;
}

// Read and discard input until the Tx prompt "> " is received
// Return value: 0=0k, not zero on timeout or error
int read_prompt(int fhtrg, char *trgbuff_str) {
	int retval_int = 0;
	if (serialReadLine(fhtrg, trgbuff_str, REPLYLINESIZE, '>', 10) > 0)
		read(fhtrg, trgbuff_str, 1); // ignore the space after the '>'
	else
		retval_int = 1;
	return retval_int;
}

int send_command(int fhtrg, char *command_str, char *out_reply_str, const char *expected_str, int echo_int) {
	int retval_int = 0;
	int srclen_int = strlen(command_str);
	int explen_int = expected_str ? strlen(expected_str) : 0;
	
	// truncate command to CMDLINESIZE
	if (srclen_int >= CMDLINESIZE) {
		command_str[CMDLINESIZE - 2] = CMDEOL;
		command_str[CMDLINESIZE - 1] = '\0';
		srclen_int = CMDLINESIZE - 1;
	}
	// Send command to Tx
	if (echo_int & 0x1)
		printf("> %s", command_str);
	if (serialWrite(fhtrg, command_str, srclen_int) == 0) {
		// Read reply from Tx
		int readlen_int = 0;
		readlen_int = serialReadLine(fhtrg, out_reply_str, REPLYLINESIZE, CMDEOL, 1);
		if (readlen_int >= 0) {
			if (echo_int & 0x2)
				printf("< %s\n", readlen_int > 0 ? out_reply_str : "timeout"); 
			if (explen_int && strncmp(out_reply_str, expected_str, explen_int))
				retval_int = 1; // we received something but not the expected response
			else if (strstr(out_reply_str, "error"))
				retval_int = 1; // Tx replied "error" to invalid command
			read_prompt(fhtrg, out_reply_str); // discard following chars until next prompt
		}
		else
			retval_int = 2; // serial read error
	}
	else
		retval_int = 3; // serial write error
	return retval_int;
}
					
int main(int argc, char** argv) {
	int retval_int = 0;
	if (argc == 3 || argc == 4) {
		
		/*
		** Initialization **************************
		*/

		// Open settings file for reading
		size_t srcbufflen_int = CMDLINESIZE;
		char *srcbuff_str = (char *)malloc(srcbufflen_int);
		char *srcfile = argv[1];
		FILE * fsrc = fopen(srcfile, "r");
		if (fsrc == NULL) {
			printf("error %d opening %s: %s\n", errno, srcfile, strerror(errno));
			return 2;
		}
		
		// Open serial port for read/write
		printf("Connecting\n");
		char *trgbuff_str = (char *)malloc(REPLYLINESIZE);
		char *trgport = argv[2];
		int fhtrg = open(trgport, O_RDWR | O_NOCTTY | O_SYNC);
		if (fhtrg < 0) {
			printf("error %d opening %s: %s\n", errno, trgport, strerror(errno));
			free(srcbuff_str);
			fclose(fsrc);
			return 3;
		}
		serialInitCom(fhtrg, B2400, 0);	// set speed to 2400 bps, 8n1 (no parity)
		
		/*
		** Handshake **************************
		*/
		unsigned int senderror_int = 0;
		unsigned short started_byt = 0;
		
		// send a \n and wait for the Tx prompt "> ", discarding all Tx initialization messages
		sprintf(srcbuff_str, "%c", CMDEOL);
		if (serialWrite(fhtrg, srcbuff_str, 1) == 0)
			read_prompt(fhtrg, trgbuff_str);
		else
			senderror_int = 1; // quit on i/o error writing to serial port
		
		if (! senderror_int) {
			// send ECHO UPLOAD commands until we receive the "UPLOAD" reply
			sprintf(srcbuff_str, "ECHO UPLOAD%c", CMDEOL);
			do {
				senderror_int = send_command(fhtrg, srcbuff_str, trgbuff_str, "UPLOAD", 0);
				if (senderror_int == 0)
					started_byt = 1;
				else if (senderror_int == 2 || senderror_int == 3)
					break ; // cancel handshake on i/o error reading or writing to serial port
				// else we received something but not the expected response: try again
			} while (! started_byt);

			if (started_byt) {
				/*
				** Upload file to Tx **************************
				*/
				printf("Connected\n");
				int linenum_int = 0;
				int nread_int = 0;
				int begin_linenum_int = 1;
				if (argc == 4)
					begin_linenum_int = atoi(argv[3]);
				
				// for each line in srcfile
				do {
					// read entire source line up to the terminating \n included
					nread_int = getline(&srcbuff_str, &srcbufflen_int, fsrc);
					if (nread_int != -1) { // else EOF
						// ignore the final \r, if any
						if (nread_int > 1) {
							if (*(srcbuff_str + nread_int - 2) == '\r') {
								*(srcbuff_str + nread_int - 2) = '\n';
								*(srcbuff_str + nread_int - 1) = '\0';
								--nread_int;
							}
						}
						char *comment_chr = strchr(srcbuff_str, COMMENT_TOKEN); // strip comments
						if (comment_chr) {
							*comment_chr = CMDEOL;
							*(comment_chr + 1) = '\0';
						}
						linenum_int++;
						if (! is_blank(srcbuff_str)) { // else ignore blank line
							if (linenum_int >= begin_linenum_int) { // else skip lines before begin_linenum_int
								senderror_int = send_command(fhtrg, srcbuff_str, trgbuff_str, NULL, 2);
								usleep(50000L); // 50ms
							}
						}
					}
				} while (nread_int != -1 && senderror_int == 0) ; // until EOF or i/o error
				retval_int += senderror_int;
				if (senderror_int == 1)
					printf("*** Error at line %d : %s\n", linenum_int, srcbuff_str);
				
				// restore interactive echo mode
				sprintf(srcbuff_str, "ECHO ON%c", CMDEOL);
				senderror_int = send_command(fhtrg, srcbuff_str, trgbuff_str, "ON", 0);
				retval_int += senderror_int;
			}
			else {
				printf("Connection failed\n");
				retval_int += senderror_int;
			}
		}
		else
			retval_int += senderror_int;
		
		free(srcbuff_str);
		free(trgbuff_str);
		fclose(fsrc);
		close(fhtrg);
	}
	else {
		printf("%s version %s\n", argv[0], TXUPLOADVERSION);
		printf("usage: %s settings.txt /dev/ttyxxxx [line#]\n", argv[0]);
		printf("send given settings file to given serial port, starting at given line number\n");
		retval_int = 1;
	}
	printf("upload %s\n", retval_int==0 ? "complete" : "failed");
	return retval_int;
}

