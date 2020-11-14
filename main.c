#include <string.h>
#include <stdio.h>
#include "serial_io.h"
#include "config.h"
#include "time.h"
#include <unistd.h>

char TX_BUFFER[BUFFER_SIZE] = {0};
unsigned char RX_BUFFER[BUFFER_SIZE] = {0};

unsigned char* AT = "AT";
unsigned char* AT_CCID = "AT+CCID";
unsigned char* AT_COPS_ALL = "AT+COPS=?";
unsigned char* AT_COPS = "AT+COPS?";
unsigned char* AT_CREG = "AT+CREG?";
char* PBREADY = "+PBREADY";
char* OK = "OK";
char* HAS_CONNECTION = "0,5";

/**
 *
 * @param command
 * @param commandSize
 * @return
 */
int writeCommand(unsigned char* command, int commandSize);

/**
 *
 * @param buf
 * @param bufsize
 * @param timeout
 * @param endMsg
 * @param maxNumAttempts
 * @return
 */
int readUntil(unsigned char* buf, int bufsize, int timeout, char* endMsg, int maxNumAttempts);


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
    printf("Initializing... \n");
    int retval = SerialInit(MODEM_PORT, MODEM_BAUD_RATE);
    if (retval < 0) {
        // TODO:: errmsg
        return -1;
    }
    //printf("%d \n", retval);
    printf("Connecting...\n");
    /*
//    SerialRecv((unsigned char*)RX_BUFFER, BUFFER_SIZE,15000);
    retval = readUntil(RX_BUFFER, BUFFER_SIZE, 15000, PBREADY, MAX_NUMBER_ATTEMPTS);
    if (retval < 0) {
        SerialDisable();
        //TODO:: errmsg
        return -1;
    }
    printf("Waiting for pbready.\n Received number of bytes: %d \n", retval);
    memset(RX_BUFFER, 0, sBUFFER_SIZE;
    */

    //writeCommand("ATE0", 4, RX_BUFFER, BUFFER_SIZE, 500);
    //memset(RX_BUFFER, 0, sBUFFER_SIZE;
    /* AT command */
    printf("sending AT command\n");
    writeCommand(AT, strlen(AT));
    printf("Receiving response to AT command\n");
    readUntil(RX_BUFFER, BUFFER_SIZE, 500, OK, MAX_NUMBER_ATTEMPTS);
    memset(RX_BUFFER, 0, BUFFER_SIZE);

    /* AT+CCID? command */
    printf("sending AT+CCID? command\n");
    writeCommand(AT_CCID, strlen(AT_CCID)); // TODO check that ccid's length is ok
    printf("Receiving response to AT+CCID? command\n");
    retval = readUntil(RX_BUFFER, BUFFER_SIZE, 500, OK, MAX_NUMBER_ATTEMPTS);
    if (retval < 0) {
        SerialDisable();
        //TODO:: errmsg
        return -1;
    }
    memset(RX_BUFFER, 0, BUFFER_SIZE);


//    writeCommand(AT_CREG, strlen(AT_CREG));
//    readUntil(RX_BUFFER, BUFFER_SIZE, 500, )

    int connectAttempts = 0;
    while(connectAttempts < MAX_NUMBER_ATTEMPTS) {
        printf("sending AT+CREG? command, attempt %d\n", connectAttempts);
        writeCommand(AT_CREG, strlen(AT_CREG));
        printf("Receiving response to AT+CREG? command\n");
        int bytes = readUntil(RX_BUFFER, BUFFER_SIZE, 500, OK, MAX_NUMBER_ATTEMPTS);
        if ((bytes > 0) && (strstr(RX_BUFFER, HAS_CONNECTION)!=NULL)) {
            break;
        }
        memset(RX_BUFFER, 0, BUFFER_SIZE);
        ++connectAttempts;
        if (connectAttempts == MAX_NUMBER_ATTEMPTS) {
            SerialDisable();
            return -1;
        }

        sleep(5);
    }
    memset(RX_BUFFER, 0, BUFFER_SIZE);

    /* AT+COPS? command */
    printf("sending AT+COPS? command\n");
    writeCommand(AT_COPS, strlen(AT_COPS));
    printf("Receiving response to AT+COPS? command\n");
    readUntil(RX_BUFFER, BUFFER_SIZE, 500, OK, MAX_NUMBER_ATTEMPTS);
    memset(RX_BUFFER, 0, BUFFER_SIZE);

    /* AT+COPS=? command */
    printf("sending AT+COPS=? command\n");
    writeCommand(AT_COPS_ALL, strlen(AT_COPS_ALL));
    printf("Receiving response to AT+COPS=? command\n");
    readUntil(RX_BUFFER, BUFFER_SIZE, 18e4, OK, MAX_NUMBER_ATTEMPTS);
    memset(RX_BUFFER, 0, BUFFER_SIZE);
    printf("yeah!\n");


    int disval = SerialDisable();
    printf("DISABLE %d \n", disval);
}

int writeCommand(unsigned char* command, int commandSize) {
    // Write command
    int commandSize_fmt = commandSize+strlen(ENDL);
    snprintf(TX_BUFFER,commandSize_fmt+1, "%s%s%c", command, ENDL, '\0');
    int bytesSent = SerialSend((unsigned char*)TX_BUFFER, commandSize_fmt);
    printf("sent bytes %d \n", bytesSent);
    TX_BUFFER[0]='\0';
    return bytesSent;
}

int readUntil(unsigned char* buf, int bufsize, int timeout, char* endMsg, int maxNumAttempts){
    // Receive answer

    int bytesRecv = 0;
    int bufferReadSoFar = 0;
    int attemps = 0;
    unsigned char* curLocation = NULL;
    curLocation = buf;

    // TODO: timeout proportional to attempts.
    int found = 0;
    while ((attemps < maxNumAttempts) && (bufferReadSoFar < bufsize)) {
        printf("In read. attempt: %d.\n", attemps);
        bytesRecv = SerialRecv(curLocation, bufsize-bufferReadSoFar, timeout);
        bufferReadSoFar += bytesRecv;
        if (strstr(buf, endMsg)!=NULL) {
            found = 1;
            break;
        }
        curLocation += bytesRecv;
        attemps++;
    }
    buf[bufferReadSoFar] = '\0';
    printf("Received message:\n %s ", buf);

    if (!found){
        return -1;
        printf("error"); // TODO error msg
    }


    SerialFlushInputBuff();
    printf("Number of bytes read: %d\n", bufferReadSoFar);
    return bufferReadSoFar;
}