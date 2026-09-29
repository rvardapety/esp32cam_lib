#ifndef ESP32CAM_LIB_ESP32_CAM_H
#define ESP32CAM_LIB_ESP32_CAM_H

#include <stdbool.h>
#include <stdint.h>

// Struct to store pointer and length of retrieved JPEG frame buffer
typedef struct {
    uint8_t *data;
    uint32_t length;
} cam_frame_t;

// API Core Lifecycle Functions
bool esp32_cam_init(const char *serial_device);
bool esp32_cam_get_frame(cam_frame_t *frame);
bool esp32_cam_free_frame(cam_frame_t *frame);
bool esp32_cam_deinit(void);

#endif //ESP32CAM_LIB_ESP32_CAM_H
