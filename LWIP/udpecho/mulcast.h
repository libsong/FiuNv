#pragma once

#include "stm32f4xx.h"
#include "lwip/udp.h"  
#include "lwip/pbuf.h"  
#include "lwip/igmp.h"  

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

//
#define  LWIP_DEMO_BUF     1024 

//
#define MULCASTSENDTOIP0 225
#define MULCASTSENDTOIP1 226
#define MULCASTSENDTOIP2 227
#define MULCASTSENDTOIP3 228

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
	u16 devType;
	u8 devName[16];//设备名称
	u8 devSN[3];//暂无，待公司生产标准化
	
	u8 mcuType[16];//mcu类型
	u32 mcuUID[3];//st uid = 96bits
	
	u8 mcuSV[3];// 软件版本
	u8 mcuIP[4];
	u8 mcuNetMask[4];
	u8 mcuGW[4];
	u8 mcuMAC[6];
	u8 mcuHV[2];//母板硬件版本
	
	u8 reserve[1];//暂无
};
#pragma pack()

extern void Multicast_Config(void);
extern void Multicast_Send(void);
extern int McuInfoInit(u8* buf);
extern void Multicast_Send(void);





