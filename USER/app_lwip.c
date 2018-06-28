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
ETH_DMADescTypeDef 	Eth_RxDesc[ETH_RXBUFNB];		//以太网DMA接收描述符数组空间
ETH_DMADescTypeDef 	Eth_TxDesc[ETH_RXBUFNB];				//以太网DMA发送描述符数组空间
uint8_t  	Eth_Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];		//以太网底层驱动接收buffers
uint8_t  	Eth_Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 		 //以太网底层驱动发送buffers

#if ( LWIP_DHCP == 1)
//任务控制块
OS_TCB *LWIP_DHCPTaskTCB;
//任务堆栈，采用内存管理的方式控制申请	
CPU_STK * LWIP_DHCP_TASK_STK;	
#endif

__lwip_dev lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口


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
	/*网络引脚设置 RMII接口
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

	  //配置PA1 PA2 PA7
	GPIO_InitStructure.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//引脚复用到网络接口上
	GPIO_InitStructure.Pull = GPIO_NOPULL ; 
	GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PB11
    GPIO_InitStructure.Pin=GPIO_PIN_11;               //PB11
    HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);         //始化

	//配置PC1,PC4 and PC5
	GPIO_InitStructure.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

                                
	//配置PG14 and PG13 
	GPIO_InitStructure.Pin =  GPIO_PIN_14|GPIO_PIN_13;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	LAN8720_RST=0;					//硬件复位LAN8720
//	delay_ms(50);
	
	HAL_Delay(50);

	LAN8720_RST=1;				 	//复位结束 
	
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


//得到8720的速度模式
//返回值:
//001:10M半双工
//101:10M全双工
//010:100M半双工
//110:100M全双工
//其他:错误.
uint32_t LAN8720_Get_Speed(ETH_HandleTypeDef *heth)
{
	uint32_t speed;
	HAL_ETH_ReadPHYRegister( heth, PHY_SR, &speed);
	
	speed=((speed&0x1C)>>2); //从LAN8720的31号寄存器中读取网络速度和双工模式
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
APP_TRACE_DBG(("网络发送完成\r\n"));	 
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

	OSTaskSemPost (&TcpipRecvThreadTCB,		//接收帧完成后给内核接收任务发信息告知
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
	
	APP_TRACE_DBG(("******************发生以太错误******************\r\n"));	  	

  if(__HAL_ETH_DMA_GET_FLAG(heth, ETH_DMA_FLAG_FBE))
  	{
		APP_TRACE_DBG(("******************发生以太致命总线错误******************\r\n"));	  	
 	   /* Clear the Eth DMA Tx IT pending bits */
 	   __HAL_ETH_DMA_CLEAR_IT(heth, ETH_DMA_IT_FBE);
		 
		 //发生致命错误必须重启以太外设
		 //此处不能直接调用ETH_PHY_LAN8720_Init()函数进行重新初始化，因为
		 //此函数中调用了HAL_Delay()函数进行延时，而此函数延时是利用的
		 //systick中断计数完成的，所以不能在中断中调用，此处设置一个
		 //初始化失败的标志，在AppTaskStart任务中轮训此标志进行初始化
		lwipdev.init_ok = 0;
  	}
}

/*
//用于USMART测试
uint16_t USMART_ETH_ReadPHYRegister(uint16_t PHYReg)
{
	uint32_t RegValue = 0;
	HAL_ETH_ReadPHYRegister(&ETH_HandleStructure,PHYReg, &RegValue);
	return RegValue;
}

//用于USMART测试
uint16_t USMART_ETH_WritePHYRegister(uint16_t PHYReg, uint32_t RegValue)
{
	return  HAL_ETH_WritePHYRegister(&ETH_HandleStructure, PHYReg, RegValue);
}
*/

//lwip 默认IP设置
//lwipx:lwip控制结构体指针
static void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//默认远端IP为:192.168.0.105
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=0;
	lwipx->remoteip[3]=114;
	
	//默认本地IP为:192.168.0.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=0;
	lwipx->ip[3]=24;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.0.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=0;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
} 

