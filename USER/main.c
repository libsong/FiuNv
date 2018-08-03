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
#include "comm.h"
#include "stm32f4xx_hal_can.h"
#include "can.h"

extern void FiuCom_Process(void);
//
extern uint8_t g_FiuByte17Data[17];
extern void FiuCom_Process(void);

extern 		uint8_t  g_workStatus;
uint8_t 	g_NetInitFine = 0;
uint8_t 	g_arMcuInfoData[MCUMULCASTPACKETLEN] = {0};
u16 		g_arMcuInfoDataLen = 0;
uint8_t		g_Error = 0;

extern ETH_HandleTypeDef ETH_Handler;      //以太网句柄,lan8720.c中定义
extern void udpecho_init(void);
extern void ErrorNumInit(void);
extern void GlobalDataInit(void);

//
uint8_t 	g_IpAllModified = 1;
uint16_t 	g_LocalUdpPort = 50099;
uint8_t 	g_IP_ADDR0 = 192;   
uint8_t 	g_IP_ADDR1 = 168;   
uint8_t 	g_IP_ADDR2 = 1;   
uint8_t 	g_IP_ADDR3 = 100; 
uint8_t 	g_NETMASK_ADDR0 = 255;   
uint8_t 	g_NETMASK_ADDR1 = 255;   
uint8_t 	g_NETMASK_ADDR2 = 255;   
uint8_t 	g_NETMASK_ADDR3 = 0;
uint8_t 	g_GW_ADDR0 = 192;   
uint8_t 	g_GW_ADDR1 = 168;   
uint8_t 	g_GW_ADDR2 = 1;   
uint8_t 	g_GW_ADDR3 = 1;
extern void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);
void IpModifiedCheck(void);

//
uint8_t 	g_CanAllModified = 1;
extern uint32_t    g_RemoteCanId;
extern uint32_t    g_LocalCanId_r;
extern int HostCanDataAnalysis(void);

//CAN任务
#define CAN_TASK_PRIO		20 //任务中是无间隔无限循环
#define CAN_STK_SIZE		128
OS_STK	CAN_TASK_STK[CAN_STK_SIZE];
void CanDataDealTask(void *pdata);
void CanModifiedCheck(void);

//LED任务
#define LED_TASK_PRIO		9
#define LED_STK_SIZE		256
OS_STK	LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);  


//1.网络连接初始化检查 
//2.udp组播的定时发送上报本设备端基本信息
#define NETINIT_TASK_PRIO		11
#define NETINIT_STK_SIZE		2018
OS_STK	NETINIT_TASK_STK[NETINIT_STK_SIZE];
void netinit_task(void *pdata);

//START任务
#define START_TASK_PRIO		10
#define START_STK_SIZE		512
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata); 

//
uint8_t Fuse_check(void);
void Error_loop(void);	


extern void TIM_SetTIM10Compare1(u32 compare);
extern void TIM10_PWM_Init(u16 arr,u16 psc);

int main(void)
{  
	uint8_t rt = 0;
	
	ErrorNumInit();
	GlobalDataInit();
	
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz   
    HAL_Init();                     //初始化HAL库
    delay_init(180);                //初始化延时函数
    uart_init(115200);              //初始化USART
	
    LED_Init();                     //初始化LED 以及 IO 等
    SDRAM_Init();                   //初始化SDRAM
	
    PCF8574_Init();                 //初始化PCF8574
    my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMCCM);		    //初始化CCM内存池
    
	printf("FIU MCU\n"); 
	
	//保险丝检测
	rt = 0;
	rt = Fuse_check();
	if (rt != 0) {
		printf("Fuse_check failed = 0x%x .\n",rt);
		goto ERROR;
	}
	
	//温度等io扩展,器件类型 KSD-01F H75 常开 ， 常温读取是 1
	if (!PCF8574_ReadBit(TEMP1_IO) || !PCF8574_ReadBit(TEMP2_IO)) {
		goto ERROR;	
	}
	
	//can
	CanModifiedCheck();
	CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_8TQ,3,CAN_MODE_NORMAL); // 1M baud
	
	//网络ip相关
	IpModifiedCheck();
	rt = EthMemAlloc();
	if(!rt) {
		printf("Eth Mem Alloc suc .\n");
	}
	else
		printf("Eth Mem Alloc failed .\n");
	
	//系统相关
	OSInit(); 					    //UCOS初始化		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //开启UCOS
	
	ERROR:
		Error_loop();
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
	OSTaskCreate(CanDataDealTask,(void*)0,(OS_STK*)&CAN_TASK_STK[CAN_STK_SIZE-1],CAN_TASK_PRIO);
	OSTaskCreate(netinit_task,(void*)0,(OS_STK*)&NETINIT_TASK_STK[NETINIT_STK_SIZE-1],NETINIT_TASK_PRIO);
	
	OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
	OS_EXIT_CRITICAL();  		//开中断
}

//led任务
void led_task(void *pdata)
{		
	PCF8574_WriteBit(RUNLED_IO,1);	//front board run led
	
	while(1)
	{
		LED_MCU = !LED_MCU; //mcu core board led6 
		
		if (!PCF8574_ReadBit(TEMP1_IO) || !PCF8574_ReadBit(TEMP2_IO)) {
			Error_loop();			
		}		
		
		OSTimeDlyHMSM(0,0,0,500);  //延时500ms
	}
}

void CanDataDealTask(void *pdata)
{
	while (1) {
		HostCanDataAnalysis();
		//OSTimeDlyHMSM(0,0,0,1);
	}
}


