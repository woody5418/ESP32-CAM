/*
 * led.h
 *
 *  Created on: 2017年12月11日
 *      Author: ai-thinker
 */

#ifndef OCR_PLATE_H_
#define OCR_PLATE_H_
#include "user_config.h"

void Camera_Scan(char *picture);
char JumpPointAnalysis_240(void);
void JumpPointAnalysis_320(char *picture);
char JumpPointAnalysis_blue(char *picture);
char CharacterSegmentation(void);
char * CharacterCompare(char *data,uint8_t BitNum);
uint16_t NormalizationAlgorithm(char *picture,uint16_t k_t,uint16_t kk_t,uint16_t num);


#endif 
