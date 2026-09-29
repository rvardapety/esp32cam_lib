#include "esp32_cam.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <stdint.h>

#define UART_DEVICE "/dev/serial0"
#define UART_BAUDRATE B115200
#define CMD_TRIGGER_CAPTURE 0xAA
#define FRAME_LENGTH_SIZE 4
#define MAX_FRAME_SIZE 500000

int file;

bool esp32_cam_init(void) {

    if ((file = open(UART_DEVICE, O_RDWR | O_NOCTTY)) < 0) {
        return false;
    }

    struct termios options;

    if (tcgetattr(file, &options) < 0) {
        close(file);
        return false;
    }

    cfsetispeed(&options, UART_BAUDRATE);
    cfsetospeed(&options, UART_BAUDRATE);

    options.c_cflag |= (CLOCAL | CREAD);

    // 8 data bits
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // No parity
    options.c_cflag &= ~PARENB;

    // 1 stop bit
    options.c_cflag &= ~CSTOPB;

    // Raw input/output
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    if (tcsetattr(file, TCSANOW, &options) < 0) {
        close(file);
        return false;
    }

    return true;
}


bool esp32_cam_get_frame(cam_frame_t *frame) {

    if (frame == NULL) {
        return false;
    }

    // Send capture command
    uint8_t command = CMD_TRIGGER_CAPTURE;

    if (write(file, &command, 1) != 1) {
        return false;
    }

    // Receive 4-byte image length
    uint32_t frame_length = 0;
    uint8_t *length_bytes = (uint8_t *)&frame_length;

    int bytes_received = 0;

    while (bytes_received < FRAME_LENGTH_SIZE) {

        int bytes_read = read(
                file,
                length_bytes + bytes_received,
                FRAME_LENGTH_SIZE - bytes_received
        );

        if (bytes_read <= 0) {
            return false;
        }

        bytes_received += bytes_read;
    }

    // Check image size
    if (frame_length == 0 || frame_length > MAX_FRAME_SIZE) {
        return false;
    }

    // Allocate memory for JPEG
    frame->data = malloc(frame_length);

    if (frame->data == NULL) {
        return false;
    }

    frame->length = frame_length;

    // Receive JPEG data
    uint32_t total_received = 0;

    while (total_received < frame->length) {

        int bytes_read = read(
                file,
                frame->data + total_received,
                frame->length - total_received
        );

        if (bytes_read <= 0) {
            free(frame->data);
            frame->data = NULL;
            frame->length = 0;

            return false;
        }

        total_received += bytes_read;
    }

    return true;
}

bool esp32_cam_free_frame(cam_frame_t *frame) {

    if (frame == NULL || frame->data == NULL) {
        return false;
    }

    free(frame->data);

    frame->data = NULL;
    frame->length = 0;

    return true;
}

bool esp32_cam_deinit(void) {

    if (file >= 0) {
        close(file);
        file = -1;
    }

    return true;
}