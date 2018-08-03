/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "sys.h"
#include <string.h>
#include "udpecho.h"
#include "lwipopts.h"
#include "err.h"
#include "api.h"
#include "comm.h"
#include "mulcast.h"

extern uint8_t 		g_HOSTCANDATABUF[CANBUFDATALEN];
extern uint32_t 	g_DealDataCur;
extern uint32_t 	g_IntrDataCur;
extern int HostCanDataAnalysis(void);
extern void HostCanDataAnalysisNoLoopBuf(uint8_t *data);
   
u8 FiuHostConfig[17] = {0};
extern struct mcuinfo g_MCUinfo; 

#if LWIP_NETCONN

extern u8 		g_IpAllModified;
extern u16 		g_LocalUdpPort;
extern uint8_t 	g_Error;

//static struct netconn 	*conn;
static struct netbuf  	*buf;
//static struct ip_addr 	*addr;
//static unsigned short 	port;

extern void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);
extern void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite);

void FiuRelayPinActive(uint8_t *data, int len)	//  
{
	int i,loop = len/8;
	
	for (i=0;i<loop;i++) {
		HostCanDataAnalysisNoLoopBuf(&data[i*8]);
	}
}

void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
  /* Tell the client that we have accepted it */
	u8_t	tmp_crc = 0;
	u8_t	tmp_buf[COMM_LEN];
	u8_t	host_cmd = 0xff;
	u8_t	host_len = 0;
	s32_t   i;
	err_t   recv_err = ERR_OK;
	u8 		tmp_flash[FLASH_PROC_SIZE];

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//add our data deal proc start.
	tmp_crc = 0;
	memset(tmp_buf,0x0,COMM_LEN);					
	memcpy(tmp_buf,p->payload, p->len);
		
	for (i = 0; i < 8; i++) {
			if (tmp_buf[i] != 0xbe) {
			recv_err = ERR_KLDATA;
		}
	}

	if (recv_err == ERR_OK) 
	{
		host_cmd = tmp_buf[8];
		host_len = tmp_buf[9];
		for (i = 10; i < 10+host_len; i++) {
		  tmp_crc += tmp_buf[i];
		}
//		if (tmp_crc != tmp_buf[10+host_len]) {
//		  recv_err = ERR_KLDATA; 
//		}
	}

	if (recv_err == ERR_OK) 
	{
		for (i = 11+host_len; i < 19+host_len; i++) {
		  if (tmp_buf[i] != 0xff) {
			recv_err = ERR_KLDATA;
		  }
		}
	}
	if (recv_err == ERR_OK) 
	{
		for (i = 19+host_len; i < 27+host_len; i++) {
		  if (tmp_buf[i] != 0xed) {
			recv_err = ERR_KLDATA;
		  }
		}
	}	
					
	if (recv_err == ERR_OK) 
	{
		tmp_crc = 0;
		
		switch(host_cmd){
		case COMM_CMD_TEST:
			for (i = 10; i<10+host_len; i++) {
				tmp_buf[i] += 1;
				tmp_crc += tmp_buf[i];
			}
			tmp_buf[10+host_len] = tmp_crc;
			memcpy(p->payload, tmp_buf, 27+host_len);
		break;

		case COMM_CMD_UID:
					memcpy(&tmp_buf[10],&g_MCUinfo.mcuUID[0],12);
					for (i = 10; i<10+host_len; i++) {
						tmp_crc += tmp_buf[i];
					}
					tmp_buf[10+host_len] = tmp_crc;
					memcpy(buf->p->payload, tmp_buf, 27+host_len);
				break;
				
		case COMM_CMD_SFRST:
			__set_FAULTMASK(1);
			HAL_NVIC_SystemReset();
		break;
		
			
		case COMM_CMD_RELAY_FIU_ACT:
			FiuRelayPinActive(&tmp_buf[10],host_len);
		break;
		
			
		case COMM_CMD_IP:
			STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp_flash,FLASH_PROC_SIZE/4);
			for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
				tmp_flash[i] = 0xaa; 
				tmp_flash[i+1] = 0x55;
			}
			
			tmp_flash[64] = tmp_buf[10];
			tmp_flash[65] = tmp_buf[11];
			tmp_flash[66] = tmp_buf[12];
			tmp_flash[67] = tmp_buf[13];
			tmp_flash[68] = tmp_buf[14];
			tmp_flash[69] = tmp_buf[15];
			
			tmp_flash[72] = tmp_buf[16];
			tmp_flash[73] = tmp_buf[17];
			tmp_flash[74] = tmp_buf[18];
			tmp_flash[75] = tmp_buf[19];
			tmp_flash[76] = tmp_buf[20];
			tmp_flash[77] = tmp_buf[21];
			tmp_flash[78] = tmp_buf[22];
			tmp_flash[79] = tmp_buf[23];
			
			STMFLASH_Write(FLASH_PROCHEAD,(u32*)tmp_flash,FLASH_PROC_SIZE/4);
			
			for (i = 10; i<10+host_len; i++) {
				tmp_buf[i] += 1;
				tmp_crc += tmp_buf[i];
			}
			tmp_buf[10+host_len] = tmp_crc;
			memcpy(p->payload, tmp_buf, 27+host_len);
		break;
		
		default:
			break;
		}
	}	
	  //add our data deal proc end.    
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (recv_err == ERR_OK) {
		memcpy(p->payload, tmp_buf,p->len);
		udp_sendto(upcb, p,addr,port);  //????
	}
  /* Free the p buffer */
	pbuf_free(p);
}

static void udpecho_thread(void *arg)
{
  struct udp_pcb *upcb;
   err_t err;

   /* Create a new UDP control block  */
   upcb = udp_new();  //??????UDP???
   if (upcb)
   {
     /* Bind the upcb to the UDP_PORT port */
     /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
	   err = udp_bind(upcb, IP_ADDR_ANY, g_LocalUdpPort);
      if(err == ERR_OK)
      {
        /* Set a receive callback for the upcb */
        udp_recv(upcb, udp_echoserver_receive_callback, NULL);   //??????????
      }
      else
      {
        udp_remove(upcb);
      }
   }
 }

 void udpecho_init(void)
{  
	udpecho_thread(NULL);
}

#endif


























