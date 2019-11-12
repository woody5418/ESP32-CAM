/*
 * led.c
 *
 *  Created on: 2017年12月11日
 *      Author: ai-thinker
 */
#include "ocrplate.h"
#include "ocrplateFont.h"
#include "levenshtein.h"

uint16_t Max_ChangePoint_240,Min_ChangePoint_240;    //跳变点纵轴初始、结束坐标
uint16_t Max_ChangePoint_320,Min_ChangePoint_320;    //跳变点横轴初始、结束坐标

static uint8_t JumpPointCalculation_240[240];  //跳变点计算 240
static uint8_t JumpPointCalculation_320[320];  //跳变点计算 320

static void CharacterRecognition_Task(void *parm);

//二值化分析
void Camera_Scan(char *picture)
{
    uint16_t x_t,y_t;
    uint8_t jumpPoint=0;
    uint8_t jumpPoint_old=0;
    uint8_t MaxMinCompare=0;
    uint8_t i_return=0;

    char *picture_t=NULL;

    picture_t = (char*)malloc(76800*sizeof(char));
    if(picture_t == NULL) {
        printf("malloc picture error"); 
        return;
    }
    memcpy(picture_t,picture,76800);
    memset((uint8_t *)JumpPointCalculation_240,0,240);
    //判断每一行的跳变点个数
    for(y_t=0;y_t<240;y_t++){
        for(x_t=0;x_t<320;x_t++){
            if(picture_t[(y_t*320)+x_t] >= 0X5E)
                jumpPoint = 1;
            else
                jumpPoint = 0;
            if(jumpPoint_old != jumpPoint){
                JumpPointCalculation_240[y_t]++;
            }
            jumpPoint_old = jumpPoint;
        }
    }
   /* for(i=0;i<240;i++){
        printf("%d ",JumpPointCalculation_240[i]);
    }
    printf("\n");
    */
    MaxMinCompare = JumpPointAnalysis_240();
    if(MaxMinCompare == 0){ //跳变点筛选成功
        printf("240 MIN:%d\n",Min_ChangePoint_240);
        printf("240 MAX:%d\n",Max_ChangePoint_240);
        JumpPointAnalysis_blue((char *)picture_t);
    }else {
        printf("ERROR CODE is %d.\n",MaxMinCompare);
        free(picture_t);
        return;
    }
    if(MaxMinCompare == 0){
        JumpPointAnalysis_320((char *)picture_t);
        i_return = CharacterSegmentation();  //字符分割
        if((i_return == 9)||(i_return == 10)){         //识别到正确的数量创建字符匹配任务
            //CharacterRecognition((char *)picture_t); //字符归一
            xTaskCreate(CharacterRecognition_Task,"CharacterRecognition_Task",8192,picture_t,8,NULL); //字符归一
        }
        printf("i_return : %d\n",i_return);
        free(picture_t);
    }else{
        printf("ERROR CODE is %d.\n",MaxMinCompare);
        free(picture_t); 
    }

}

