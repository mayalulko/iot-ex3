# IoT-ex1
## Authors
Oriyan Hermoni oriyan.hermoni@mail.huji.ac.il , 302170204<br> 
Maya Lulko maya.lulko@mail.huji.ac.il , 312414089

## Package contents:
* CMakeLists.txt - CMake file
* HTTP_client.c, HTTP_client.h - contract and implementation of HTTP client
* main.c - Entry point to program
* Makefile
* README.md
* socket.h - contract for creating a TCP socket, with read/write functionality
* socket_linux.c - implementation of socket.h with BSD sockets

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
The base of our design is the socket_linux.c that contains the API of the socket - including SocketInit, SocketRead, 
SocketWrite,SocketConnect. The HTTP_client uses this interface to open a socket with the server and send & receive 
requests of GET and POST. The main.c instantiates the HTTP_client and sends one GET requests followed by a Post request.
For more details see the Application flow below. <br>

To avoid doing heap allocation, we statically allocated the buffers at compile time. We allocated three buffers:
1. REQUEST_BUFFER - 4kB, re-using this buffer for sending GET and POST requests on Tx
2. RESPONSE_BUFFER - 4k, re-using this buffer to store the entire HTTP response on Rx
3. HOST - 128B , for storing the host name
4. Additionally, in main.c we allocated 1kB for storing the body of the HTTP responses<br><br>

All of these are more than enough space for the expected response data.

## Application Flow
1. Initializing HTTP client - Creating tcp socket and connecting to server 
2. Sending a GET request to server - to URL '/'
3. Receiving response to the GET request from the server 
4. Parsing the body of the response (looking for the "200 OK" and returning the body of the response)
5. Sending a POST request to server - to URL '/', with content "Gimme gimme gimme a maaaan after midniiiiiight"
6. Receiving response from server
7. Parsing the body of the response (looking for the "200 OK" and returning the body of the response)
8. Done! 

