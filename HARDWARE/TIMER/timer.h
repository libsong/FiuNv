#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//��ʱ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
extern TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 
extern TIM_HandleTypeDef TIM2_Handler; 
extern TIM_HandleTypeDef TIM5_Handler; 

void TIM3_Init(u16 arr,uint32_t psc);
void TIM2_Init(uint32_t arr,uint16_t psc);
void TIM5_Init(uint32_t arr,uint16_t psc);
#endif