//跳变点分析，纵向分析出车牌的上下边界和高度值 返回1表示筛选成功
char JumpPointAnalysis_240(void)
{
    uint16_t i;

    Min_ChangePoint_240=240;
    Max_ChangePoint_240=0;

    for(i=0;i<240;i++){
        while(JumpPointCalculation_240[i] <= 15){
            i++;
        }
        Min_ChangePoint_240 = i;
        while(JumpPointCalculation_240[i] > 15){
            i++;
        }
        Max_ChangePoint_240 = i;
        if(Max_ChangePoint_240-Min_ChangePoint_240 >= 15) 
            i = 240;
    }
    Min_ChangePoint_240 = Min_ChangePoint_240 - 3;//向上微调3像素点
	Max_ChangePoint_240 = Max_ChangePoint_240 + 3;//向下微调3个像素点
    if(Min_ChangePoint_240 > Max_ChangePoint_240)  //判断合法性1 ：最小值 >最大值
		return 1;
	else if(Min_ChangePoint_240==240||Max_ChangePoint_240==0) //判断合法性2：值没有重新赋值
		return 2;
	else if((Max_ChangePoint_240-Min_ChangePoint_240) < 15)  //判断合法性3：阈值调节2-（2）
		return 3;
    else  
        return 0;
}
//Min_ChangePoint_240 73 
//Max_ChangePoint_240 145
char JumpPointAnalysis_blue(char *picture)
{
    uint16_t i,j;
    uint16_t min_320,max_320;

    min_320 = 320;
    max_320 = 0;
    for(i = Min_ChangePoint_240;i < Max_ChangePoint_240;i++){
        for(j=0;j<320;j++){
              if(picture[j+(i*320)] <= 0X5E){
                    if(j < min_320)
                        min_320 = j;
                    if(j > max_320)
                        max_320 = j;
              }
        }
    } 
    Min_ChangePoint_320 = min_320;    //获取各行的最小值微调
    Max_ChangePoint_320 = max_320;    //获取各行的最大值微调
    if(Min_ChangePoint_320 > Max_ChangePoint_320)  //判断合法性1 ：最小值 >最大值
		return 1;
	else if(Min_ChangePoint_320==240||Max_ChangePoint_320==0) //判断合法性2：值没有重新赋值
		return 2;
	else if((Max_ChangePoint_320-Min_ChangePoint_320) < 15)  //判断合法性3：0阈值调节2-（2）
		return 3;
    else {
        printf("320 MIN:%d\n",Min_ChangePoint_320);
        printf("320 MAX:%d\n",Max_ChangePoint_320);
        return 0;
    }
}

//纵向跳变点分析
void JumpPointAnalysis_320(char *picture)
{
    int16_t i,j;
    uint8_t jumpPoint=0;
    uint8_t jumpPoint_old=0;

    memset((char *)JumpPointCalculation_320,0,320);
    for(i=Min_ChangePoint_320;i<Max_ChangePoint_320;i++){
        for(j=Min_ChangePoint_240;j<Max_ChangePoint_240;j++){
            if(picture[(j*320)+i] >= 0X5E){
                jumpPoint = 1;
            }else
                jumpPoint = 0;
            if(jumpPoint_old != jumpPoint){
                JumpPointCalculation_320[i]++;
            }
            jumpPoint_old = jumpPoint;
        }
    }
   /* for(int16_t n = 0; n < (Max_ChangePoint_320-Min_ChangePoint_320); n++)
        printf("%d ",JumpPointCalculation_320[n]);
    printf("\n");
    */
}




//字符分割
char CharacterSegmentation(void)
{
    uint8_t i=0;        //统计的字符分割的个数，不为9说明分割有误
    uint16_t b;

    for(b=Min_ChangePoint_320;b<Max_ChangePoint_320;b++){
        if(JumpPointCalculation_320[b] == 0){
            i++;
            while(JumpPointCalculation_320[b] == 0){
                b++;
                if(b > Max_ChangePoint_320)
                    break;
            }
        }
    }
    i--;
    return i;
}



