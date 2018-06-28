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
#define MCUPACKETTYPE 1	//���ݰ����� 1�豸��������Ϣ
#define DEVTYPECODE 0x0200 //�豸�����ʶ��0x0100��Դ�� 0x0200������ 0x0300��ѹģ����
#define DEVNAME "FIU"
#define MCUTYPE "STM32F429"
#define DEVSN "SN0"

#define MCUSV_MAJOR 1	//���汾�� 
#define MCUSV_MINOR 0  //�ΰ汾
#define MCUSV_DEBUG 0  //bug�޸�

#define MCUHV_MAJOR 1	//���汾�� 
#define MCUHV_MINOR 0  //�ΰ汾

//define the slave info 
#define MCUMULCASTPACKETLEN 200
#pragma pack(1)
struct mcuinfo{
	u16 devType;
	u8 devName[16];//�豸����
	u8 devSN[3];//���ޣ�����˾������׼��
	
	u8 mcuType[16];//mcu����
	u32 mcuUID[3];//st uid = 96bits
	
	u8 mcuSV[3];// ����汾
	u8 mcuIP[4];
	u8 mcuNetMask[4];
	u8 mcuGW[4];
	u8 mcuMAC[6];
	u8 mcuHV[2];//ĸ��Ӳ���汾
	
	u8 reserve[1];//����
};
#pragma pack()

extern void Multicast_Config(void);
extern void Multicast_Send(void);
extern int McuInfoInit(u8* buf);
extern void Multicast_Send(void);





