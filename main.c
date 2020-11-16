#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "serial_io.h"
#include "config.h"

static char TX_BUFFER[BUFFER_SIZE] = {0};
static char RX_BUFFER[BUFFER_SIZE] = {0};

static const char* OK_END_MSG = "OK";
enum RETURN_CODE {SUCCESS = 0, ERROR = -1, WARNING = -2};

/**
 * Executes `command` on modem
 * @param command Command to execute
 * @param commandLen Command length
 * @return ERROR if a fatal error has occurred, WARNING if a non fatal error occurred, and number of sent bytes if successful.
 */
static int executeCommand(const char* command, int commandLen);

/**
 * Reads output from modem until encountering `endMsg` in the output buffer, and printing the output to stdout.
 * Waits `timeout` milliseconds between each attempt to read, until `maxNumAttempts` has elapsed.
 *
 * @param buf Input buffer, assumed to be zeroed
 * @param bufsize Size of input buffer
 * @param timeout Timeout in millisecond between attempts
 * @param endMsg string to look for in input buffer, encountering this will cause function to return
 * @param maxNumAttempts Maximum number of attempts to execute command
 * @return ERROR if a fatal error has occurred, WARNING if a non fatal error occurred, and number of sent received if successful.
 */
static int readUntil(char* buf, int bufsize, int timeout, const char* endMsg, int maxNumAttempts);

/**
 * Executes `executeCommand()` and `readUntil` sequentially, resetting RX_BUFFER after printing output to stdout.
 */
static int execSimpleModemCommand(const char* cmd, int timeout);

/**
 * Executes `AT+CREG?` until modem is notifying it successfully registered to cellular network or until predefined number of attempts has elapsed.
 */
static int execAT_CREG(void);


int main() {
    printf("1) Initializing serial connection to modem\n");
    int retval = SerialInit(MODEM_PORT, MODEM_BAUD_RATE);
    if (retval < 0) {
        fprintf(stderr, "\tFailed to initialize serial connection, aborting.\n");
        return ERROR;
    }

    const char* PBREADY = "+PBREADY";
    printf("2) Connecting to modem, please turn it on. Waiting up to 30 seconds for `%s`\n", PBREADY);
    retval = readUntil(RX_BUFFER, BUFFER_SIZE, 30000, PBREADY, MAX_NUMBER_ATTEMPTS);
    if (retval < 0) {
        fprintf(stderr, "\tFailed to connect to modem, aborting.\n");
        SerialDisable();
        return ERROR;
    }
    memset(RX_BUFFER, 0, BUFFER_SIZE);
    printf("3) Turning off echo mode\n");
    char* cmd = "ATE0";
    retval = execSimpleModemCommand(cmd, 500);
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }

    cmd = "AT";
    printf("3) Sending `%s` command and reading output\n", cmd);
    retval = execSimpleModemCommand(cmd, 500);
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }

    cmd = "AT+CCID";
    printf("4) Sending `%s` command and reading output\n", cmd);
    retval = execSimpleModemCommand(cmd, 500);
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }

    printf("5) Sending `AT+CREG?` command and reading output. "
           "Repeat this until recognized by a cellular operator with 5 seconds between attempts.\n");
    retval = execAT_CREG();
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }

    cmd = "AT+COPS?";
    printf("6) Sending `%s` command and reading output\n", cmd);
    retval = execSimpleModemCommand(cmd, 500);
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }

    cmd = "AT+COPS=?";
    printf("7) Sending `%s` command and reading output\n", cmd);
    retval = execSimpleModemCommand(cmd, 180000);
    if (retval == ERROR) {
        fprintf(stderr, "\tA fatal error has occurred, aborting.\n");
        return SerialDisable();
    } else if (retval == WARNING) {
        fprintf(stderr, "\tCommand execution of `%s` failed, skipping.\n", cmd);
    }


    printf("8) Closing serial connection to modem\n");
    return SerialDisable();
}

int executeCommand(const char* command, int commandLen) {
    printf("\tExecuting command `%s`\n", command);
    int commandLenUpdated = commandLen + (int)strlen(ENDL);
    // Pre-formatting command
    snprintf(TX_BUFFER, commandLenUpdated + 1, "%s%s%c", command, ENDL, '\0');
    int bytesSent = SerialSend((unsigned char*)TX_BUFFER, commandLenUpdated);
    TX_BUFFER[0]='\0';

    if (bytesSent < 0) {
        fprintf(stderr, "Error occurred while sending command to modem\n");
        return ERROR;
    }

    printf("\tSuccessfully sent %d bytes to modem\n", bytesSent);
    return bytesSent;
}

