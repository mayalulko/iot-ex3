# IoT-ex3
## Authors
Oriyan Hermoni oriyan.hermoni@mail.huji.ac.il , 302170204<br> 
Maya Lulko maya.lulko@mail.huji.ac.il , 312414089

## Package contents:
* CMakeLists.txt - CMake file
* serial_io_linux.c, serial_io.h - manages the connection API to serial port.
* main.c - Entry point to program
* config.h - parameters configuration file
* Makefile
* README.md

## Build instructions
### Requirements
* Linux distro (tested on Ubuntu Simplicity)
* gcc compiler v7.5.0 (with support for C standard gnu99)
* tar - for creating tar archive from files
* CMake if building with CLion
* make

### Compilation
There are two options:
1. Build with CMake: <br>
<t>CMakeLists.txt is included, open the project dir as a project in CLion and load this file
2. Build with make:<br>
<t>run `make` , `make all` to generate an executable `main`<br>
<t>run `make main` to only generate object files (.o)<br>
<t>run `make clean` to clean all object files and executables<br>
<t>run `make tar` to create tar archive


### Run
After compiling the project, simply execute `main`.

## Design
The base of our design is the serial_io_linux.c that contains the API of the serial port connection to the modem - 
including SerialInit, SerialRecv, SerialSent, SerialFlushInputBuff and SerialDisable. 
In main.c we initialize the serial port. After the connection is established (getting +PBREADY), we write a few commands
such as AT, AT+CREG?, AT+COPS?, AT+CCID, AT+COPS=?. After each command, we check if we got 'OK' as a sign that the 
command reached its destination and got an expected response. 
We will point out that we are trying to read the response for several attempts by our configurable "MAX_NUMBER_ATTEMPTS"
that you can find in the config.h file.
Note - we assume that the modem is connected via cable and activated after the start of the program.

To avoid doing heap allocation, we statically allocated the buffers at compile time. We allocated three buffers:
1. REQUEST_BUFFER - 4kB, re-using this buffer for sending AT commands on Tx to the modem.
2. RESPONSE_BUFFER - 4kB, re-using this buffer to store the entire response on Rx from the modem.


All of these are more than enough space for the expected response data.

## Application Flow
1. Initializing Serial port connection via SerialInit 
2. Waiting for +PBREADY to indicate that the modem is ready to receive commands. 
3. Sending AT command.
4. Parsing the body of the response (looking for the "OK" and returning the body of the response)
5. Sending AT+CCID command.
6. Parsing the body of the response (looking for the "OK" and returning the body of the response). 
7. Sending AT+CREG? command.
8. Parsing the body of the response (looking for the "OK" and returning the body of the response). 
Then, parsing the response for "0,5" - i.e. able to connect. We noticed that after a few moments that we get "0,4" (unknown), 
we are able to get "0,5" (registered, roaming). 
9. Sending AT+COPS? command.
10. Parsing the body of the response (looking for the "OK" and returning the body of the response).
11. Sending AT+COPS=? command.
12. Parsing the body of the response (looking for the "OK" and returning the body of the response).
13. Done.

## General Notes
1. We saw that some responses take a few moments until we receive a response (for example - getting the first pbready, 
AT+CREG to get 0,5, AT+COPS=?). So we added a number of attempts with a timeout to get the response we expect 
(or print an error message if not). 
2. We tried to take out the SIM card and running the AT+CCID command. As expected we received error.
3. We saw AT+CREG takes a while until we have "0,5" - we examined the altering response after a while.
  