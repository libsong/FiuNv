/*
*********************************************************************************************************
*
*                                               wt'LWIP
*                                           Application Code
*
* Filename      : app_lwip.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               APP_WT_SERIAL_MODULE_PRESENT present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  APP_LWIP_MODULE_PRESENT                              /* See Note #1.                                         */
#define  APP_LWIP_MODULE_PRESENT


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/
//#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "lwip/timers.h"
#include "malloc.h"
#include "delay.h"
#include "usart.h" 
#include "pcf8574.h"
#include <stdio.h>


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   APP_LWIP_MODULE
#define  APP_LWIP_MODULE_EXT
#else
#define  APP_LWIP_MODULE_EXT  extern
#endif

/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/

#ifndef  APP_CFG_LWIP_EN
#define  APP_CFG_LWIP_EN                   DEF_DISABLED
#endif


#if (APP_CFG_LWIP_EN == DEF_ENABLED)
/*
*********************************************************************************************************
*                                      CONDITIONAL INCLUDE FILES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                 DEFINES
*********************************************************************************************************
*/

#if(LWIP_DHCP == 1)
//����DHCP�����ջ��С
#define LWIP_DHCP_STK_SIZE  		    128
//�����������ȼ� ��app_cfg.h�ж�����
//#define DHCP_THREAD_PRIO       		5
#define LWIP_MAX_DHCP_TRIES		4   //DHCP������������Դ���
#endif

//lwip���ƽṹ��
typedef struct  
{
//	u8_t mac[6];      //MAC��ַ	��ETH_HandleStructure������
	u8_t remoteip[4];	//Զ������IP��ַ 
	u8_t ip[4];       //����IP��ַ
	u8_t netmask[4]; 	//��������
	u8_t gateway[4]; 	//Ĭ�����ص�IP��ַ
	__IO u8_t init_ok;	//lwip��ʼ���ɹ�ʱ��λ	
	__IO u8_t dhcpstatus;	//dhcp״̬ 
//					//0,δ��ȡDHCP��ַ;
//					//1,����DHCP��ȡ״̬
//					//2,�ɹ���ȡDHCP��ַ
					//0XFF,��ȡʧ��.
}__lwip_dev;

//���ݸ�tcp server�������ӳ���Ĳ���
typedef struct  
{
	//tcp server �����������ӵ�netconn
	struct netconn *newconn;
	//�������TCB�������ڴ����ķ�ʽ��������	
	OS_TCB *Child_TaskTCB;	
	//�����ջ�������ڴ����ķ�ʽ��������	
	CPU_STK *Child_TASK_STK;	
}__tcp_child_arg;
/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHYоƬ��ַ.
#define LAN8720_RST 		   	PDout(3) 			//LAN8720��λ����	 

extern ETH_HandleTypeDef ETH_HandleStructure;
extern uint8_t Eth_Rx_Buff[][ETH_RX_BUF_SIZE]; 							//��̫��DMA����������ָ��Ľ��ջ������ռ�
extern uint8_t Eth_Tx_Buff[][ETH_TX_BUF_SIZE]; 							//��̫��DMA����������ָ��ķ��ͻ������ռ�
extern ETH_DMADescTypeDef Eth_RxDesc[];		//��̫��DMA��������������ռ�
extern ETH_DMADescTypeDef Eth_TxDesc[];		//��̫��DMA��������������ռ�

extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.
extern __lwip_dev lwipdev;	//lwip���ƽṹ��
extern struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

#if(LWIP_DHCP == 1)
extern OS_TCB *LWIP_DHCPTaskTCB;
#endif

/*
*********************************************************************************************************
*                                                 MACRO'S
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

extern HAL_StatusTypeDef App_Lwip_Init(void);
//extern u32_t  memp_get_memorysize(void);	//��memp.c���涨��
extern void HAL_ETH_MspInit(ETH_HandleTypeDef *heth);
extern HAL_StatusTypeDef  ETH_PHY_LAN8720_Init (void);
extern void lwip_reset_netif_ipaddr(__lwip_dev *lwipdev);
extern void Lwip_netlink_status_check(void);
/*
extern uint16_t USMART_ETH_ReadPHYRegister(uint16_t PHYReg);
extern uint16_t USMART_ETH_WritePHYRegister(uint16_t PHYReg, uint32_t RegValue);
*/	
extern void lwip_comm_dhcp_creat(void);
#endif

#endif

