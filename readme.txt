FiuNv
�°�KL FIU��������ڰ�����
compiler by keil ,
led : front error = 0, run = 1, mcu run led = 500ms flicker while normal run

FiuNv1.0.0
20180521
1����Ϊ��̬ip lwipopts.h L48
2������pingͨ
3����������·��Ĭ�Ϲر����鲥

20180524
modify 		stm32f4xx_hal_rth.c Line1885
turn on 	mulcast macro in opt.h line805
add 		|NETIF_FLAG_IGMP in ethernetif.c line23 to enable IGMP 
modify		igmp.c Line701 (func igmp_start_timer)
add			init.c Line61 
��ϵͳ��ʼ����led��������1��udp�����շ����鲥���ͣ�ֱ��1ms�������ֲ����� ��ok��

20180629
mulcast send period = 1s , send to 225,226,227,228:54345
led flick period = 500ms

ip set etc... at lwip_comm.c Line92

20180702
IO init in led.c

20180723
���Ϲ���
֡����:
1.	Byte0-Byte8 :ͨ���� ��72ͨ��
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0��������:0x0:��λ,0x1 :��·,0x2 :��UbattA+ ��·,0x3 :��UbattA- ��·,0x4 :��UbattB+ ��·,0x5 :��UbattB- ��·,

����ͨ������
֡����:
1.	Byte0-Byte8 :ͨ���� ��72ͨ��
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0��������:0x0:��λ,0x1 :��·,0x2 :��UbattA+ ��·,0x3 :��UbattA- ��·,0x4 :��UbattB+ ��·,0x5 :��UbattB- ��·,

�����л�©��ģ��
֡����:
1.	Byte0-Byte8 :ͨ���� ��72ͨ��
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0��������:0x0:��λ,0x1 :ECU-LOAD,0x2 : ECU-UbattA+,0x3 : ECU-UbattA-,0x4 : ECU-UbattB+,0x5 : ECU-UbattB-,0x6 : ECU-ECU,
3.	Byte10,Byte11:����ֵ : Byte10 ��λ,Byte11��λ

�����ͨ������
֡����:
1.	Byte0-Byte8 :ͨ���� ��64ͨ��
2.	Byte9: Bit7-Bit4  1: with load,0 with no load
Bit3-Bit0????:0x0:��λ,0x1 :��·,0x2 :��UbattC+ ��·,0x3 :�� UbattC- ��·,

---------
#define COMM_LEN 128 //MAX LEN COMM WITH THE WIN HOST , when FIU the packet max 12*8 + 27 = 123

20180726
selftest,can res ... not surpport
PF6 ������ Ƶ�� �ȿ���

20180803
�����γ� v1.0.0 �汾
FIU���ƽ�������ok������������ -> v.1.0.x 







































































































































































