/*
*********************************************************************************************************
*
*                                           wt's LWIP
*                                        Application Code
*
* Filename      : app_lwip.c
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  APP_LWIP_MODULE
#include <app_lwip.h>

#ifndef OS_ERR
typedef u8 OS_ERR;
#endif

//extern HAL_StatusTypeDef HAL_ETH_DeInit(ETH_HandleTypeDef *heth);

/*
*********************************************************************************************************
*                                                 ENABLE
*********************************************************************************************************
*/

#if (APP_CFG_LWIP_EN == DEF_ENABLED)


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

ETH_HandleTypeDef 	ETH_HandleStructure;
ETH_DMADescTypeDef 	Eth_RxDesc[ETH_RXBUFNB];		//��̫��DMA��������������ռ�
ETH_DMADescTypeDef 	Eth_TxDesc[ETH_RXBUFNB];				//��̫��DMA��������������ռ�
uint8_t  	Eth_Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];		//��̫���ײ���������buffers
uint8_t  	Eth_Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 		 //��̫���ײ���������buffers

#if ( LWIP_DHCP == 1)
//������ƿ�
OS_TCB *LWIP_DHCPTaskTCB;
//�����ջ�������ڴ����ķ�ʽ��������	
CPU_STK * LWIP_DHCP_TASK_STK;	
#endif

__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�


/*
*********************************************************************************************************
*                                         LOCAL  FUNCTION  DECLARE
*********************************************************************************************************
*/

#if ( LWIP_DHCP == 1)
void lwip_dhcp_task(void *pdata);
#endif
void LwipBSP_STM32_ETH_IntHandler(void);
static void lwip_comm_default_ip_set(__lwip_dev *lwipx);
static void  tcpip_recv_thread (void *p_arg);

//static HAL_StatusTypeDef  ETH_PHY_LAN8720_Init (void);
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/**
  * @brief  Initializes the ETH MSP.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
//	u8_t rval=0;
//  uint32_t i,j = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	//Enable the Ethernet interface clock using 
	__HAL_RCC_ETH_CLK_ENABLE();	  
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	/*������������ RMII�ӿ�
	  ETH_MDIO -------------------------> PA2
	  ETH_MDC --------------------------> PC1
	  ETH_RMII_REF_CLK------------------> PA1
	  ETH_RMII_CRS_DV ------------------> PA7
	  ETH_RMII_RXD0 --------------------> PC4
	  ETH_RMII_RXD1 --------------------> PC5
	  ETH_RMII_TX_EN -------------------> PG11
	  ETH_RMII_TXD0 --------------------> PG13
	  ETH_RMII_TXD1 --------------------> PG14
	  ETH_RESET-------------------------> PD3*/

	  //����PA1 PA2 PA7
	GPIO_InitStructure.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//���Ÿ��õ�����ӿ���
	GPIO_InitStructure.Pull = GPIO_NOPULL ; 
	GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PB11
    GPIO_InitStructure.Pin=GPIO_PIN_11;               //PB11
    HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);         //ʼ��

	//����PC1,PC4 and PC5
	GPIO_InitStructure.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

                                
	//����PG14 and PG13 
	GPIO_InitStructure.Pin =  GPIO_PIN_14|GPIO_PIN_13;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	LAN8720_RST=0;					//Ӳ����λLAN8720
//	delay_ms(50);
	
	HAL_Delay(50);

	LAN8720_RST=1;				 	//��λ���� 
	
	/* set priority of ETH*/  
	//void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
	BSP_IntPrioSet (BSP_INT_ID_ETH, APP_CFG_INTERRUPT_ETH_PRIO);

	/* configurate ISR of ETH*/
	BSP_IntVectSet(BSP_INT_ID_ETH, LwipBSP_STM32_ETH_IntHandler);

	/* enable NVIC interupt  of ETH*/
	//void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
	BSP_IntEn(BSP_INT_ID_ETH);

	return;
	
}


//�õ�8720���ٶ�ģʽ
//����ֵ:
//001:10M��˫��
//101:10Mȫ˫��
//010:100M��˫��
//110:100Mȫ˫��
//����:����.
uint32_t LAN8720_Get_Speed(ETH_HandleTypeDef *heth)
{
	uint32_t speed;
	HAL_ETH_ReadPHYRegister( heth, PHY_SR, &speed);
	
	speed=((speed&0x1C)>>2); //��LAN8720��31�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ
	return speed;
}


