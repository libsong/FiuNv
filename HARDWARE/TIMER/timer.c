#include "timer.h"
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//��ʱ���ж���������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern uint8_t g_bLoose;
extern uint8_t g_workStatus;

TIM_HandleTypeDef TIM10_Handler;//��ʱ����� ���������ƹ��ϼ���ʱ��
TIM_OC_InitTypeDef TIM10_CH1Handler;
TIM_HandleTypeDef TIM2_Handler;      //��ʱ����� 

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!(��ʱ��3����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM2_Init(uint32_t arr,uint16_t psc)
{  
    TIM2_Handler.Instance=TIM2;                          //ͨ�ö�ʱ��3
    TIM2_Handler.Init.Prescaler=psc-1;                     //��Ƶϵ��
    TIM2_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM2_Handler.Init.Period=arr-1;                        //�Զ�װ��ֵ
    TIM2_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM2_Handler);
    
	__HAL_TIM_CLEAR_IT(&TIM2_Handler, TIM_IT_UPDATE);//���ⶨʱ�� һ�������������ж�
    HAL_TIM_Base_Start_IT(&TIM2_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE   
}


//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();            //ʹ��TIM2ʱ��
		HAL_NVIC_SetPriority(TIM2_IRQn,1,3);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM2_IRQn);          //����ITM3�ж�   
	}
}


//��ʱ��3�жϷ�����
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM2_Handler);
}


//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_Initure;
	
    if(htim==(&TIM2_Handler))
    {		
		if (g_bLoose == 1) {
			HAL_TIM_PWM_Stop(&TIM10_Handler,TIM_CHANNEL_1);
			HAL_TIM_PWM_DeInit(&TIM10_Handler);
		}
		GPIO_Initure.Pin = GPIO_PIN_6 ; 
		GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
		GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
		GPIO_Initure.Pull=GPIO_PULLDOWN;
		HAL_GPIO_Init(GPIOF,&GPIO_Initure);
		HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
		
		HAL_TIM_Base_Stop_IT(&TIM2_Handler);
		HAL_TIM_Base_DeInit(&TIM2_Handler);
		
		g_workStatus = 0;
    }
}

#if 1
///////// ��ʱ�� 10 ��Ƶ�� ռ�ձȵ�
void TIM10_PWM_Init(u16 arr,u16 psc,uint32_t ccr)
{ 
    TIM10_Handler.Instance=TIM10;            //��ʱ��3
    TIM10_Handler.Init.Prescaler=psc;       //��ʱ����Ƶ
    TIM10_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//���ϼ���ģʽ
    TIM10_Handler.Init.Period=arr-1;          //�Զ���װ��ֵ
    TIM10_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM10_Handler);       //��ʼ��PWM
    
    TIM10_CH1Handler.OCMode=TIM_OCMODE_PWM1; //ģʽѡ��PWM1
    TIM10_CH1Handler.Pulse=ccr-1;            //���ñȽ�ֵ,��ֵ����ȷ��ռ�ձȣ�Ĭ�ϱȽ�ֵΪ�Զ���װ��ֵ��һ��,��ռ�ձ�Ϊ50%
    TIM10_CH1Handler.OCPolarity=TIM_OCPOLARITY_LOW; //����Ƚϼ���Ϊ�� 
    HAL_TIM_PWM_ConfigChannel(&TIM10_Handler,&TIM10_CH1Handler,TIM_CHANNEL_1);//����TIM3ͨ��4
	
    HAL_TIM_PWM_Start(&TIM10_Handler,TIM_CHANNEL_1);//����PWMͨ��4
}


//��ʱ���ײ�������ʱ��ʹ�ܣ���������
//�˺����ᱻHAL_TIM_PWM_Init()����
//htim:��ʱ�����
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_TIM10_CLK_ENABLE();			//ʹ�ܶ�ʱ��3
    __HAL_RCC_GPIOF_CLK_ENABLE();			//����GPIOBʱ��
	
    GPIO_Initure.Pin=GPIO_PIN_6;           	//PB1
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//�����������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	GPIO_Initure.Alternate= GPIO_AF3_TIM10;	//PB1����ΪTIM3_CH4
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);
	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
}


//����TIMͨ��4��ռ�ձ�
//compare:�Ƚ�ֵ
void TIM_SetTIM10Compare1(u32 compare)
{
	TIM10->CCR4=compare; 
}
#endif