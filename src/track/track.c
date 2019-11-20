/*
 * led.c
 *
 *  Created on: 2017年12月11日
 *      Author: ai-thinker
 */
#include "track.h"


void Track_Find(char *picture,size_t data_t)
{
    char *picture_t = NULL;

    picture_t = (char *)malloc(data_t*sizeof(char));
    if(picture_t == NULL) {
        printf("malloc picture error"); 
        return;
    }
    memcpy(picture_t,picture,data_t);
    
}