void LwipBSP_STM32_ETH_IntHandler(void)
{

	HAL_ETH_IRQHandler(&ETH_HandleStructure);

}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
APP_TRACE_DBG(("���緢�����\r\n"));	 
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	OS_ERR	os_err;

	OSTaskSemPost (&TcpipRecvThreadTCB,		//����֡��ɺ���ں˽���������Ϣ��֪
                           OS_OPT_POST_NONE,
                           &os_err);
                           
	LWIP_ASSERT("os_err == OS_ERR_NONE", os_err == OS_ERR_NONE);
}

/**
  * @brief  Ethernet transfer error callbacks
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
//	HAL_StatusTypeDef eth_err;
	
	APP_TRACE_DBG(("******************������̫����******************\r\n"));	  	

  if(__HAL_ETH_DMA_GET_FLAG(heth, ETH_DMA_FLAG_FBE))
  	{
		APP_TRACE_DBG(("******************������̫�������ߴ���******************\r\n"));	  	
 	   /* Clear the Eth DMA Tx IT pending bits */
 	   __HAL_ETH_DMA_CLEAR_IT(heth, ETH_DMA_IT_FBE);
		 
		 //���������������������̫����
		 //�˴�����ֱ�ӵ���ETH_PHY_LAN8720_Init()�����������³�ʼ������Ϊ
		 //�˺����е�����HAL_Delay()����������ʱ�����˺�����ʱ�����õ�
		 //systick�жϼ�����ɵģ����Բ������ж��е��ã��˴�����һ��
		 //��ʼ��ʧ�ܵı�־����AppTaskStart��������ѵ�˱�־���г�ʼ��
		lwipdev.init_ok = 0;
  	}
}

/*
//����USMART����
uint16_t USMART_ETH_ReadPHYRegister(uint16_t PHYReg)
{
	uint32_t RegValue = 0;
	HAL_ETH_ReadPHYRegister(&ETH_HandleStructure,PHYReg, &RegValue);
	return RegValue;
}

//����USMART����
uint16_t USMART_ETH_WritePHYRegister(uint16_t PHYReg, uint32_t RegValue)
{
	return  HAL_ETH_WritePHYRegister(&ETH_HandleStructure, PHYReg, RegValue);
}
*/

//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
static void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//Ĭ��Զ��IPΪ:192.168.0.105
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=0;
	lwipx->remoteip[3]=114;
	
	//Ĭ�ϱ���IPΪ:192.168.0.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=0;
	lwipx->ip[3]=24;
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//Ĭ������:192.168.0.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=0;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 

//��������ip��ַ
void lwip_reset_netif_ipaddr(__lwip_dev *lwipdev)
{
	struct ip_addr ipaddr, netmask, gw;

	IP4_ADDR(&ipaddr,lwipdev->ip[0],lwipdev->ip[1],lwipdev->ip[2],lwipdev->ip[3]);                       //��IP
	IP4_ADDR(&netmask,lwipdev->netmask[0],lwipdev->netmask[1] ,lwipdev->netmask[2],lwipdev->netmask[3]); //����������
	IP4_ADDR(&gw,lwipdev->gateway[0],lwipdev->gateway[1],lwipdev->gateway[2],lwipdev->gateway[3]);       //������
	
	netif_set_down(&lwip_netif);

	netif_set_gw(&lwip_netif, &gw);
	netif_set_netmask(&lwip_netif, &netmask);
	netif_set_ipaddr(&lwip_netif, &ipaddr);
	
//	netif_set_addr(&lwip_netif, &ipaddr, &netmask, &gw);

  netif_set_up(&lwip_netif);

}