//字符识别任务
void CharacterRecognition_Task(void *parm)
{
    //uint8_t Result;     //识别结果
    uint16_t k1,kk1,k2,kk2,k3,kk3,k4,kk4,k5,kk5,k6,kk6,k7,kk7,k8,kk8;  //字符边界
    uint16_t b;

    //从右至左识别字符的k， kk值
    b = Min_ChangePoint_320+1;
    while(JumpPointCalculation_320[b] == 0) b++;  //取第一个字符
    k1 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk1 = b;
    if((kk1-k1) <= 5){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第一个字符
        k1 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk1 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第二个字符
    k2 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk2 = b;
    if((kk2-k2) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第二个字符
        k2 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk2 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第三个字符
    k3 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk3 = b;
    if((kk3-k3) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第三个字符
        k3 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk3 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第四个字符
    k4 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk4 = b;
    if((kk4-k4) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第四个字符
        k4 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk4 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第五个字符
    k5 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk5 = b;
    if((kk5-k5) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第五个字符
        k5 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk5 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第六个字符
    k6 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk6 = b;
    if((kk6-k6) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第六个字符
        k6 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk6 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第七个字符
    k7 = b+1;
    while(JumpPointCalculation_320[b] > 0) b++;
    kk7 = b;
    if((kk7-k7) < 4){
        while(JumpPointCalculation_320[b] == 0) b++;  //取第七个字符
        k7 = b+1;
        while(JumpPointCalculation_320[b] > 0) b++;
        kk7 = b;
    }
    while(JumpPointCalculation_320[b] == 0) b++;  //取第八个字符
    k8 = b+1;
    while(JumpPointCalculation_320[b] > 0) {
        if(b>=Max_ChangePoint_320) break;
        b++;
    }
    kk8 = b;
//取第一个字符

    NormalizationAlgorithm(parm,k1,kk1);
    NormalizationAlgorithm(parm,k2,kk2);
    NormalizationAlgorithm(parm,k3,kk3);
    NormalizationAlgorithm(parm,k4,kk4);
    NormalizationAlgorithm(parm,k5,kk5);
    NormalizationAlgorithm(parm,k6,kk6);
    NormalizationAlgorithm(parm,k7,kk7);
    NormalizationAlgorithm(parm,k8,kk8);
    vTaskDelete(NULL);
}


//归一算法   40*70
uint16_t NormalizationAlgorithm(char *picture,uint16_t k1_t,uint16_t kk1_t)
{
    uint16_t m=0;
    uint16_t i,j;
    uint16_t i_t,j_t;
    uint16_t x_t,y_t;
    uint16_t i_return=0;
    uint16_t Min_240,Max_240;//上下边界线
    uint8_t *comp_province; //省份缓存数组

    //printf("--%d,%d\n",k1_t,kk1_t);
    if((kk1_t - k1_t)<=10)  {
        k1_t-=8;
        kk1_t+=8;
    }
    if((kk1_t - k1_t) < 40)
    {
        Min_240 = Min_ChangePoint_240+1;    //上边界
        Max_240 = Max_ChangePoint_240-1;    //下边界
        i_t = Max_240-Min_240;
        if(i_t%2 != 0) i_t += 1;
        i_t = i_t/2;
        j_t = kk1_t-k1_t;
        if(j_t%2 != 0) j_t += 1;
        j_t = j_t/2;
        x_t = Min_240;
        comp_province = (uint8_t *)malloc((i_t*j_t+1)*sizeof(uint8_t));
        if(comp_province == NULL){
            printf("province malloc error");   //申请缓存空间
            return 1;
        }
        for(i=0;i<i_t;i++){   //行
            y_t = k1_t;
            for(j=0;j<j_t;j++){
                if(picture[(x_t*320)+y_t] >= 0x5E) m++;
                if(picture[(x_t*320)+y_t+1] >= 0x5E) m++;
                if(picture[((x_t+1)*320)+y_t] >= 0x5E) m++;
                if(picture[((x_t+1)*320)+y_t+1] >= 0x5E) m++;
                if(m>2){
                    //comp_province[i*j_t+j] = 0x11;
                    printf("0x11,");
                    //printf("[]");
                }else{
                    //comp_province[i*j_t+j] = 0xff;
                    printf("0xff,");  
                    //printf("  ");
                }
                y_t+=2;
                m=0;
            }
            x_t+=2;
            printf("\n");
        }
        if(comp_province){
            //i_return = levenshtein((char *)comp_province,(char *)Font_Jing); 
            //printf("comp_province is :%zu\n",i_return);
            printf("comp_province is :%d\n",i_t*j_t+1);
            free(comp_province);
            return i_return;
        }else {
            printf("comp_province is NULL\n");
            free(comp_province);
        } 
    }
    return 0;
}
/*
uint16_t CharacterCompare(char *data,uint8_t BitNum){
    uint16_t i_return=0;
    
    switch(BitNum){
        case 0:{
            if(levenshtein((char *)data,(char *)Font_Jing)<100){
                i_return = 1;
                break;
            }else if(levenshtein((char *)data,(char *)Font_Jin)<100){
                i_return = 2;
                break;
            }else if(levenshtein((char *)data,(char *)Font_Ji)<100){

            }
            break;
        }
        
    }
    return i_return;
}
*/







  







