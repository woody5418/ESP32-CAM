#include "sdcard.h"

/*
     *   IO14      CLK
     *   IO15      CMD
     *   IO2       D0
     *   IO4       D1
     *   IO12      D2
     *   IO13      D3
   */

static const char *TAG = "sdcard";
sdmmc_card_t* card;


//挂载SD卡
static void Mount_SD_CARD(void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%d). "
                "Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return;
    }
}

//取消挂载SD卡
static void UnMount_SD_CARD(void)
{
    esp_vfs_fat_sdmmc_unmount();
}

//SD卡引脚初始化
void SD_Card_Init(void)
{
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    Mount_SD_CARD();
    sdmmc_card_print_info(stdout, card);
    UnMount_SD_CARD();
}
/*
 *保存数据到SD卡
 *fileName：写入的文件名字
 *fileData：写入的数据
 *len：数据长度
 */
void Save_JPEG_SD(char *fileName,char *fileData,uint32_t len)
{
    Mount_SD_CARD();
    //ESP_LOGI(TAG, "Opening file");
    FILE* fp = fopen(fileName, "w");
    if (fp == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        UnMount_SD_CARD();
        return;
    }
    fwrite(fileData,len,1,fp);
    fclose(fp);
    UnMount_SD_CARD();
}

FRESULT SDCard_WriteFile(char* fileName,char *fileData,DWORD ofs,UINT strl)
{																		
    FRESULT res;         // FatFs function common result code

    Mount_SD_CARD();
    FILE* fp = fopen(fileName, "w"); 
    if (fp == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        UnMount_SD_CARD();
        return FR_INT_ERR;
    }
    res = fseek (fp, ofs, SEEK_SET);  //SEEK_SET：文件开头   SEEK_CUR：当前位置   SEEK_END：文件结尾
	fwrite(fileData, strl, 1,fp);
	fclose(fp);//关闭文件
    UnMount_SD_CARD();
	return res;
}


//读取内容
FRESULT ReadFileData(char *fileName,char *fileData,uint32_t datalen)
{
    FRESULT res;  

	Mount_SD_CARD();
    FILE* fp = fopen(fileName, "r");
    if (fp == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        UnMount_SD_CARD();
        return FR_INT_ERR;
    }
    fread(fileData, 1, datalen, fp);
    
	fclose(fp);
	UnMount_SD_CARD();
    ESP_LOGI(TAG,"Read Data is :%s", fileData);
    return res;
}


//创建一个新目录，输入目录的路径，只能一级一级的建立
FRESULT SDCard_f_mkdir(const TCHAR* path)
{
    FRESULT res;         // FatFs function common result code

	Mount_SD_CARD();
	res=f_mkdir(path);//创建一个目录
	UnMount_SD_CARD();
	return res;
}

















