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
//设置DHCP任务堆栈大小
#define LWIP_DHCP_STK_SIZE  		    128
//设置任务优先级 在app_cfg.h中定义了
//#define DHCP_THREAD_PRIO       		5
#define LWIP_MAX_DHCP_TRIES		4   //DHCP服务器最大重试次数
#endif

//lwip控制结构体
typedef struct  
{
//	u8_t mac[6];      //MAC地址	在ETH_HandleStructure变量中
	u8_t remoteip[4];	//远端主机IP地址 
	u8_t ip[4];       //本机IP地址
	u8_t netmask[4]; 	//子网掩码
	u8_t gateway[4]; 	//默认网关的IP地址
	__IO u8_t init_ok;	//lwip初始化成功时置位	
	__IO u8_t dhcpstatus;	//dhcp状态 
//					//0,未获取DHCP地址;
//					//1,进入DHCP获取状态
//					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;

//传递给tcp server的连接子程序的参数
typedef struct  
{
	//tcp server 创建的新连接的netconn
	struct netconn *newconn;
	//子任务的TCB，采用内存管理的方式控制申请	
	OS_TCB *Child_TaskTCB;	
	//任务堆栈，采用内存管理的方式控制申请	
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

#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHY芯片地址.
#define LAN8720_RST 		   	PDout(3) 			//LAN8720复位引脚	 

extern ETH_HandleTypeDef ETH_HandleStructure;
extern uint8_t Eth_Rx_Buff[][ETH_RX_BUF_SIZE]; 							//以太网DMA接收描述符指向的接收缓冲区空间
extern uint8_t Eth_Tx_Buff[][ETH_TX_BUF_SIZE]; 							//以太网DMA发送描述符指向的发送缓冲区空间
extern ETH_DMADescTypeDef Eth_RxDesc[];		//以太网DMA接收描述符数组空间
extern ETH_DMADescTypeDef Eth_TxDesc[];		//以太网DMA发送描述符数组空间

extern u8_t *memp_memory;				//在memp.c里面定义.
extern u8_t *ram_heap;					//在mem.c里面定义.
extern __lwip_dev lwipdev;	//lwip控制结构体
extern struct netif lwip_netif;				//定义一个全局的网络接口

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
//extern u32_t  memp_get_memorysize(void);	//在memp.c里面定义
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

