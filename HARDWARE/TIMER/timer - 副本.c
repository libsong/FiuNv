#include "timer.h"
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F429开发板
//定时器中断驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern void FiuCom_Process(void);
extern uint8_t		g_FiuByte17Data[17];

TIM_HandleTypeDef TIM10_Handler;         
TIM_OC_InitTypeDef TIM10_CH1Handler;	    
TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 ，用来控制故障激活时间
TIM_HandleTypeDef TIM2_Handler;
TIM_HandleTypeDef TIM5_Handler;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK/2)
void TIM3_Init(u16 arr,uint32_t psc)
{  
    TIM3_Handler.Instance=TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler=psc;                     //分频系数
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period=arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    //HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}


//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
	if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM2_IRQn,1,4);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM2_IRQn);          //开启ITM3中断   
	}
	if(htim->Instance==TIM5)
	{
		__HAL_RCC_TIM5_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM5_IRQn,1,5);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM5_IRQn);          //开启ITM3中断   
	}
}


//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}


//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM3_Handler))
    {
        //故障持续时间到，复位
		//TODO:中断里不应该放置复杂处理，待后续更改
		//HAL_TIM_PWM_Stop(&TIM10_Handler,TIM_CHANNEL_1);
		HAL_TIM_Base_Stop_IT(&TIM2_Handler);
		HAL_TIM_Base_Stop_IT(&TIM5_Handler);
		
		g_FiuByte17Data[0] = 0xbe;
		g_FiuByte17Data[1] = 0xbe;
		
		g_FiuByte17Data[2] = 4;
		
		g_FiuByte17Data[15] = 0xed;
		g_FiuByte17Data[16] = 0xed;
		
		FiuCom_Process();
    }
	if(htim==(&TIM2_Handler)) //后到
    {
		HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);
		HAL_TIM_Base_Start_IT(&TIM5_Handler);
    }
	if(htim==(&TIM5_Handler)) //先到
    {
		HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
		HAL_TIM_Base_Stop_IT(&TIM5_Handler);
    }
}

////timer2 控制周期，当时间到达，PF6 置 1
void TIM2_Init(uint32_t arr,uint16_t psc)
{  
    TIM2_Handler.Instance=TIM2;                          //通用定时器3
    TIM2_Handler.Init.Prescaler=psc;                     //分频系数
    TIM2_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM2_Handler.Init.Period=arr;                        //自动装载值
    TIM2_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM2_Handler);
    
    //HAL_TIM_Base_Start_IT(&TIM2_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM2_Handler);
}

////timer5 控制占空比 当时间到达 PF6 置 0
void TIM5_Init(uint32_t arr,uint16_t psc)
{  
    TIM5_Handler.Instance=TIM5;                          //通用定时器3
    TIM5_Handler.Init.Prescaler=psc;                     //分频系数
    TIM5_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM5_Handler.Init.Period=arr;                        //自动装载值
    TIM5_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM5_Handler);
    
    //HAL_TIM_Base_Start_IT(&TIM2_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}

void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM5_Handler);
}


#if 0
///////// 定时器 10 作频率 占空比等
//TIM3 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM10_PWM_Init(u16 arr,u16 psc)
{ 
    TIM10_Handler.Instance=TIM10;            //定时器3
    TIM10_Handler.Init.Prescaler=psc;       //定时器分频
    TIM10_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//向上计数模式
    TIM10_Handler.Init.Period=arr;          //自动重装载值
    TIM10_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM10_Handler);       //初始化PWM
    
    TIM10_CH1Handler.OCMode=TIM_OCMODE_PWM1; //模式选择PWM1
    TIM10_CH1Handler.Pulse=arr/2;            //设置比较值,此值用来确定占空比，默认比较值为自动重装载值的一半,即占空比为50%
    TIM10_CH1Handler.OCPolarity=TIM_OCPOLARITY_LOW; //输出比较极性为低 
    HAL_TIM_PWM_ConfigChannel(&TIM10_Handler,&TIM10_CH1Handler,TIM_CHANNEL_1);//配置TIM3通道4
	
    HAL_TIM_PWM_Start(&TIM10_Handler,TIM_CHANNEL_1);//开启PWM通道4
}


//定时器底层驱动，时钟使能，引脚配置
//此函数会被HAL_TIM_PWM_Init()调用
//htim:定时器句柄
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_TIM10_CLK_ENABLE();			//使能定时器3
    //__HAL_RCC_GPIOF_CLK_ENABLE();			//开启GPIOB时钟
	
    GPIO_Initure.Pin=GPIO_PIN_6;           	//PB1
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	GPIO_Initure.Alternate= GPIO_AF3_TIM10;	//PB1复用为TIM3_CH4
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);
}


//设置TIM通道4的占空比
//compare:比较值
void TIM_SetTIM10Compare1(u32 compare)
{
	TIM10->CCR4=compare; 
}
#endif