//重新设置ip地址
void lwip_reset_netif_ipaddr(__lwip_dev *lwipdev)
{
	struct ip_addr ipaddr, netmask, gw;

	IP4_ADDR(&ipaddr,lwipdev->ip[0],lwipdev->ip[1],lwipdev->ip[2],lwipdev->ip[3]);                       //绑定IP
	IP4_ADDR(&netmask,lwipdev->netmask[0],lwipdev->netmask[1] ,lwipdev->netmask[2],lwipdev->netmask[3]); //绑定子网掩码
	IP4_ADDR(&gw,lwipdev->gateway[0],lwipdev->gateway[1],lwipdev->gateway[2],lwipdev->gateway[3]);       //绑定网关
	
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
	sn0=*(__IO u32_t*)(0x1FFF7A10);               //获取STM32的唯一ID的前24位作为MAC地址后三字节
	
	ETH_HandleStructure.Instance = ETH;

	ETH_HandleStructure.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
	ETH_HandleStructure.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
	ETH_HandleStructure.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
	ETH_HandleStructure.Init.RxMode = ETH_RXINTERRUPT_MODE;
	ETH_HandleStructure.Init.PhyAddress = LAN8720_PHY_ADDRESS;

	
	//NETIF_MAX_HWADDR_LEN
	lwip_netif.hwaddr_len = ETHARP_HWADDR_LEN; //设置MAC地址长度,为6个字节

	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	//MAC地址的最高字节的LSB位为0代表是单播地址，为1是组播地址
	lwip_netif.hwaddr[0]= 2;//高三字节(IEEE称之为组织唯一ID,OUI)地址固定为:2.0.0
	lwip_netif.hwaddr[1]= 0;
	lwip_netif.hwaddr[2]= 0;
	lwip_netif.hwaddr[3]= (sn0>>16)&0XFF;//低三字节用STM32的唯一ID
	lwip_netif.hwaddr[4]= (sn0>>8)&0XFFF;;
	lwip_netif.hwaddr[5]= sn0&0XFF; 
	
//MAC地址设置
//STM32固件库是在HAL_ETH_Init()  →ETH_MACDMAConfig() →ETH_MACAddressConfig()写入MAC地址寄存器的
	ETH_HandleStructure.Init.MACAddr = lwip_netif.hwaddr;

	HAL_ETH_DeInit(&ETH_HandleStructure);
	
//如果读取PHY的连接状态超时或者获取自协商超时则返回HAL_TIMEOUT；其他错误返回HAL_ERROR
	eth_err = HAL_ETH_Init(&ETH_HandleStructure);	

//STM32固件库是在HAL_ETH_Init()  →ETH_MACDMAConfig() 中只使能了接收完成中断
//此处使能发送完成中断和异常错误总中断
__HAL_ETH_DMA_ENABLE_IT((&ETH_HandleStructure), ETH_DMA_IT_NIS | ETH_DMA_IT_T | ETH_DMA_IT_AIS | ETH_DMA_IT_FBE);	

//构造DMA发送和接收描述符成为链表方式
	HAL_ETH_DMATxDescListInit(&ETH_HandleStructure, Eth_TxDesc, (uint8_t *)Eth_Tx_Buff, ETH_TXBUFNB);
	HAL_ETH_DMARxDescListInit(&ETH_HandleStructure,  Eth_RxDesc, (uint8_t *)Eth_Rx_Buff, ETH_RXBUFNB);	

	HAL_ETH_Start(&ETH_HandleStructure); //开启MAC和DMA	在low_level_init()中调用	
	return eth_err;
}


