FiuNv
新版KL FIU，程序基于阿波罗
compiler by keil ,
led : front error = 0, run = 1, mcu run led = 500ms flicker while normal run

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

20180629
mulcast send period = 1s , send to 225,226,227,228:54345
led flick period = 500ms

ip set etc... at lwip_comm.c Line92

20180702
IO init in led.c

20180723
复合故障
帧内容:
1.	Byte0-Byte8 :通道号 共72通道
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0故障类型:0x0:复位,0x1 :开路,0x2 :对UbattA+ 短路,0x3 :对UbattA- 短路,0x4 :对UbattB+ 短路,0x5 :对UbattB- 短路,

快速通道故障
帧内容:
1.	Byte0-Byte8 :通道号 共72通道
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0故障类型:0x0:复位,0x1 :开路,0x2 :对UbattA+ 短路,0x3 :对UbattA- 短路,0x4 :对UbattB+ 短路,0x5 :对UbattB- 短路,

快速切换漏电模拟
帧内容:
1.	Byte0-Byte8 :通道号 共72通道
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0故障类型:0x0:复位,0x1 :ECU-LOAD,0x2 : ECU-UbattA+,0x3 : ECU-UbattA-,0x4 : ECU-UbattB+,0x5 : ECU-UbattB-,0x6 : ECU-ECU,
3.	Byte10,Byte11:电阻值 : Byte10 高位,Byte11低位

大电流通道故障
帧内容:
1.	Byte0-Byte8 :通道号 共64通道
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0????:0x0:复位,0x1 :开路,0x2 :对UbattC+ 短路,0x3 :对 UbattC- 短路,

---------
#define COMM_LEN 128 //MAX LEN COMM WITH THE WIN HOST , when FIU the packet max 12*8 + 27 = 123

20180726
selftest,can res ... not surpport
PF6 可用于 频率 等控制

20180803
初步形成 v1.0.0 版本
FIU控制界面联调ok，待生产测试 -> v.1.0.x 







































































































































































