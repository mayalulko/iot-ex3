#include <string.h>
#include <stdio.h>
#include "serial_io.h"
#include "config.h"


char TX_BUFFER[BUFFER_SIZE] = {0};
unsigned char RX_BUFFER[BUFFER_SIZE] = {0};

unsigned char* AT = "AT";
unsigned char* AT_CCID = "AT+CCID";
unsigned char* AT_COPS_ALL = "AT+COPS=?";
unsigned char* AT_COPS = "AT+COPS?";
unsigned char* AT_CREG = "AT+CREG?";

/**
 *
 * @param command
 * @param commandSize
 * @param buf
 * @param bufsize
 * @param timeout
 * @return
 */
int writeAndRead(unsigned char* command, int commandSize, unsigned char* buf, int bufsize, int timeout);


int main() {
/**
 * init
 * wait for pbready (read)
 * AT (write)
 * AT+CCID
 * AT+COPS=?
 * AT+CREG
 * AT+COPS?
 * print the command and response
 * TODO: turn off the echo - this will affect the parsing -don't want that
 */
    int retval = SerialInit(MODEM_PORT, MODEM_BAUD_RATE);
    printf("%d \n", retval);
    printf("Connecting...\n");
//    retval = SerialRecv((unsigned char*)RX_BUFFER, BUFFER_SIZE,15000);
//    printf("after 1st rcv %d \n", retval);
//    RX_BUFFER[retval] = '\0';
//    printf("%s\n", RX_BUFFER);
//    SerialFlushInputBuff();

    writeAndRead("ATE0", 4, RX_BUFFER, BUFFER_SIZE, 500);
    memset(RX_BUFFER, 0, sizeof(char));
    writeAndRead(AT, strlen(AT), RX_BUFFER, BUFFER_SIZE, 500);
    memset(RX_BUFFER, 0, sizeof(char));
    writeAndRead(AT_CCID, strlen(AT_CCID), RX_BUFFER, BUFFER_SIZE, 500);
    memset(RX_BUFFER, 0, sizeof(char));
    writeAndRead(AT_CREG, strlen(AT_CREG), RX_BUFFER, BUFFER_SIZE, 500);
    memset(RX_BUFFER, 0, sizeof(char));
    writeAndRead(AT_COPS, strlen(AT_COPS), RX_BUFFER, BUFFER_SIZE, 500);
    memset(RX_BUFFER, 0, sizeof(char));
    writeAndRead(AT_COPS_ALL, strlen(AT_COPS_ALL), RX_BUFFER, BUFFER_SIZE, 18e4);
    memset(RX_BUFFER, 0, sizeof(char));
    printf("yeah!\n");


    int disval = SerialDisable();
    printf("DISABLE %d \n", disval);
}

int writeAndRead(unsigned char* command, int commandSize, unsigned char* buf, int bufsize, int timeout) {
    // Write command
    int commandSize_fmt = commandSize+strlen(ENDL);
    snprintf(TX_BUFFER,commandSize_fmt+1, "%s%s%c", command, ENDL, '\0');
    int bytesSent = SerialSend((unsigned char*)TX_BUFFER, commandSize_fmt);
    printf("sent bytes %d \n", bytesSent);
    TX_BUFFER[0]='\0';

    // Receive answer
    int bytesRcv = SerialRecv((unsigned char*)buf, bufsize, timeout);
    printf("bytes received %d \n", bytesRcv);

    buf[bytesRcv] = '\0';
    printf("Received message: %s \n", buf);

    SerialFlushInputBuff();

    return bytesRcv;

}