/* PHY Lan8720 init
*
*/
HAL_StatusTypeDef  ETH_PHY_LAN8720_Init (void)
{
	HAL_StatusTypeDef eth_err;
	u32_t sn0;
	sn0=*(__IO u32_t*)(0x1FFF7A10);               //��ȡSTM32��ΨһID��ǰ24λ��ΪMAC��ַ�����ֽ�
	
	ETH_HandleStructure.Instance = ETH;

	ETH_HandleStructure.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
	ETH_HandleStructure.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
	ETH_HandleStructure.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
	ETH_HandleStructure.Init.RxMode = ETH_RXINTERRUPT_MODE;
	ETH_HandleStructure.Init.PhyAddress = LAN8720_PHY_ADDRESS;

	
	//NETIF_MAX_HWADDR_LEN
	lwip_netif.hwaddr_len = ETHARP_HWADDR_LEN; //����MAC��ַ����,Ϊ6���ֽ�

	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	//MAC��ַ������ֽڵ�LSBλΪ0�����ǵ�����ַ��Ϊ1���鲥��ַ
	lwip_netif.hwaddr[0]= 2;//�����ֽ�(IEEE��֮Ϊ��֯ΨһID,OUI)��ַ�̶�Ϊ:2.0.0
	lwip_netif.hwaddr[1]= 0;
	lwip_netif.hwaddr[2]= 0;
	lwip_netif.hwaddr[3]= (sn0>>16)&0XFF;//�����ֽ���STM32��ΨһID
	lwip_netif.hwaddr[4]= (sn0>>8)&0XFFF;;
	lwip_netif.hwaddr[5]= sn0&0XFF; 
	
//MAC��ַ����
//STM32�̼�������HAL_ETH_Init()  ��ETH_MACDMAConfig() ��ETH_MACAddressConfig()д��MAC��ַ�Ĵ�����
	ETH_HandleStructure.Init.MACAddr = lwip_netif.hwaddr;

	HAL_ETH_DeInit(&ETH_HandleStructure);
	
//�����ȡPHY������״̬��ʱ���߻�ȡ��Э�̳�ʱ�򷵻�HAL_TIMEOUT���������󷵻�HAL_ERROR
	eth_err = HAL_ETH_Init(&ETH_HandleStructure);	

//STM32�̼�������HAL_ETH_Init()  ��ETH_MACDMAConfig() ��ֻʹ���˽�������ж�
//�˴�ʹ�ܷ�������жϺ��쳣�������ж�
__HAL_ETH_DMA_ENABLE_IT((&ETH_HandleStructure), ETH_DMA_IT_NIS | ETH_DMA_IT_T | ETH_DMA_IT_AIS | ETH_DMA_IT_FBE);	

//����DMA���ͺͽ�����������Ϊ����ʽ
	HAL_ETH_DMATxDescListInit(&ETH_HandleStructure, Eth_TxDesc, (uint8_t *)Eth_Tx_Buff, ETH_TXBUFNB);
	HAL_ETH_DMARxDescListInit(&ETH_HandleStructure,  Eth_RxDesc, (uint8_t *)Eth_Rx_Buff, ETH_RXBUFNB);	

	HAL_ETH_Start(&ETH_HandleStructure); //����MAC��DMA	��low_level_init()�е���	
	return eth_err;
}


//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,LAN8720��ʼ��ʧ��
//      3,�������ʧ��.  
HAL_StatusTypeDef App_Lwip_Init(void)
{
	HAL_StatusTypeDef eth_err;	
	OS_ERR  os_err;
	struct netif *Netif_Init_Flag;		  //����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip_addr ipaddr;  			  //ip��ַ
	struct ip_addr netmask; 			  //��������
	struct ip_addr gw;      			  //Ĭ������ 

	CPU_SR_ALLOC();

	lwip_comm_default_ip_set(&lwipdev);	  //����Ĭ��IP����Ϣ

//�����ȡPHY������״̬��ʱ���߻�ȡ��Э�̳�ʱ�򷵻�HAL_TIMEOUT���������󷵻�HAL_ERROR
	eth_err = ETH_PHY_LAN8720_Init();
	if(eth_err != HAL_OK )	return eth_err;		//��ʼ��LAN8720ʧ�� 	
		
	tcpip_init(NULL,NULL);				  //��ʼ��tcp ip�ں�,�ú�������ᴴ��tcpip_thread�ں�����

	CPU_CRITICAL_ENTER();	
  //�������񴴽�
  OSTaskCreate((OS_TCB     *)&TcpipRecvThreadTCB,                                 //������ƿ�
              "tcpip recv_thread"	,                                   //������
              (OS_TASK_PTR  )tcpip_recv_thread,                               //����ִ�к�������
              NULL,                                    //���ݸ�����Ĳ���
              (OS_PRIO      )TCPIP_RECV_THREAD_PRIO,                                   //���ȼ�
              (CPU_STK     *)&TcpipRecvThreadStk[0],                                 //�����ջ��ַ
              (CPU_STK_SIZE )TCPIP_RECV_THREAD_STACKSIZE/10,                           //�����ջʹ��ֵ
              (CPU_STK_SIZE )TCPIP_RECV_THREAD_STACKSIZE,                              //�����ջ��С
              (OS_MSG_QTY   )0,
              (OS_TICK      )0,
              (void        *)0,
              (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
              (OS_ERR      *)&os_err); 	
	
	if(OS_ERR_NONE != os_err)	
		return HAL_ERROR;	
	
	CPU_CRITICAL_EXIT() ;	
	
#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//ʹ�þ�̬IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                       //��IP
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]); //����������
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);       //������
	
