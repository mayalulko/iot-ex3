#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include "unistd.h"
#include "config.h"

static int serialFd;

/**
 * @brief Initialises the serial connection.
 * @param port - the port to connected to. e.g: /dev/ttyUSB0, /dev/ttyS1 for Linux and COM8, COM10, COM53 for Windows.
 * @param baud - the baud rate of the communication. For example: 9600, 115200
 * @return 0 if succeeded in opening the port and -1 otherwise.
 */
int SerialInit(char* port, unsigned int baud) {
    serialFd = open(port, O_RDWR | O_NOCTTY);
    if (serialFd < 0){
        return -1;
    }

    struct termios SerialPortSettings;
    tcgetattr(serialFd, &SerialPortSettings);
    cfsetispeed(&SerialPortSettings, baud);
    cfsetospeed(&SerialPortSettings, baud);
    SerialPortSettings.c_cflag &= ~PARENB; // clear parity bit
    SerialPortSettings.c_cflag &= ~CSTOPB; // stop bit =1
    SerialPortSettings.c_cflag &= ~CSIZE;
    SerialPortSettings.c_cflag |= CS8;
    SerialPortSettings.c_cflag &= ~CRTSCTS; //turn off hardware based flow control.
    SerialPortSettings.c_cflag |= CREAD | CLOCAL;
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tcsetattr(serialFd, TCSANOW, &SerialPortSettings);

    return 0;
}

/**
 * @brief Receives data from serial connection.
 * @param buf - the buffer that receives the input.
 * @param max_len - maximum bytes to read into buf (buf must be equal or greater than max_len).
 * @param timeout_ms - read operation timeout milliseconds.
 * @return amount of bytes read into buf, -1 on error.
*/
int SerialRecv(unsigned char *buf, unsigned int max_len, unsigned int timeout_ms) {
    struct termios SerialPortSettings;
    tcgetattr(serialFd, &SerialPortSettings);

    SerialPortSettings.c_cc[VMIN] = 15; // TODO:: check if this is ok and shouldn't be bigger
    SerialPortSettings.c_cc[VTIME] = timeout_ms * 0.01; // deciseconds
    tcsetattr(serialFd, TCSANOW, &SerialPortSettings); // config!!

    int bytes_read = read(serialFd, buf, max_len);
    if (bytes_read < 0) {
        return -1;
    }
    return bytes_read;
}

/**
 * @brief Sends data through the serial connection.
 * @param buf - the buffer that contains the data to send
 * @param size - number of bytes to send
 * @return amount of bytes written into buf, -1 on error
 */
int SerialSend(unsigned char *buf, unsigned int size) {
    int writeval = write(serialFd, buf, size);
    if (writeval < (int)size) {
        return -1;
    }
    return writeval;
}


/**
 * @brief Empties the input buffer.
 */
void SerialFlushInputBuff(void) {
    int tf = tcflush(serialFd, TCIFLUSH);
    if (tf < 0) {
        return;
    }
}

/**
 * @brief Disable the serial connection.
 * @return 0 if succeeded in closing the port and -1 otherwise.
 */
int SerialDisable(void) {
    return close(serialFd);
}
