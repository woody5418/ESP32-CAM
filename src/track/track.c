/*
 * led.c
 *
 *  Created on: 2017年12月11日
 *      Author: ai-thinker
 */
#include "track.h"


static void Track_Finding_Task(void *parm);

void Track_Find(char *picture,size_t data_t)
{
    char *picture_t = NULL;

    picture_t = (char *)malloc(data_t*sizeof(char));
    if(picture_t == NULL) {
        printf("malloc picture error"); 
        return;
    }
    memcpy(picture_t,picture,data_t);

    xTaskCreate(Track_Finding_Task,"Track_Finding_Task",8192,picture_t,8,NULL);
}


void Track_Finding_Task(void *parm)