#endif

	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//�������б������һ������
	
	if(Netif_Init_Flag==NULL)
	{
		return HAL_ERROR;//�������ʧ�� 
	}
	else                                //������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);		//��netif����
	}

/*		
#if  ( LWIP_DHCP == 1)			        //���ʹ��DHCP�Ļ�
	lwip_comm_dhcp_creat(); //���� DHCP ����
#endif
*/
	return HAL_OK;//����OK.
}   

//�ں˽�������
static void  tcpip_recv_thread (void *p_arg)
{
    OS_ERR  os_err;
	OS_SEM_CTR  n;
    (void)p_arg;

    while (DEF_TRUE) {
		n = OSTaskSemPend (0,
                           OS_OPT_PEND_BLOCKING,		//�ȴ�����֡�ź����Ľ���
                           0,
                           &os_err)	;
                           
	   LWIP_ASSERT("n == 0", n == 0);	//���n !=0,˵�������ٶ�̫�죬�����̴߳�������
	   LWIP_ASSERT("os_err == OS_ERR_NONE", os_err == OS_ERR_NONE);
		 
		ethernetif_input(&lwip_netif);      //�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 

        //APP_TRACE_INFO(("Object test task 0 running ....\n"));
    }
}


//��̬������ߵ��Ȳ�Ρ�(lwipdev.init_ok == 1)�����⵽���߲��벢��lwipЭ��ջ��ʼ���ɹ�
//�ڵ��ô˺���֮ǰ�������ȵ���App_Lwip_Init()
//�˺�����Ҫ��ѵ���ã�Ĭ�Ϸŵ�AppTaskStart���������ѭ������
void Lwip_netlink_status_check(void)
{
	HAL_StatusTypeDef lwip_err;
	uint32_t	phyreg; 
	static uint8_t m = 0;//���ڱ�ʶ�Ƿ��Ѿ��ɹ����ù�һ��App_Lwip_Init()����������1
	
	lwip_err = HAL_ETH_ReadPHYRegister(&ETH_HandleStructure, PHY_BSR, &phyreg);//��ȡPHY�Ĵ����ж������Ƿ�����
	if((lwip_err == HAL_OK)&&((phyreg & PHY_LINKED_STATUS) == PHY_LINKED_STATUS))	//����������Ӻ�
	{
		if(lwipdev.init_ok == 0)	//���֮ǰ����û�����Ӻ�
		{
//			APP_TRACE_DBG(("�������Ӻ�\r\n"));	
			if(m == 1)	//���֮ǰ�Ѿ��ɹ����ù�һ��App_Lwip_Init()��˵��lwip�Ѿ���ʼ��������ʱֻ��Ҫ��ʼ��ETH��MAC��DMA
			{
				if(ETH_PHY_LAN8720_Init() == HAL_OK)//���³�ʼ��ETH
				{
					lwipdev.init_ok = 1;
//					APP_TRACE_DBG(("���³�ʼ��LWIP�ɹ�   ��ʼ��ETH��MAC��DMA\r\n"));						
				}
			}
			else			//���֮ǰû�гɹ���ʼ����lwip�ں�ջ����Ҫ��ʼ��ETH��MAC��DMA���Լ�lwip�ں�ջ
			{
				if(App_Lwip_Init() == HAL_OK)//���³�ʼ��ETH
				{
					lwipdev.init_ok = 1;
//					APP_TRACE_DBG(("���³�ʼ��LWIP�ɹ�   ��ʼ��ETH��MAC��DMA���Լ�lwip�ں�ջ\r\n"));						
				}
			}
					
		}
		else 
		{
			m = 1;
//			APP_TRACE_DBG(("��ʼ��ETH�ɹ�\r\n"));	
		}
			
	}
	else
	{
//		APP_TRACE_DBG(("����û�����Ӻ�\r\n"));				
		lwipdev.init_ok = 0;
	}
}


