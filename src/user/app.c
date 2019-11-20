#include "user_config.h"
#include "driver/gpio.h"
#include "../camera/include/camera.h"
#include "../camera/include/bitmap.h"
#include "../camera/camera_common.h"
#include "ocrplate/ocrplate.h"
#include "sdCard/sdcard.h"
#include "qr_recoginize.h"
#include "track/track.h"
#include "led.h"

#define MODE_SDCARD_JPEG    0   //SD卡
#define MODE_QUIRC_GARY     0   //二维码识别
#define MODE_OCR_SCAN       0   //车牌识别  准确率不够
#define MODE_TRACK_JPEG     1   //循迹（色块）

#if MODE_SDCARD_JPEG
#define CAMERA_PIXEL_FORMAT CAMERA_PF_JPEG
#define CAMERA_FRAME_SIZE CAMERA_FS_QVGA
static uint32_t sd_savename = 0;
#endif

#if MODE_QUIRC_GARY
#define CAMERA_PIXEL_FORMAT CAMERA_PF_GRAYSCALE
#define CAMERA_FRAME_SIZE CAMERA_FS_QVGA
#endif

#if MODE_OCR_SCAN
#define CAMERA_PIXEL_FORMAT CAMERA_PF_GRAYSCALE
#define CAMERA_FRAME_SIZE CAMERA_FS_QVGA
#endif

#if MODE_TRACK_JPEG
#define CAMERA_PIXEL_FORMAT CAMERA_PF_JPEG
#define CAMERA_FRAME_SIZE CAMERA_FS_QVGA
#endif

static const char* TAG = "camera_main";

static camera_pixelformat_t s_pixel_format;
static void Camera_JPEG_Task(void *parm);


void app_main()
{
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }
#if MODE_SDCARD_JPEG
    SD_Card_Init();
#endif
    camera_config_t camera_config = {
        .ledc_channel = LEDC_CHANNEL_0,
        .ledc_timer = LEDC_TIMER_0,
        .pin_d0 = CONFIG_D0,
        .pin_d1 = CONFIG_D1,
        .pin_d2 = CONFIG_D2,
        .pin_d3 = CONFIG_D3,
        .pin_d4 = CONFIG_D4,
        .pin_d5 = CONFIG_D5,
        .pin_d6 = CONFIG_D6,
        .pin_d7 = CONFIG_D7,
        .pin_xclk = CONFIG_XCLK,
        .pin_pclk = CONFIG_PCLK,
        .pin_vsync = CONFIG_VSYNC,
        .pin_href = CONFIG_HREF,
        .pin_sscb_sda = CONFIG_SDA,
        .pin_sscb_scl = CONFIG_SCL,
        .pin_reset = CONFIG_RESET,
        .xclk_freq_hz = CONFIG_XCLK_FREQ,
    };

    camera_model_t camera_model;
    err = camera_probe(&camera_config, &camera_model);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x", err);
        return;
    }

    if (camera_model == CAMERA_OV2640) {
        ESP_LOGI(TAG, "Detected OV2640 camera, using JPEG format");
        s_pixel_format = CAMERA_PIXEL_FORMAT;
        camera_config.frame_size = CAMERA_FRAME_SIZE;
        if (s_pixel_format == CAMERA_PF_JPEG)
            camera_config.jpeg_quality = 15;
    } else {
        ESP_LOGE(TAG, "Camera not supported");
        return;
    }

    camera_config.pixel_format = s_pixel_format;
    err = camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }
    xTaskCreate(Camera_JPEG_Task,"Camera_JPEG_Task",4096,&camera_config,6,NULL);
    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
}


static void Camera_JPEG_Task(void *parm)
{
#if MODE_SDCARD_JPEG
    char Namebuffer[64];
#endif
    while (1)
    {
#if MODE_SDCARD_JPEG
        sd_savename++;
        if(s_pixel_format == CAMERA_PF_JPEG){
            esp_err_t err = camera_run();
            if (err != ESP_OK) {
                ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                return;
            }
            ESP_LOGI(TAG,"camera width height is: %d ,%d ,%d",camera_get_fb_width(),camera_get_fb_height(),camera_get_data_size());
            //SDCard_f_mkdir("/sdcard/config.ini");
            sprintf(Namebuffer,"/sdcard/IMG_%d.jpg",sd_savename);
            SDCard_WriteFile((char *)Namebuffer,(char *)camera_get_fb(),0,camera_get_data_size());
        }
#endif
#if MODE_QUIRC_GARY
        if(s_pixel_format == CAMERA_PF_GRAYSCALE)
        {
            esp_err_t err = camera_run();
            if (err != ESP_OK) {
                ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                return;
            }
            ESP_LOGI(TAG,"camera width height is: %d ,%d ,%d",camera_get_fb_width(),camera_get_fb_height(),camera_get_data_size());
            camera_config_t *camera_config = parm;
            ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
            xTaskCreate(qr_recoginze, "qr_recoginze", 111500, camera_config, 7, NULL);
        }
#endif  
#if MODE_OCR_SCAN
        if(s_pixel_format == CAMERA_PF_GRAYSCALE){
            esp_err_t err = camera_run();
            if (err != ESP_OK) {
                ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                return;
            }
            ESP_LOGI(TAG,"camera width height is: %d ,%d ,%d",camera_get_fb_width(),camera_get_fb_height(),camera_get_data_size());
            Camera_Scan((char *)camera_get_fb());
        }
#endif    
#if MODE_TRACK_JPEG
         if(s_pixel_format == CAMERA_PF_JPEG){
            esp_err_t err = camera_run();
            if (err != ESP_OK) {
                ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                return;
            }
            ESP_LOGI(TAG,"camera width height is: %d ,%d ,%d",camera_get_fb_width(),camera_get_fb_height(),camera_get_data_size());
            ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
            Track_Find((char *)camera_get_fb(),camera_get_data_size());
         }
#endif      
        vTaskDelay(1000/portTICK_RATE_MS );
    }
    
}