int readUntil(char* buf, int bufsize, int timeout, const char* endMsg, int maxNumAttempts){
    printf("\tReading data continuously from modem, waiting until `%s` appears in output.\n", endMsg);
    printf("\t\tMaking %d attempts with %d ms timeout between each attempt.\n", maxNumAttempts, timeout);

    int bytesRecv;
    int totalBytesRecv = 0;
    int attempts = 0;
    unsigned char* curLocation = NULL;
    curLocation = (unsigned char*) buf;

    int found = 0;  // If we found endMsg in the rx buffer, we will mark this value with 1.
    while ((attempts < maxNumAttempts) && (totalBytesRecv < bufsize)) {
        bytesRecv = SerialRecv(curLocation, bufsize - totalBytesRecv, timeout);
        if (bytesRecv < 0) {
            fprintf(stderr, "\tError occurred while reading data from modem, aborting.\n");
            return ERROR;
        }

        totalBytesRecv += bytesRecv;
        printf("\tSuccessfully read %d bytes from modem, combined for a total of %d bytes so far\n", bytesRecv, totalBytesRecv);
        if (strstr(buf, endMsg)!=NULL) {
            found = 1;
            break;
        }

        printf("\tFailed attempt #%d / %d\n", ++attempts, maxNumAttempts);
        curLocation += bytesRecv;
    }

    buf[totalBytesRecv] = '\0';
    printf("\tReceived message from modem:\n%s\n", buf);

    if (!found){
        fprintf(stderr, "\tError, did not encounter `%s` in incoming data.\n", endMsg);
        return WARNING;
    }

    SerialFlushInputBuff();
    return totalBytesRecv;
}

int execSimpleModemCommand(const char* cmd, int timeout) {
    int bytesSent = executeCommand(cmd, (int)strlen(cmd));
    if (bytesSent < 0) {
        fprintf(stderr, "\tAn error occurred while executing command `%s`\n", cmd);
        return ERROR;
    }

    int bytesRead = readUntil(RX_BUFFER, BUFFER_SIZE, timeout, OK_END_MSG, MAX_NUMBER_ATTEMPTS);
    SerialFlushInputBuff();
    memset(RX_BUFFER, 0, BUFFER_SIZE);
    if (bytesRead < 0) {
        fprintf(stderr, "\tAn error occurred while reading the output of command`%s`\n", cmd);
        return ERROR;
    }

    return bytesRead;
}

int execAT_CREG(void) {
    const char* cmd = "AT+CREG?";
    const char* HAS_CONNECTION = "0,5";
    int connectAttempts = 0;
    int maxAttempts = 10; // Maximum number of attempts for executing AT+CREG? until giving up
    int bytesRecv = 0;
    int bytesSend;

    while(connectAttempts < maxAttempts) {
        bytesSend = executeCommand(cmd, (int)strlen(cmd));
        if (bytesSend == ERROR) {
            fprintf(stderr, "\tAn error occurred while executing command `%s`, aborting\n", cmd);
            break;
        }

        bytesRecv = readUntil(RX_BUFFER, BUFFER_SIZE, 1000, OK_END_MSG, 1);

        if (bytesRecv == ERROR) {
            fprintf(stderr, "\tAn error occurred while reading from modem, aborting\n");
            break;
        }

        if ((bytesRecv > 0) && (strstr(RX_BUFFER, HAS_CONNECTION)!=NULL)) {
            printf("\tSuccessfully executed `%s`, and received %d bytes\n", cmd, bytesRecv);
            break;
        }

        printf("\tFailed attempt #%d / #%d\n", ++connectAttempts, maxAttempts);
        if (connectAttempts < maxAttempts) {
            SerialFlushInputBuff();
            memset(RX_BUFFER, 0, BUFFER_SIZE);
            sleep(5);
        }

    }

    SerialFlushInputBuff();
    memset(RX_BUFFER, 0, BUFFER_SIZE);
    return bytesRecv;
}