//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.  
HAL_StatusTypeDef App_Lwip_Init(void)
{
	HAL_StatusTypeDef eth_err;	
	OS_ERR  os_err;
	struct netif *Netif_Init_Flag;		  //调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			  //ip地址
	struct ip_addr netmask; 			  //子网掩码
	struct ip_addr gw;      			  //默认网关 

	CPU_SR_ALLOC();

	lwip_comm_default_ip_set(&lwipdev);	  //设置默认IP等信息

//如果读取PHY的连接状态超时或者获取自协商超时则返回HAL_TIMEOUT；其他错误返回HAL_ERROR
	eth_err = ETH_PHY_LAN8720_Init();
	if(eth_err != HAL_OK )	return eth_err;		//初始化LAN8720失败 	
		
	tcpip_init(NULL,NULL);				  //初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务

	CPU_CRITICAL_ENTER();	
  //接收任务创建
  OSTaskCreate((OS_TCB     *)&TcpipRecvThreadTCB,                                 //任务控制块
              "tcpip recv_thread"	,                                   //任务名
              (OS_TASK_PTR  )tcpip_recv_thread,                               //任务执行函数名称
              NULL,                                    //传递给任务的参数
              (OS_PRIO      )TCPIP_RECV_THREAD_PRIO,                                   //优先级
              (CPU_STK     *)&TcpipRecvThreadStk[0],                                 //任务堆栈地址
              (CPU_STK_SIZE )TCPIP_RECV_THREAD_STACKSIZE/10,                           //任务堆栈使用值
              (CPU_STK_SIZE )TCPIP_RECV_THREAD_STACKSIZE,                              //任务堆栈大小
              (OS_MSG_QTY   )0,
              (OS_TICK      )0,
              (void        *)0,
              (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
              (OS_ERR      *)&os_err); 	
	
	if(OS_ERR_NONE != os_err)	
		return HAL_ERROR;	
	
	CPU_CRITICAL_EXIT() ;	
	
#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//使用静态IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                       //绑定IP
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]); //绑定子网掩码
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);       //绑定网关
	
#endif

	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	
	if(Netif_Init_Flag==NULL)
	{
		return HAL_ERROR;//网卡添加失败 
	}
	else                                //网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}

/*		
#if  ( LWIP_DHCP == 1)			        //如果使用DHCP的话
	lwip_comm_dhcp_creat(); //创建 DHCP 任务
#endif
*/
	return HAL_OK;//操作OK.
}   

//内核接收任务
static void  tcpip_recv_thread (void *p_arg)
{
    OS_ERR  os_err;
	OS_SEM_CTR  n;
    (void)p_arg;

    while (DEF_TRUE) {
		n = OSTaskSemPend (0,
                           OS_OPT_PEND_BLOCKING,		//等待接收帧信号量的接收
                           0,
                           &os_err)	;
                           
	   LWIP_ASSERT("n == 0", n == 0);	//如果n !=0,说明接收速度太快，接收线程处理不过来
	   LWIP_ASSERT("os_err == OS_ERR_NONE", os_err == OS_ERR_NONE);
		 
		ethernetif_input(&lwip_netif);      //从网络缓冲区中读取接收到的数据包并将其发送给LWIP处理 

        //APP_TRACE_INFO(("Object test task 0 running ....\n"));
    }
}


//动态监测网线的热插拔。(lwipdev.init_ok == 1)代表检测到网线插入并且lwip协议栈初始化成功
//在调用此函数之前，必须先调用App_Lwip_Init()
//此函数需要轮训调用，默认放到AppTaskStart这个任务中循环调用
void Lwip_netlink_status_check(void)
{
	HAL_StatusTypeDef lwip_err;
	uint32_t	phyreg; 
	static uint8_t m = 0;//用于标识是否已经成功调用过一次App_Lwip_Init()，若是则置1
	
	lwip_err = HAL_ETH_ReadPHYRegister(&ETH_HandleStructure, PHY_BSR, &phyreg);//读取PHY寄存器判断网线是否连接
	if((lwip_err == HAL_OK)&&((phyreg & PHY_LINKED_STATUS) == PHY_LINKED_STATUS))	//如果网线连接好
	{
		if(lwipdev.init_ok == 0)	//如果之前网线没有连接好
		{
//			APP_TRACE_DBG(("网线连接好\r\n"));	
			if(m == 1)	//如果之前已经成功调用过一次App_Lwip_Init()，说明lwip已经初始化过，此时只需要初始化ETH的MAC和DMA
			{
				if(ETH_PHY_LAN8720_Init() == HAL_OK)//重新初始化ETH
				{
					lwipdev.init_ok = 1;
//					APP_TRACE_DBG(("重新初始化LWIP成功   初始化ETH的MAC和DMA\r\n"));						
				}
			}
			else			//如果之前没有成功初始化过lwip内核栈，需要初始化ETH的MAC和DMA，以及lwip内核栈
			{
				if(App_Lwip_Init() == HAL_OK)//重新初始化ETH
				{
					lwipdev.init_ok = 1;
//					APP_TRACE_DBG(("重新初始化LWIP成功   初始化ETH的MAC和DMA，以及lwip内核栈\r\n"));						
				}
			}
					
		}
		else 
		{
			m = 1;
//			APP_TRACE_DBG(("初始化ETH成功\r\n"));	
		}
			
	}
	else
	{
//		APP_TRACE_DBG(("网线没有连接好\r\n"));				
		lwipdev.init_ok = 0;
	}
}


