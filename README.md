# ArduinoRC README
Version v1.6.1 published Nov 21, 2022  
https://arduinorc.reseau.org/

## COPYRIGHT: 

Copyright (C) Richard Goutorbe.  All right reserved.  
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.  
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.  
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact

## NO WARRANTY, NO LIABILITY:

You acknowledge and agree that the access and use of the arduinorc software and its documentation is at your sole risk and discretion. The author will not be held responsible for any damages or harm to any device, loss of data or other harm that results from your use of the arduinorc software and its documentation. The arduinorc software and its documentation is provided "as is", without any warranties of any kind, whether express, implied, statutory or otherwise. To the maximum extent permitted under applicable law, the author expressly disclaim all warranties, including but not limited to the implied warranties of merchantability, fitness for a particular purpose, and non-infringement. The author does not represent or warrant that the arduinorc software and its documentation will meet your requirements, or that the operation of the arduinorc software will be uninterrupted or error-free, or that defects in the arduinorc software will be corrected. Furthermore, the author does not warrant or make any representations regarding the use, or the results of the use of the arduinorc software and its documentation in terms of its correctness, accuracy, reliability, or otherwise. No oral or written information or advice given by the author or its authorized representatives shall create any representation or warranty. In no event shall the author be liable to you for any damages whatsoever in connection with the usage of the arduinorc software and its documentation, whether arising from loss of profits, revenues, loss or accuracy of information, or for any indirect, special, consequential, punative or incidental damages even if advised of the possibility thereof.

## PROJECT INFORMATION:

With this software you can build a multi-channel (6 channels by default, up to 9 channels) RC transmitter.

Visit the ArduinoRC project's website for more information: https://arduinorc.reseau.org/

## FEATURES:

* Programmable with Linux or Windows via USB
* up to 9 proportional channels, 6 channels by default 
* Channels controlled by a potentiometer or by a switch
* 9 model memories
* Model selection switch
* 2 Programmable mixers
* Dual rate/Exponential switch 
* Throttle cut switch
* End point adjustment
* Subtrims
* Potentiometers and servos calibration 
* Throttle security check at startup
* Transmitter battery low voltage alarm
* Arduino board with ATmega328 microcontroller (Nano v3.0 recommended)

## FILES:

./arduinotx/arduinotx.ino
	The main sketch

./arduinotx/arduinotx_buzz.cpp
	This module manages the piezzo buzzer

./arduinotx/arduinotx_command.cpp
	This module implements the command interpreter

/arduinotx/arduinotx_config.h 
	This file contains all user-customizable settings

./arduinotx/arduinotx_eeprom.cpp
	This module manages the persistency of the transmitter settings in the Arduino's EEPROM.

./arduinotx/arduinotx_led.cpp
	This module manages the flashing Led (run modes, alarms)

./arduinotx/arduinotx_lib.cpp
	This module contains miscellaneous string and i/o functions

./arduinotx/arduinotx_transmitter.cpp
	This module manages the transmitter

./linux/txupload
	A Linux utility for uploading a settings file to the transmitter
