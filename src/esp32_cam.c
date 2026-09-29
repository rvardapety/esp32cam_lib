#include "esp32_cam.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>

#define CMD_TRIGGER_CAPTURE 0xAA  // Custom simple protocol command byte

static int uart_fd = -1;

bool esp32_cam_init(const char *serial_device) {
    // Open the serial device (e.g., "/dev/serial0" or "/dev/ttyUSB0")
    uart_fd = open(serial_device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd < 0) {
        return false;
    }

    // Configure low-level UART hardware settings via termios
    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;      // No parity
    options.c_cflag &= ~CSTOPB;      // 1 Stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;          // 8 Data bits
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input mode
    options.c_oflag &= ~OPOST;       // Raw output mode

    tcsetattr(uart_fd, TCSANOW, &options);
    return true;
}

bool esp32_cam_get_frame(cam_frame_t *frame) {
    if (uart_fd < 0 || frame == NULL) return false;

    // 1. Send the API command byte to trigger the camera
    uint8_t cmd = CMD_TRIGGER_CAPTURE;
    if (write(uart_fd, &cmd, 1) < 0) return false;

    // 2. Read the upcoming 4-byte frame length header sent by ESP32
    uint32_t incoming_length = 0;
    int bytes_read = 0;
    uint8_t *len_ptr = (uint8_t *)&incoming_length;

    while (bytes_read < 4) {
        int r = read(uart_fd, len_ptr + bytes_read, 4 - bytes_read);
        if (r > 0) bytes_read += r;
    }

    if (incoming_length == 0 || incoming_length > 500000) return false; // Sanity check max jpeg sizes

    // 3. Dynamic allocation based on incoming packet payload size
    frame->length = incoming_length;
    frame->data = (uint8_t *)malloc(incoming_length);
    if (frame->data == NULL) return false;

    // 4. Read the raw JPEG binary stream directly from the UART file descriptor
    uint32_t total_payload_received = 0;
    while (total_payload_received < frame->length) {
        int r = read(uart_fd, frame->data + total_payload_received, frame->length - total_payload_received);
        if (r > 0) {
            total_payload_received += r;
        }
    }

    return true;
}

bool esp32_cam_free_frame(cam_frame_t *frame) {
    if (frame && frame->data) {
        free(frame->data);
        frame->data = NULL;
        frame->length = 0;
        return true;
    }
    return false;
}

bool esp32_cam_deinit(void) {
    if (uart_fd >= 0) {
        close(uart_fd);
        uart_fd = -1;
    }
    return true;
}
