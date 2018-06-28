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
extern ETH_HandleTypeDef ETH_Handler;      //��̫�����,lan8720.c�ж���
extern void udpecho_init(void);

//LED����
#define LED_TASK_PRIO		9
#define LED_STK_SIZE		64
OS_STK	LED_TASK_STK[LED_STK_SIZE];
void led_task(void *pdata);  


//1.�������ӳ�ʼ����� 
//2.udp�鲥�Ķ�ʱ�����ϱ����豸�˻�����Ϣ
#define NETINIT_TASK_PRIO		11
#define NETINIT_STK_SIZE		512
OS_STK	NETINIT_TASK_STK[NETINIT_STK_SIZE];
void netinit_task(void *pdata);

//START����
#define START_TASK_PRIO		10
#define START_STK_SIZE		128
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata); 

int main(void)
{  
	u8 rt = 0;
	
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz   
    HAL_Init();                     //��ʼ��HAL��
    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
	
    LED_Init();                     //��ʼ��LED 
    SDRAM_Init();                   //��ʼ��SDRAM
	
    PCF8574_Init();                 //��ʼ��PCF8574
    my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);		    //��ʼ��CCM�ڴ��
    
	printf("FIU MCU\n"); 
	
	rt = EthMemAlloc();
	if(!rt) {
		printf("Eth Mem Alloc suc .\n");
	}
	else
		printf("Eth Mem Alloc failed .\n");

	OSInit(); 					    //UCOS��ʼ��		
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
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
	OSTaskCreate(netinit_task,(void*)0,(OS_STK*)&NETINIT_TASK_STK[NETINIT_STK_SIZE-1],NETINIT_TASK_PRIO);//����LED����
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  		//���ж�
}

//led����
void led_task(void *pdata)
{		
	while(1)
	{
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,500);  //��ʱ500ms
	}
}


//netinit����
void netinit_task(void *pdata)
{
	uint32_t phyreg = 0;
	u8 rt = 0;
	u8 data[MCUMULCASTPACKETLEN] = {0};

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
		McuInfoInit(data);
		
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
					McuInfoInit(data);
					
					printf("Lwip Init Suc .\n"); 
				}
			}
		}
		else {
			//TODO:�鲥�ϱ�
			Multicast_Send();
		}	
		
		OSTimeDlyHMSM(0,0,1,0);  //��ʱ500ms
	}
}







































