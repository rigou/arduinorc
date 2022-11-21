/* libserial.h - Open, read, and write from serial port in C
** inspired by http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
** 30-04-2013

Copyright (C) 2014 Richard Goutorbe.  All right reserved. Email arduinorc at gmail dot com .
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LibSerial_h
#define LibSerial_h

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#define SERIAL_READ_BLOCKING 1
#define SERIAL_READ_NONBLOCKING 0

int serialInitCom(int fh, int speed, int parity);
int serialReadMode(int fh, int should_block);
int serialReadLine(int fh, char *out_buff_str, int bufflen_int, const char eol_char);
int serialReadLine(int fh, char *out_buff_str, int bufflen_int, const char eol_char, unsigned int timeout_int);
int serialWrite(int fh, const char *buff_str, int nchars_int);

#endif