//���ʹ����DHCP
#if ( LWIP_DHCP == 1)
//����DHCP����
void lwip_comm_dhcp_creat(void)
{
//	CPU_SR_ALLOC();
  OS_ERR  err;
//  CPU_STK *StkPtr;   //���������ջָ��
  
	LWIP_DHCPTaskTCB   = (OS_TCB *)mymalloc(SRAMCCM,sizeof(OS_TCB));               //��������ƿ����ռ�
  LWIP_DHCP_TASK_STK = (CPU_STK *)mymalloc(SRAMCCM,LWIP_DHCP_STK_SIZE * sizeof(CPU_STK)); //�������ջ����ռ�

  LWIP_ASSERT("LWIP_DHCPTaskTCB != NULL", LWIP_DHCPTaskTCB != NULL);        //�������
  LWIP_ASSERT("LWIP_DHCP_TASK_STK != NULL", LWIP_DHCP_TASK_STK != NULL);

//	CPU_CRITICAL_ENTER();	//�����ٽ���
	//����DHCP����
	OSTaskCreate((OS_TCB 	* )LWIP_DHCPTaskTCB,		//������ƿ�
				 					(CPU_CHAR	* )"dhcp task", 			//��������
                 (OS_TASK_PTR )lwip_dhcp_task, 			//������
                 (void		* )0,						//���ݸ��������Ĳ���
                 (OS_PRIO	  )DHCP_THREAD_PRIO, 	//�������ȼ�
                 (CPU_STK   * )LWIP_DHCP_TASK_STK,					//�����ջ����ַ
                 (CPU_STK_SIZE)LWIP_DHCP_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)LWIP_DHCP_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,						//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,						//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,						//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);					//��Ÿú�������ʱ�ķ���ֵ
//	CPU_CRITICAL_EXIT();	//�˳��ٽ���	
	
}
//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	OS_ERR err;
	
	dhcp_stop(&lwip_netif); 		//�ر�DHCP
	OSTaskDel(LWIP_DHCPTaskTCB,&err);//ɾ��DHCP����
	myfree(SRAMEX, LWIP_DHCPTaskTCB);  
	myfree(SRAMEX, LWIP_DHCP_TASK_STK); 

}
//DHCP��������
void lwip_dhcp_task(void *pdata)
{
	u32_t ip=0,netmask=0,gw=0;
	OS_ERR err;
	
	dhcp_start(&lwip_netif);//����DHCP 
	lwipdev.dhcpstatus=0;	//����DHCP
	printf("���ڲ���DHCP������,���Ե�...........\r\n");   
	while(1)
	{ 
		printf("���ڻ�ȡ��ַ...\r\n");
		ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
		netmask=lwip_netif.netmask.addr;//��ȡ��������
		gw=lwip_netif.gw.addr;			//��ȡĬ������ 
		if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
		{
			lwipdev.dhcpstatus=2;	//DHCP�ɹ�
 			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//������ͨ��DHCP��ȡ����IP��ַ
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//����ͨ��DHCP��ȡ�������������ַ
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//������ͨ��DHCP��ȡ����Ĭ������
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
		{  
			lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
			//ʹ�þ�̬IP��ַ
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
//		delay_ms(250); //��ʱ250ms
		OSTimeDly(250,OS_OPT_TIME_DLY,&err); //��ʱ250ms
		
	}
	lwip_comm_dhcp_delete();//ɾ��DHCP���� 
}

#endif

#endif











