#include <stm32f4xx.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mulcast.h"  
  
struct udp_pcb* udp_server_multi_pcb;//组播pcb控制块
struct ip_addr ipgroup_rev,ipgroup_send;  
struct mcuinfo g_MCUinfo;  
  
u16_t lwip_demo_buf_len = 0;  
u8_t lwip_demo_buf[LWIP_DEMO_BUF];  
  
//send
void multicast_send_data(unsigned char * data,unsigned short len)  
{  
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);  
  err_t err = 0;
	
	if(udp_server_multi_pcb!=NULL){ 
		memcpy(p->payload, data, len);  
		  
		err = udp_sendto(udp_server_multi_pcb, p,(struct ip_addr *) (&ipgroup_send),UDP_MULTICASE_SEND_PORT); 
		printf("multicast_send_data err = %d \n",err);
	}
		pbuf_free(p);  
}  
  
   
void Multicast_Config(void)   
{  
	int i;   
	err_t err;
	
	//	IP4_ADDR(&ipgroup_rev, 230,1,1,11);//用于接收组播的地址
	IP4_ADDR(&ipgroup_send, MULCASTSENDTOIP0,MULCASTSENDTOIP1,MULCASTSENDTOIP2,MULCASTSENDTOIP3);//用于发送组播的地址  

//	err = igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipgroup_rev));//只需要将接收地址放入igmp组，发送不需要	
//		if (err != 0) {
//			printf("igmp_joingroup failed .\n");
//		}
		
	udp_server_multi_pcb = udp_new();   
	if(udp_server_multi_pcb!=NULL){  
		printf("mulcast pcb new suc .\n");
	//		udp_bind(udp_server_multi_pcb,IP_ADDR_ANY,UDP_MULTICASE_RECV_PORT);//组播接收地址
	//		udp_recv(udp_server_multi_pcb,udp_server_rev,NULL);//  
	}  
}  
  
void Multicast_Send(void)  
{   
	   multicast_send_data(lwip_demo_buf,lwip_demo_buf_len);  
}  

int McuInfoInit(u8* buf)
{
	u8 tmp[128];
	int i,j;
	u16 len = 0;
	u8 crc = 0;
	
	for (i=0;i<8;i++) {
		tmp[i] = 0xbe;
	}
	for (i=8;i<16;i++) {
		tmp[i] = 0xaa;
	}
	
	g_MCUinfo.devType = DEVTYPECODE;
	memcpy(g_MCUinfo.devName,DEVNAME,strlen(DEVNAME));
	memcpy(g_MCUinfo.devSN,DEVSN,strlen(DEVSN));
	memcpy(g_MCUinfo.mcuType,MCUTYPE,strlen(MCUTYPE));
	g_MCUinfo.mcuSV[0] = MCUSV_MAJOR;
	g_MCUinfo.mcuSV[1] = MCUSV_MINOR;
	g_MCUinfo.mcuSV[2] = MCUSV_DEBUG;
	g_MCUinfo.mcuHV[0] = MCUHV_MAJOR;
	g_MCUinfo.mcuHV[1] = MCUHV_MINOR;
	
	len = sizeof(struct mcuinfo);
	memcpy(&tmp[16],&len,2);
	
	tmp[18] = MCUPACKETTYPE;

	memcpy(&tmp[19],&g_MCUinfo,len);
	for (i=19;i<(19+len);i++) {
		crc += tmp[i];
	}
	tmp[i++] = crc;
	
	for (j=i;j<(i+8);j++){
		tmp[j] = 0x55;
	}
	for (j=i+8;j<(i+8+8);j++){
		tmp[j] = 0xed;
	}
	
	memcpy(buf,tmp,(8+8+2+1+len+1+8+8));
	
	return (8+8+2+1+len+1+8+8);
}

//组播接收回调函数
//void udp_server_rev(void* arg,struct udp_pcb* upcb,struct pbuf* p,struct ip_addr*addr ,u16_t port){  
//  
//	int i,j;  
//	  
//	if(p!=NULL){  
//		if((p->tot_len)>=LWIP_DEMO_BUF){         //如果长度过长则额外处理        
//			memcpy(lwip_demo_buf,p->payload,LWIP_DEMO_BUF);  
//			lwip_demo_buf_len = LWIP_DEMO_BUF;  
//		}else{                
//			memcpy(lwip_demo_buf,p->payload,p->tot_len);  
//			lwip_demo_buf_len = p->tot_len;  
//		}  
//	  
//	 for(i=0;i<p->tot_len;i++)//测试组播时，有时候即使没发出去也可能显示收到，加2以区别
//		{  
//			printf("%02x  ",lwip_demo_buf[i]);  
//		  lwip_demo_buf[i]=lwip_demo_buf[i]+2;  
//		}  
//  
//		printf("\n");   
//  
//	}   
//}  




