FiuNv
新版KL FIU

FiuNv1.0.0
20180521
1、改为静态ip lwipopts.h L48
2、可以ping通
3、无线网络路由默认关闭了组播

20180524
modify 		stm32f4xx_hal_rth.c Line1885
turn on 	mulcast macro in opt.h line805
add 		|NETIF_FLAG_IGMP in ethernetif.c line23 to enable IGMP 
modify		igmp.c Line701 (func igmp_start_timer)
add			init.c Line61 
（系统初始化、led闪、串口1、udp单播收发、组播发送，直连1ms网络助手不丢包 已ok）








































































































































































实验器材:
	阿波罗STM32F429开发板
	
实验目的:
	学会LWIP在STM32上的移植，本实验使用了UCOSII操作系统。
	
硬件资源:
	1,DS0(连接在PB1) 
	2,串口1(波特率:115200,PA9/PA10连接在板载USB转串口芯片CH340上面) 
	3,ALIENTEK 2.8/3.5/4.3/7寸LCD模块(包括MCU屏和RGB屏,都支持) 
	4,STM32以太网MAC+外部PHY(LAN8720)
	5,网线一根
	
实验现象:
	本实验利用STM32的内部以太网MAC+外置PHY实现开发板的网络连接，本实验主要演示和讲解如何将LWIP和UCOSII
	操作系统结合起来实现开发板联网。本实验开启了LWIP的DHCP功能，开发板可以自动从路由器获取IP地址，也可
	以关闭DHCP，使用默认静态IP地址，默认静态IP地址为192.168.1.30，当开发板联网完成以后可以在电脑上向开
	发板发送ping包来测试网络是否通畅。
	
注意事项:
	1,4.3寸和7寸屏需要比较大电流,USB供电可能不足,请用外部电源适配器(推荐外接12V 1A电源).
	2,本例程在LCD_Init函数里面(在lcd.c),用到了printf,如果不初始化串口1,将导致液晶无法显示!! 
	3,本例程在初始化网络的时候需要连接网线，否则初始化不成功！！
	 

					正点原子@ALIENTEK
					2016-6-26
					广州市星翼电子科技有限公司
					电话：020-38271790
					传真：020-36773971
					购买：http://shop62103354.taobao.com
					http://shop62057469.taobao.com
					公司网站：www.alientek.com
					技术论坛：www.openedv.com
					
					
					
					
