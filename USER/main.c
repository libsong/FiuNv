#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "sdram.h"
#include "lan8720.h"
#include "timer.h"
#include "pcf8574.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "mulcast.h"

//add by lbs
u8 g_NetInitFine = 0;
u8 g_IpAllModified = 0;
extern ETH_HandleTypeDef ETH_Handler;      //以太网句柄,lan8720.c中定义
extern void udpecho_init(void);

//LED任务
#define LED_TASK_PRIO		9
#define LED_STK_SIZE		64
OS_STK	LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);  


//1.网络连接初始化检查 
//2.udp组播的定时发送上报本设备端基本信息
#define NETINIT_TASK_PRIO		11
#define NETINIT_STK_SIZE		512
OS_STK	NETINIT_TASK_STK[NETINIT_STK_SIZE];
void netinit_task(void *pdata);

//START任务
#define START_TASK_PRIO		10
#define START_STK_SIZE		128
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata); 

int main(void)
{  
	u8 rt = 0;
	
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz   
    HAL_Init();                     //初始化HAL库
    delay_init(180);                //初始化延时函数
    uart_init(115200);              //初始化USART
	
    LED_Init();                     //初始化LED 
    SDRAM_Init();                   //初始化SDRAM
	
    PCF8574_Init();                 //初始化PCF8574
    my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMCCM);		    //初始化CCM内存池
    
	printf("FIU MCU\n"); 
	
	rt = EthMemAlloc();
	if(!rt) {
		printf("Eth Mem Alloc suc .\n");
	}
	else
		printf("Eth Mem Alloc failed .\n");

	OSInit(); 					    //UCOS初始化		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //开启UCOS
}

//start任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	
	OSStatInit();  			//初始化统计任务
	OS_ENTER_CRITICAL();  	//关中断
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif
	
	OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);//创建LED任务
	OSTaskCreate(netinit_task,(void*)0,(OS_STK*)&NETINIT_TASK_STK[NETINIT_STK_SIZE-1],NETINIT_TASK_PRIO);//创建LED任务
	OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
	OS_EXIT_CRITICAL();  		//开中断
}

//led任务
void led_task(void *pdata)
{		
	while(1)
	{
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,500);  //延时500ms
	}
}


//netinit任务
void netinit_task(void *pdata)
{
	uint32_t phyreg = 0;
	u8 rt = 0;
	u8 data[MCUMULCASTPACKETLEN] = {0};

	rt = lwip_comm_init();//第一次尝试网络初始化以及相关参数
	if(rt != HAL_OK) {
		printf("Lwip Init failed .\n");
		if(rt == HAL_TIMEOUT)
		{
			printf(("!!! NO Link !!!\n"));				//should not here		
		}
	}				
	else {
		g_NetInitFine = 1;
		
		udpecho_init();
		Multicast_Config();
		McuInfoInit(data);
		
		printf("Lwip Init Suc .\n"); 
	}
	
	while(1)
	{
		if (g_NetInitFine == 0) {//上电时未插网线
			//phy连接检查
			HAL_ETH_ReadPHYRegister(&ETH_Handler, PHY_BSR, &phyreg);
			if ((phyreg & PHY_LINKED_STATUS) != PHY_LINKED_STATUS){
				printf("Net Link check : no link .\n");
			}
			else {
				printf(("!!! Net Link !!!\n"));	
				rt = lwip_comm_init();
				if(rt != HAL_OK) {
					printf("Lwip Init failed .\n");
					if(rt == HAL_TIMEOUT)
					{
						printf(("!!! NO Link !!!\n"));				//should not here		
					}
				}				
				else {
					g_NetInitFine = 1;
					
					udpecho_init();
					Multicast_Config();
					McuInfoInit(data);
					
					printf("Lwip Init Suc .\n"); 
				}
			}
		}
		else {
			//TODO:组播上报
			Multicast_Send();
		}	
		
		OSTimeDlyHMSM(0,0,1,0);  //延时500ms
	}
}







