//netinit任务
void netinit_task(void *pdata)
{
	uint32_t phyreg = 0;
	uint8_t rt = 0;
	

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
		g_arMcuInfoDataLen = McuInfoInit(g_arMcuInfoData);
		
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
					g_arMcuInfoDataLen = McuInfoInit(g_arMcuInfoData);
					
					printf("Lwip Init Suc .\n"); 
				}
			}
		}
		else {
			Multicast_Send(g_arMcuInfoData,g_arMcuInfoDataLen);
			printf("g_arMcuInfoDataLen = %d .\n",g_arMcuInfoDataLen); 
			
			//软复位检测
			rt = __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST);
			if (rt) {
		//		HAL_ETH_WritePHYRegister(&ETH_Handler, PHY_BCR,PHY_RESET);
		//		delay_ms(100);
				
				PCF8574_WriteBit(SOFTRSTLED_IO,1);	//front board run led.
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//buzzer
				delay_ms(500);
				PCF8574_WriteBit(SOFTRSTLED_IO,1);	
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//buzzer
				
				printf("Device SFRST done .\n");
				__HAL_RCC_CLEAR_RESET_FLAGS();
			}
		}	
		
		OSTimeDlyHMSM(0,0,1,0);  //1s
	}
}

void IpModifiedCheck(void)
{
	uint8_t tmp[FLASH_PROCHEAD_SIZE];
	int i;
	
	//read flash to check if ip... modified or not
	//20's 0x55aa present modified
	STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp,FLASH_PROCHEAD_SIZE/4);
	for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
		if (tmp[i] == 0xaa && tmp[i+1] == 0x55) {
			
		}
		else {
			g_IpAllModified = 0;
		}
	}
	if (g_IpAllModified == 1) {
		STMFLASH_Read(FLASH_IP_PORT,(u32 *)tmp,2);
		g_IP_ADDR0 = tmp[0];   
		g_IP_ADDR1 = tmp[1];   
		g_IP_ADDR2 = tmp[2];  
		g_IP_ADDR3 = tmp[3];
		
		g_LocalUdpPort = 50099; // fiu host only surpport 50099
		g_NETMASK_ADDR0 = 255;
		g_NETMASK_ADDR1 = 255;
		g_NETMASK_ADDR2 = 255;
		g_NETMASK_ADDR3 = 0;
		g_GW_ADDR0 = tmp[0];
		g_GW_ADDR1 = tmp[1];
		g_GW_ADDR2 = tmp[2];
		g_GW_ADDR3 = 1;
	}
}

void CanModifiedCheck(void)
{
	uint8_t tmp[FLASH_PROC_SIZE] = {0};
	int i;
	
	//read flash to check if ip... modified or not
	//20's 0x55aa present modified
	STMFLASH_Read(FLASH_PROCHEAD_CAN,(u32 *)tmp,FLASH_PROC_SIZE/4);
	for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
		if (tmp[i] == 0xaa && tmp[i+1] == 0x55) {
			
		}
		else {
			g_CanAllModified = 0;
		}
	}
	if (g_CanAllModified == 1) {		
		g_RemoteCanId = (tmp[51]<<24) | (tmp[50]<<16) | (tmp[49]<<8) | tmp[48];
	}
	
	g_CanAllModified = 1;
	STMFLASH_Read(FLASH_PROCHEAD_CAN_r,(u32 *)tmp,FLASH_PROC_SIZE/4);
	for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
		if (tmp[i] == 0xaa && tmp[i+1] == 0x55) {
			
		}
		else {
			g_CanAllModified = 0;
		}
	}
	if (g_CanAllModified == 1) {		
		g_LocalCanId_r = (tmp[51]<<24) | (tmp[50]<<16) | (tmp[49]<<8) | tmp[48]; 
	}
	
}

uint8_t Fuse_check(void) 
{
	uint8_t fuse_pinVal = 1;
	uint8_t fuse_status = 0;
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);//fuse  1
	delay_ms(50);
	fuse_pinVal = HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_11);
	if (fuse_pinVal == 0)
		setbit(fuse_status,0);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);//fuse  2
	fuse_pinVal = 1;
	delay_ms(50);
	fuse_pinVal = HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_10);
	if (fuse_pinVal == 0)
		setbit(fuse_status,1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);//fuse  3
	fuse_pinVal = 1;
	delay_ms(50);
	fuse_pinVal = HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_9);
	if (fuse_pinVal == 0)
		setbit(fuse_status,2);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//fuse  4
	fuse_pinVal = 1;
	delay_ms(50);
	fuse_pinVal = HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_8);
	if (fuse_pinVal == 0)
		setbit(fuse_status,3);
	
	HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_SET);//fuse  5
	fuse_pinVal = 1;
	delay_ms(50);
	fuse_pinVal = HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_7);
	if (fuse_pinVal == 0)
		setbit(fuse_status,4);
	
	return fuse_status;
}

void Error_loop(void)
{
	g_Error = 1;
	HAL_GPIO_WritePin(GPIOH,GPIO_PIN_3,GPIO_PIN_RESET);//error protect relay
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//buzzer
	PCF8574_WriteBit(ERRORLED_IO,1);	//front board error led
	PCF8574_WriteBit(RUNLED_IO,0);	//front board run led
	
	while (1) {
		printf("error loop - ing\n");
		delay_ms(500);
		PCF8574_WriteBit(RUNLED_IO,1);	//front board run led
		delay_ms(500);
		PCF8574_WriteBit(RUNLED_IO,0);	//front board run led
	}		
}































