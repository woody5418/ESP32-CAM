/*
 * led.h
 *
 *  Created on: 2017年12月11日
 *      Author: ai-thinker
 */

#ifndef SD_CARD_H_
#define SD_CARD_H_
#include "user_config.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "diskio.h"
#include "ff.h"

void SD_Card_Init(void);
void Save_JPEG_SD( char *fileName, char *fileData,uint32_t len);

FRESULT SDCard_WriteFile(char *fileName,char *fileData,DWORD ofs,UINT strl);
FRESULT SDCard_f_mkdir(const TCHAR* path);
FRESULT ReadFileData(char *fileName,char *fileData,uint32_t datalen);
#endif /* MAIN_LED_H_ */
