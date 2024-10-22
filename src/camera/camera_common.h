#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "rom/lldesc.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "include/camera.h"
#include "sensor.h"

#define CONFIG_D0 5
#define CONFIG_D1 18
#define CONFIG_D2 19
#define CONFIG_D3 21
#define CONFIG_D4 36
#define CONFIG_D5 39
#define CONFIG_D6 34
#define CONFIG_D7 35
#define CONFIG_XCLK 0
#define CONFIG_PCLK 22
#define CONFIG_VSYNC 25
#define CONFIG_HREF 23
#define CONFIG_SDA 26
#define CONFIG_SCL 27
#define CONFIG_RESET 32
#define CONFIG_QR_RECOGNIZE  19
#define CONFIG_XCLK_FREQ 10000000

#define CONFIG_OV2640_SUPPORT 1
#define CONFIG_OV7725_SUPPORT 0 


typedef union {
    struct {
        uint8_t sample2;
        uint8_t unused2;
        uint8_t sample1;
        uint8_t unused1;
    };
    uint32_t val;
} dma_elem_t;

typedef enum {
    /* camera sends byte sequence: s1, s2, s3, s4, ...
     * fifo receives: 00 s1 00 s2, 00 s2 00 s3, 00 s3 00 s4, ...
     */
    SM_0A0B_0B0C = 0,
    /* camera sends byte sequence: s1, s2, s3, s4, ...
     * fifo receives: 00 s1 00 s2, 00 s3 00 s4, ...
     */
    SM_0A0B_0C0D = 1,
    /* camera sends byte sequence: s1, s2, s3, s4, ...
     * fifo receives: 00 s1 00 00, 00 s2 00 00, 00 s3 00 00, ...
     */
    SM_0A00_0B00 = 3,
} i2s_sampling_mode_t;

typedef void (*dma_filter_t)(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst);

typedef struct {
    camera_config_t config;
    sensor_t sensor;
    uint8_t *fb;
    size_t fb_size;
    size_t data_size;
    size_t width;
    size_t height;
    size_t in_bytes_per_pixel;
    size_t fb_bytes_per_pixel;
    size_t stride;
    size_t frame_count;

    lldesc_t *dma_desc;
    dma_elem_t **dma_buf;
    bool dma_done;
    size_t dma_desc_count;
    size_t dma_desc_cur;
    size_t dma_received_count;
    size_t dma_filtered_count;
    size_t dma_per_line;
    size_t dma_buf_width;
    size_t dma_sample_count;
    i2s_sampling_mode_t sampling_mode;
    dma_filter_t dma_filter;
    intr_handle_t i2s_intr_handle;
    intr_handle_t vsync_intr_handle;
    QueueHandle_t data_ready;
    SemaphoreHandle_t frame_ready;
    TaskHandle_t dma_filter_task;
} camera_state_t;

extern camera_state_t* s_state ;
