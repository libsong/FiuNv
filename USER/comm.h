#pragma once

#include "sys.h"

#define setbit(x,y) x|=(1<<y) //XµÄY ÖÃ 1
#define clrbit(x,y) x&=~(1<<y) //XµÄYÇå0

//flash addr to r/w ip...
#define FLASH_PROCHEAD  0X080F0000   //proc addr start ,sector11
#define FLASH_PROCHEAD_SIZE 48
#define FLASH_PROC_SIZE 64

#define FLASH_IP_PORT  (FLASH_PROCHEAD + 0X30)	//conntent start addr ,(ip+port+netmask+gw)

//flash addr to r/w for REMOTE ID
#define FLASH_PROCHEAD_CAN  (FLASH_PROCHEAD + 0X40)  
//flash addr to r/w for LOCAL ID
#define FLASH_PROCHEAD_CAN_r  (FLASH_PROCHEAD + 0X80)  
//flash addr to r/w for work mode
#define FLASH_PROCHEAD_WM  (FLASH_PROCHEAD + 0XC0)   

//
typedef enum {
	COMM_CMD_TEST,

	COMM_CMD_RELAY_CONF,//hvs
	COMM_CMD_RELAY_ACTIVE,//hvs
	COMM_CMD_RELAY_RESET,//hvs
	COMM_CMD_RELAY_ADC,//hvs GET VAL CUR

	COMM_CMD_RELAY_PWR,//pdo	
	
	COMM_CMD_RELAY_FIU_ACT,//NEW fiu
	
	COMM_CMD_IP = 0xc8,//ip port gw nm, v1.1.0 fiu mofigy ip info. by mulcast
	COMM_CMD_UID = 0xc9, //give mcu uid
	COMM_CMD_SFRST = 0xca,//mcu soft reset,STM32 mcu
}
COMM_CMD;

#define COMM_LEN 128 //MAX LEN COMM WITH THE WIN HOST , when FIU the packet max 12*8 + 27 = 123
#define CANBUFDATALEN 200   //8*n
#define ERRORNUM 80
//
extern uint8_t 	g_IpAllModified;
extern uint16_t 	g_LocalUdpPort;
extern uint8_t 	g_IP_ADDR0;   
extern uint8_t 	g_IP_ADDR1;   
extern uint8_t 	g_IP_ADDR2;   
extern uint8_t 	g_IP_ADDR3; 
extern uint8_t 	g_NETMASK_ADDR0;   
extern uint8_t 	g_NETMASK_ADDR1;   
extern uint8_t 	g_NETMASK_ADDR2;   
extern uint8_t 	g_NETMASK_ADDR3;
extern uint8_t 	g_GW_ADDR0;   
extern uint8_t 	g_GW_ADDR1;   
extern uint8_t 	g_GW_ADDR2;   
extern uint8_t 	g_GW_ADDR3;