//如果使能了DHCP
#if ( LWIP_DHCP == 1)
//创建DHCP任务
void lwip_comm_dhcp_creat(void)
{
//	CPU_SR_ALLOC();
  OS_ERR  err;
//  CPU_STK *StkPtr;   //定义任务堆栈指针
  
	LWIP_DHCPTaskTCB   = (OS_TCB *)mymalloc(SRAMCCM,sizeof(OS_TCB));               //给任务控制块分配空间
  LWIP_DHCP_TASK_STK = (CPU_STK *)mymalloc(SRAMCCM,LWIP_DHCP_STK_SIZE * sizeof(CPU_STK)); //给任务堆栈分配空间

  LWIP_ASSERT("LWIP_DHCPTaskTCB != NULL", LWIP_DHCPTaskTCB != NULL);        //参数检查
  LWIP_ASSERT("LWIP_DHCP_TASK_STK != NULL", LWIP_DHCP_TASK_STK != NULL);

//	CPU_CRITICAL_ENTER();	//进入临界区
	//创建DHCP任务
	OSTaskCreate((OS_TCB 	* )LWIP_DHCPTaskTCB,		//任务控制块
				 					(CPU_CHAR	* )"dhcp task", 			//任务名字
                 (OS_TASK_PTR )lwip_dhcp_task, 			//任务函数
                 (void		* )0,						//传递给任务函数的参数
                 (OS_PRIO	  )DHCP_THREAD_PRIO, 	//任务优先级
                 (CPU_STK   * )LWIP_DHCP_TASK_STK,					//任务堆栈基地址
                 (CPU_STK_SIZE)LWIP_DHCP_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)LWIP_DHCP_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,						//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,						//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,						//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);					//存放该函数错误时的返回值
//	CPU_CRITICAL_EXIT();	//退出临界区	
	
}
//删除DHCP任务
void lwip_comm_dhcp_delete(void)
{
	OS_ERR err;
	
	dhcp_stop(&lwip_netif); 		//关闭DHCP
	OSTaskDel(LWIP_DHCPTaskTCB,&err);//删除DHCP任务
	myfree(SRAMEX, LWIP_DHCPTaskTCB);  
	myfree(SRAMEX, LWIP_DHCP_TASK_STK); 

}
//DHCP处理任务
void lwip_dhcp_task(void *pdata)
{
	u32_t ip=0,netmask=0,gw=0;
	OS_ERR err;
	
	dhcp_start(&lwip_netif);//开启DHCP 
	lwipdev.dhcpstatus=0;	//正在DHCP
	printf("正在查找DHCP服务器,请稍等...........\r\n");   
	while(1)
	{ 
		printf("正在获取地址...\r\n");
		ip=lwip_netif.ip_addr.addr;		//读取新IP地址
		netmask=lwip_netif.netmask.addr;//读取子网掩码
		gw=lwip_netif.gw.addr;			//读取默认网关 
		if(ip!=0)   					//当正确读取到IP地址的时候
		{
			lwipdev.dhcpstatus=2;	//DHCP成功
 			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//解析出通过DHCP获取到的IP地址
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//解析通过DHCP获取到的子网掩码地址
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//解析出通过DHCP获取到的默认网关
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //通过DHCP服务获取IP地址失败,且超过最大尝试次数
		{  
			lwipdev.dhcpstatus=0XFF;//DHCP失败.
			//使用静态IP地址
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP服务超时,使用静态IP地址!\r\n");
			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
//		delay_ms(250); //延时250ms
		OSTimeDly(250,OS_OPT_TIME_DLY,&err); //延时250ms
		
	}
	lwip_comm_dhcp_delete();//删除DHCP任务 
}

#endif

#endif











