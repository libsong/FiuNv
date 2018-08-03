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

extern ETH_HandleTypeDef ETH_Handler;      //��̫�����,lan8720.c�ж���
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

//CAN����
#define CAN_TASK_PRIO		20 //���������޼������ѭ��
#define CAN_STK_SIZE		128
OS_STK	CAN_TASK_STK[CAN_STK_SIZE];
void CanDataDealTask(void *pdata);
void CanModifiedCheck(void);

//LED����
#define LED_TASK_PRIO		9
#define LED_STK_SIZE		256
OS_STK	LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);  


//1.�������ӳ�ʼ����� 
//2.udp�鲥�Ķ�ʱ�����ϱ����豸�˻�����Ϣ
#define NETINIT_TASK_PRIO		11
#define NETINIT_STK_SIZE		2018
OS_STK	NETINIT_TASK_STK[NETINIT_STK_SIZE];
void netinit_task(void *pdata);

//START����
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
	
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz   
    HAL_Init();                     //��ʼ��HAL��
    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
	
    LED_Init();                     //��ʼ��LED �Լ� IO ��
    SDRAM_Init();                   //��ʼ��SDRAM
	
    PCF8574_Init();                 //��ʼ��PCF8574
    my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);		    //��ʼ��CCM�ڴ��
    
	printf("FIU MCU\n"); 
	
	//����˿���
	rt = 0;
	rt = Fuse_check();
	if (rt != 0) {
		printf("Fuse_check failed = 0x%x .\n",rt);
		goto ERROR;
	}
	
	//�¶ȵ�io��չ,�������� KSD-01F H75 ���� �� ���¶�ȡ�� 1
	if (!PCF8574_ReadBit(TEMP1_IO) || !PCF8574_ReadBit(TEMP2_IO)) {
		goto ERROR;	
	}
	
	//can
	CanModifiedCheck();
	CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_8TQ,3,CAN_MODE_NORMAL); // 1M baud
	
	//����ip���
	IpModifiedCheck();
	rt = EthMemAlloc();
	if(!rt) {
		printf("Eth Mem Alloc suc .\n");
	}
	else
		printf("Eth Mem Alloc failed .\n");
	
	//ϵͳ���
	OSInit(); 					    //UCOS��ʼ��		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
	
	ERROR:
		Error_loop();
}

//start����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
#endif
	
	OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);//����LED����
	OSTaskCreate(CanDataDealTask,(void*)0,(OS_STK*)&CAN_TASK_STK[CAN_STK_SIZE-1],CAN_TASK_PRIO);
	OSTaskCreate(netinit_task,(void*)0,(OS_STK*)&NETINIT_TASK_STK[NETINIT_STK_SIZE-1],NETINIT_TASK_PRIO);
	
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  		//���ж�
}

//led����
void led_task(void *pdata)
{		
	PCF8574_WriteBit(RUNLED_IO,1);	//front board run led
	
	while(1)
	{
		LED_MCU = !LED_MCU; //mcu core board led6 
		
		if (!PCF8574_ReadBit(TEMP1_IO) || !PCF8574_ReadBit(TEMP2_IO)) {
			Error_loop();			
		}		
		
		OSTimeDlyHMSM(0,0,0,500);  //��ʱ500ms
	}
}

void CanDataDealTask(void *pdata)
{
	while (1) {
		HostCanDataAnalysis();
		//OSTimeDlyHMSM(0,0,0,1);
	}
}


//netinit����
void netinit_task(void *pdata)
{
	uint32_t phyreg = 0;
	uint8_t rt = 0;
	

	rt = lwip_comm_init();//��һ�γ��������ʼ���Լ���ز���
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
		if (g_NetInitFine == 0) {//�ϵ�ʱδ������
			//phy���Ӽ��
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
			
			//��λ���
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































