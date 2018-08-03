#pragma once

#include "stm32f4xx.h"
#include "lwip/udp.h"  
#include "lwip/pbuf.h"  
#include "lwip/igmp.h"  

typedef uint32_t  	u32;
typedef uint16_t 	u16;
typedef uint8_t  	u8;
//
#define  LWIP_DEMO_BUF     512 

//
#define MULCASTSENDTOIP0 225
#define MULCASTSENDTOIP1 226
#define MULCASTSENDTOIP2 227
#define MULCASTSENDTOIP3 228

#define MULCASTRECVIP0 235
#define MULCASTRECVIP1 236
#define MULCASTRECVIP2 237
#define MULCASTRECVIP3 238

#define UDP_MULTICASE_RECV_PORT 45654    // multicast port for recive  
#define UDP_MULTICASE_SEND_PORT 54345   // multicast port for send 

//
#define MCUPACKETTYPE 1	//数据包种类 1设备发基本信息
#define DEVTYPECODE 0x0200 //设备种类标识，0x0100电源类 0x0200故障类 0x0300高压模拟类
#define DEVNAME "FIU"
#define MCUTYPE "STM32F429"
#define DEVSN "SN0"

#define MCUSV_MAJOR 1	//主版本号 
#define MCUSV_MINOR 0  //次版本
#define MCUSV_DEBUG 0  //bug修改

#define MCUHV_MAJOR 1	//主版本号 
#define MCUHV_MINOR 0  //次版本

//define the slave info 
#define MCUMULCASTPACKETLEN 200
#pragma pack(1)
struct mcuinfo{
	u16		devType;
	u16		port;
	int8_t	devName[32];
	int8_t	devSN[16];
	int8_t	mcuType[16]; 
	uint32_t mcuUID[3];
	u8		mcuSV[3];
	u8		mcuIP[4];
	u8		mcuNetMask[4];
	u8		mcuGW[4]; 
	u8		mcuMAC[6]; 
	u8		mcuHV[2];
	u8		reserve[1];
};
#pragma pack()

extern void Multicast_Config(void);
extern void Multicast_Send(unsigned char * data,unsigned short len);
extern int McuInfoInit(u8* buf);





