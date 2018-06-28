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

#if LWIP_NETCONN

u16 LOCAL_UDPPORT_m = 10000;
extern u8 g_IpAllModified;

static struct netconn *conn;
static struct netbuf  *buf;
static struct ip_addr *addr;
static unsigned short port;


/*-----------------------------------------------------------------------------------*/
static void udpecho_thread(void *arg)
{
	
  err_t 	err = ERR_OK;
  err_t 	recv_err = ERR_OK;
  int 		i;
//  u8 		tmp_buf[COMM_LEN];
  u8 		tmp_crc = 0;
  int 		host_cmd = -1;
  u8 		IpPort[8] = {0};
//  u8 		tmp_flash[FLASH_PROC_SIZE];
  
  LWIP_UNUSED_ARG(arg);

  printf("udpecho_thread in .\n");
  conn = netconn_new(NETCONN_UDP);
  if (conn != NULL)
  {
		printf("conn new suc .\n");
	if (g_IpAllModified == 0)
		err = netconn_bind(conn, IP_ADDR_ANY, LOCAL_UDPPORT);
	else
		err = netconn_bind(conn, IP_ADDR_ANY, LOCAL_UDPPORT_m);
    if (err == ERR_OK)
    {
      while (1) 
      {
        recv_err = netconn_recv(conn, &buf);
        if (recv_err == ERR_OK) 
        {
          addr = netbuf_fromaddr(buf);
          port = netbuf_fromport(buf);
          netconn_connect(conn, addr, port);
          buf->addr.addr = 0;
				
						#if 0
				//add our data deal proc start.
	//		  if (buf->p->len == COMM_LEN) {
	//			memset(tmp_flash,0xff,128);	//		  
	//			  
	//			memcpy(tmp_buf, buf->p->payload, buf->p->len);
	//			for (i = 0; i < 4; i++) {
	//		      if (tmp_buf[i] != 0xbe) {
	//				recv_err = ERR_KLDATA;
	//			  }
	//			}
	//			
	//			if (recv_err == ERR_OK) 
	//			{
	//				host_cmd = tmp_buf[4];
	//				
	//				for (i = 5; i < 13; i++) {
	//				  tmp_crc += tmp_buf[i];
	//				}
	//				if (tmp_crc != tmp_buf[13]) {
	//				  recv_err = ERR_KLDATA; 
	//				}
	//				recv_err = ERR_OK;
	//			}
	//			
	//			if (recv_err == ERR_OK) 
	//			{
	//				for (i = 14; i < 18; i++) {
	//				  if (tmp_buf[i] != 0xff) {
	//					recv_err = ERR_KLDATA;
	//				  }
	//				}
	//			}
	//			if (recv_err == ERR_OK) 
	//			{
	//				for (i = 18; i < 22; i++) {
	//				  if (tmp_buf[i] != 0xed) {
	//					recv_err = ERR_KLDATA;
	//				  }
	//				}
	//			}	
	//				
	//			if (recv_err == ERR_OK) 
	//			{
	//				tmp_crc = 0;
	//				
	//				switch(host_cmd){
	//					case COMM_CMD_TEST:
	//						for (i = 5; i<13; i++) {
	//							tmp_buf[i] += 1;
	//							tmp_crc += tmp_buf[i];
	//						}
	//						tmp_buf[13] = tmp_crc;
	//						memcpy(buf->p->payload, tmp_buf, COMM_LEN);
	//					break;
	//						
	//					case COMM_CMD_RELAY:
	//						if (tmp_buf[5]) {//relay1
	//							RELAY1_ON;
	//						}
	//						else {
	//							RELAY1_OFF;
	//						}
	//						
	//						if (tmp_buf[6]) {//relay2
	//							RELAY2_ON;
	//						}
	//						else {
	//							RELAY2_OFF;
	//						}
	//						
	//						if (tmp_buf[7]) {//relay3
	//							RELAY3_ON;
	//						}
	//						else {
	//							RELAY3_OFF;
	//						}
	//						
	//						if (tmp_buf[8]) {//relay4
	//							RELAY4_ON;
	//						}
	//						else {
	//							RELAY4_OFF;
	//						}
	//					break;
	//						
	//					case COMM_CMD_IP:
	//						STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp_flash,FLASH_PROC_SIZE/4);
	//						for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
	//							tmp_flash[i] = 0xaa; 
	//							tmp_flash[i+1] = 0x55;
	//						}
	//						
	//						tmp_flash[64] = tmp_buf[5];
	//						tmp_flash[65] = tmp_buf[6];
	//						tmp_flash[66] = tmp_buf[7];
	//						tmp_flash[67] = tmp_buf[8];
	//						tmp_flash[68] = tmp_buf[9];
	//						tmp_flash[69] = tmp_buf[10];
	//						STMFLASH_Write(FLASH_PROCHEAD,(u32*)tmp_flash,FLASH_PROC_SIZE/4);
	//					break;
	//					
	//					case COMM_CMD_GW:
	//						STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp_flash,FLASH_PROC_SIZE/4);
	//						for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
	//							tmp_flash[i] = 0xaa; 
	//							tmp_flash[i+1] = 0x55;
	//						}
	//						
	//						tmp_flash[72] = tmp_buf[5];
	//						tmp_flash[73] = tmp_buf[6];
	//						tmp_flash[74] = tmp_buf[7];
	//						tmp_flash[75] = tmp_buf[8];
	//						tmp_flash[76] = tmp_buf[9];
	//						tmp_flash[77] = tmp_buf[10];
	//						tmp_flash[78] = tmp_buf[11];
	//						tmp_flash[79] = tmp_buf[12];
	//						STMFLASH_Write(FLASH_PROCHEAD,(u32*)tmp_flash,FLASH_PROC_SIZE/4);
	//					break;
	//					
	//					default:
	//						break;
	//				}
	//			}	
	//		  }
				//add our data deal proc end.    
	#endif
							
						netconn_send(conn, buf);
						netbuf_delete(buf);
        }
      }
    }
    else
    {
      netconn_delete(conn);
      printf("can not bind netconn");
    }
  }
  else
  {
    printf("can not create new UDP netconn");
  }
}

/*-----------------------------------------------------------------------------------*/
void udpecho_init(void)
{
  sys_thread_new("udpecho_thread", udpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, UDPECHO_THREAD_PRIO);
}

#endif /* LWIP_NETCONN */




























