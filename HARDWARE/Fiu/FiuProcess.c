#include "FiuProcess.h"
#include "usart.h"
#include "delay.h"
#include "math.h"


/******************************************************************
DEFINE CONTROL PINS OF ADUC841 FOR THE PURPOSE OF AD7982 CONTROL.
Customers should define the pins according to their design.
******************************************************************/
//sbit SCK=0x0A3;
//sbit CNV=0x0A0;
//sbit SDO=0x0A2;
//sbit SDI=0x0A1;


#define CHANNUMBER 72



extern uint8_t g_FiuByte17Data[17];




unsigned char DataRead[3];
unsigned char utemp1;
unsigned char uKasuStatus[CHANNUMBER];
unsigned char uKamnStatus[CHANNUMBER];

u16 uDqStatus[14];
u16 uResistance[15];

u8 uFiuCom_buf[17];

u8 uKstdSetChan,uKsmnSetChan;

u8 uFaultModePreStatus,uFaultModeNowStatus,uFaultModeStatus;  // 0x0:复合故障，1：快速通道 2：快速漏电 3：大电流
extern u8 udp_demo_recvbuf[2000];	//UDP接收数据缓冲区 
extern u8 cancommandbuf[100];
void AD7982_ReadData(unsigned char nByte);
void tdreset_process(void);

void HI_FunRelay_Set(u8 uSetValue);

void Delay(unsigned int Time)                 
{

	while(Time)
	{
		Time--;
	}
}


void Resistance_select(u8 uResvalueHigh,u8 uResvalueLow )
{
	

	
 if(((uResvalueHigh>>7)&0x1)==0)
 { 
	
	if(((uResvalueLow>>0)&0x1)==1)
	{
		RESCTRL1=1;
	}
	else
	{
		RESCTRL1=0;
	}
	if(((uResvalueLow>>1)&0x1)==1)
	{
		RESCTRL2=1;
	}
	else
	{
		RESCTRL2=0;
	}
	if(((uResvalueLow>>2)&0x1)==1)
	{
		RESCTRL3=1;
	}
	else
	{
		RESCTRL3=0;
	}
	if(((uResvalueLow>>3)&0x1)==1)
	{
		RESCTRL4=1;
	}
	else
	{
		RESCTRL4=0;
	}
	if(((uResvalueLow>>4)&0x1)==1)
	{
		RESCTRL5=1;
	}
	else
	{
		RESCTRL5=0;
	}
	if(((uResvalueLow>>5)&0x1)==1)
	{
		RESCTRL6=1;
	}
	else
	{
		RESCTRL6=0;
	}
	if(((uResvalueLow>>6)&0x1)==1)
	{
		RESCTRL7=1;
	}
	else
	{
		RESCTRL7=0;
	}
	if(((uResvalueLow>>7)&0x1)==1)
	{
		RESCTRL8=1;
	}
	else
	{
		RESCTRL8=0;
	}
	if(((uResvalueHigh>>0)&0x1)==1)
	{
		RESCTRL9=1;
	}
	else
	{
		RESCTRL9=0;
	}
	if(((uResvalueHigh>>1)&0x1)==1)
	{
		RESCTRL10=1;
	}
	else
	{
		RESCTRL10=0;
	}
	if(((uResvalueHigh>>2)&0x1)==1)
	{
		RESCTRL11=1;
	}
	else
	{
		RESCTRL11=0;
	}
	if(((uResvalueHigh>>3)&0x1)==1)
	{
		RESCTRL12=1;
	}
	else
	{
		RESCTRL12=0;
	}
	if(((uResvalueHigh>>4)&0x1)==1)
	{
		RESCTRL13=1;
	}
	else
	{
		RESCTRL13=0;
	}
	if(((uResvalueHigh>>5)&0x1)==1)
	{
		RESCTRL14=1;
	}
	else
	{
		RESCTRL14=0;
	}
	if(((uResvalueHigh>>6)&0x1)==1)
	{
		RESCTRL15=1;
	}
	else
	{
		RESCTRL15=0;
	}
	
}	
	
	
}	




u8 Fuse_test(void)
{
	u8 uTest_result;
	uTest_result=0;
	
	FUSETEST1=1;
	delay_ms(40);
  if(FUSEDATA1==0)
  uTest_result|=0x1;
	
	FUSETEST2=1;
	delay_ms(40);
  if(FUSEDATA2==0)
  uTest_result|=(0x1<<1);
	
	FUSETEST3=1;
	delay_ms(40);
  if(FUSEDATA3==0)
  uTest_result|=(0x1<<2);
	
	FUSETEST4=1;
	delay_ms(40);
  if(FUSEDATA4==0)
  uTest_result|=(0x1<<3);
	
	FUSETEST5=1;
	delay_ms(40);
  if(FUSEDATA5==0)
  uTest_result|=(0x1<<4);
	

	
	
	FUSETEST1=0;
	FUSETEST2=0;
	FUSETEST3=0;
	FUSETEST4=0;
	FUSETEST5=0;
	
	
	return uTest_result;
	
}	




void Sn74374_fun(u8 uNumber,u16 uSetValue)   //uNumber:0-13  
{ 
	u16 uSetValuetemp;
	
	uSetValuetemp=uDqStatus[uNumber]|uSetValue;
 
 
	if ((uSetValuetemp&0x01)>0) 
	 {RELAYEN1_SET;}	
  else
	 {RELAYEN1_RESET;}
	if (((uSetValuetemp>>1)&0x01)>0) 
	{RELAYEN2_SET;}	
	else 
	{RELAYEN2_RESET;}
	if (((uSetValuetemp>>2)&0x01)>0) 
	{RELAYEN3_SET;}	
	else 
	{RELAYEN3_RESET;}
	if (((uSetValuetemp>>3)&0x01)>0) 
	{RELAYEN4_SET;}	
	else 
	{RELAYEN4_RESET;}
	if (((uSetValuetemp>>4)&0x01)>0) 
	{RELAYEN5_SET;}	
	else 
	{RELAYEN5_RESET;}
	if (((uSetValuetemp>>5)&0x01)>0) 
	{RELAYEN6_SET;}	
	else 
	{RELAYEN6_RESET;}
		if (((uSetValuetemp>>6)&0x01)>0) 
	{RELAYEN7_SET;}	
	else 
	{RELAYEN7_RESET;}
	if (((uSetValuetemp>>7)&0x01)>0) 
	{RELAYEN8_SET;}	
	else 
	{RELAYEN8_RESET;}
	if (((uSetValuetemp>>8)&0x01)>0) 
	{RELAYEN9_SET;}	
	else 
	{RELAYEN9_RESET;}
	if (((uSetValuetemp>>9)&0x01)>0) 
	{RELAYEN10_SET;}	
	else 
	{RELAYEN10_RESET;}
	if (((uSetValuetemp>>10)&0x01)>0) 
	{RELAYEN11_SET;}	
	else 
	{RELAYEN11_RESET;}
	if (((uSetValuetemp>>11)&0x01)>0) 
	{RELAYEN12_SET;}	
	else 
	{RELAYEN12_RESET;}
	if (((uSetValuetemp>>12)&0x01)>0) 
	{RELAYEN13_SET;}	
	else 
	{RELAYEN13_RESET;}
	if (((uSetValuetemp>>13)&0x01)>0) 
	{RELAYEN14_SET;}	
	else 
	{RELAYEN14_RESET;}
	if (((uSetValuetemp>>14)&0x01)>0) 
	{RELAYEN15_SET;}	
	else 
	{RELAYEN15_RESET;}
	if (((uSetValuetemp>>15)&0x01)>0) 
	{RELAYEN16_SET;}	
	else 
	{RELAYEN16_RESET;}
	
	uDqStatus[uNumber]=uSetValuetemp;

  switch (uNumber)
			{
        case 0:CHCLK1_RESET;delay_ms(1);CHCLK1_SET;delay_ms(1);CHCLK1_RESET;break;  
        case 1:CHCLK2_RESET;delay_ms(1);CHCLK2_SET;delay_ms(1);CHCLK2_RESET;break;  
        case 2:CHCLK3_RESET;delay_ms(1);CHCLK3_SET;delay_ms(1);CHCLK3_RESET;break;  
        case 3:CHCLK4_RESET;delay_ms(1);CHCLK4_SET;delay_ms(1);CHCLK4_RESET;break;  
				case 4:CHCLK5_RESET;delay_ms(1);CHCLK5_SET;delay_ms(1);CHCLK5_RESET;break;  
        case 5:CHCLK6_RESET;delay_ms(1);CHCLK6_SET;delay_ms(1);CHCLK6_RESET;break;  
        case 6:CHCLK7_RESET;delay_ms(1);CHCLK7_SET;delay_ms(1);CHCLK7_RESET;break;  
        case 7:CHCLK8_RESET;delay_ms(1);CHCLK8_SET;delay_ms(1);CHCLK8_RESET;break;  
				case 8:CHCLK9_RESET;delay_ms(1);CHCLK9_SET;delay_ms(1);CHCLK9_RESET;break;  
        case 9:CHCLK10_RESET;delay_ms(1);CHCLK10_SET;delay_ms(1);CHCLK10_RESET;break;  
        case 10:CHCLK11_RESET;delay_ms(1);CHCLK11_SET;delay_ms(1);CHCLK11_RESET;break;  
        case 11:CHCLK12_RESET;delay_ms(1);CHCLK12_SET;delay_ms(1);CHCLK12_RESET;break;  
				case 12:CHCLK13_RESET;delay_ms(1);CHCLK13_SET;delay_ms(1);CHCLK13_RESET;break;  
        case 13:CHCLK14_RESET;delay_ms(1);CHCLK14_SET;delay_ms(1);CHCLK14_RESET;break;  
				
        default:break;
      }
	
}	


void Sn74374_Clear0(u8 uNumber )
{
	uDqStatus[uNumber]=0;
	RELAYEN1_RESET;
	RELAYEN2_RESET;
	RELAYEN3_RESET;
	RELAYEN4_RESET;
	RELAYEN5_RESET;
	RELAYEN6_RESET;
	RELAYEN7_RESET;
	RELAYEN8_RESET;
	RELAYEN9_RESET;
	RELAYEN10_RESET;
	RELAYEN11_RESET;
	RELAYEN12_RESET;
	RELAYEN13_RESET;
	RELAYEN14_RESET;
	RELAYEN15_RESET;
	RELAYEN16_RESET;
	switch (uNumber)
			{
        case 0:CHCLK1_RESET;delay_ms(1);CHCLK1_SET;delay_ms(1);CHCLK1_RESET;break;  
        case 1:CHCLK2_RESET;delay_ms(1);CHCLK2_SET;delay_ms(1);CHCLK2_RESET;break;  
        case 2:CHCLK3_RESET;delay_ms(1);CHCLK3_SET;delay_ms(1);CHCLK3_RESET;break;  
        case 3:CHCLK4_RESET;delay_ms(1);CHCLK4_SET;delay_ms(1);CHCLK4_RESET;break;  
				case 4:CHCLK5_RESET;delay_ms(1);CHCLK5_SET;delay_ms(1);CHCLK5_RESET;break;  
        case 5:CHCLK6_RESET;delay_ms(1);CHCLK6_SET;delay_ms(1);CHCLK6_RESET;break;  
        case 6:CHCLK7_RESET;delay_ms(1);CHCLK7_SET;delay_ms(1);CHCLK7_RESET;break;  
        case 7:CHCLK8_RESET;delay_ms(1);CHCLK8_SET;delay_ms(1);CHCLK8_RESET;break;  
				case 8:CHCLK9_RESET;delay_ms(1);CHCLK9_SET;delay_ms(1);CHCLK9_RESET;break;  
        case 9:CHCLK10_RESET;delay_ms(1);CHCLK10_SET;delay_ms(1);CHCLK10_RESET;break;  
        case 10:CHCLK11_RESET;delay_ms(1);CHCLK11_SET;delay_ms(1);CHCLK11_RESET;break;  
        case 11:CHCLK12_RESET;delay_ms(1);CHCLK12_SET;delay_ms(1);CHCLK12_RESET;break;  
				case 12:CHCLK13_RESET;delay_ms(1);CHCLK13_SET;delay_ms(1);CHCLK13_RESET;break;  
        case 13:CHCLK14_RESET;delay_ms(1);CHCLK14_SET;delay_ms(1);CHCLK14_RESET;break;  
				
        default:break;
      }
	
}



void Sn74374_Set0(void)
{
	 u8 i;
	 for(i=0;i<14;i++)   //初始化 SN74374 ，设为0
	{
   Sn74374_Clear0(i);
	}	
	
	
}	



void fuhegz_set(u8 uChannel,u8 uSetValue)   //uChannel:0-71
{
	u8 utemp,uDqNumber,uDqPin;
	u16 uDqPinVal;
	u8 uWithload,uSetValue1;
	
	
	 uWithload=(uSetValue>>4)&0x1;  
	 uSetValue1=uSetValue&0xf;
	
    if(uWithload==0)   //wiht no load
		{
		utemp=uChannel*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
	  }
	 
	 
	
	
  if(uSetValue1==2)  // ECU-UBATA+ 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HV_FunRelay_Set(1);
	}		
	else if(uSetValue1==3)  // ECU-UBATA- 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HV_FunRelay_Set(2);
	}		
	else if(uSetValue1==4)  // ECU-UBATB+ 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HV_FunRelay_Set(3);
	}		
	else if(uSetValue1==5)  // ECU-UBATB- 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HV_FunRelay_Set(4);
	}		
	
 uFaultModePreStatus=0;
	
}	


void ddlgz_set(u8 uChannel,u8 uSetValue)   //uChannel:0-71
{
	u8 utemp,uDqNumber,uDqPin;
	u16 uDqPinVal;
	u8 uWithload,uSetValue1;
	
	  uWithload=(uSetValue>>4)&0x1;
	  uSetValue1=uSetValue&0xf;
	
    if(uWithload==0)  //with no load
		{
		utemp=uChannel*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
	  }
	
	
	
	
  if(uSetValue1==2)  // ECU-UBATC+ 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HI_FunRelay_Set(1);
	}		
	else if(uSetValue1==3)  // ECU-UBATC- 短路故障
  {
    utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);
		HI_FunRelay_Set(2);
	}		
	
}	



void kstdgz_set(u8 uChannel,u8 uSetValue)   //uChannel:0-71
{
	u8 i,utemp,uDqNumber,uDqPin;
	u16 uDqPinVal;
	u8 uWithload,uSetValue1;
	
	  uWithload=(uSetValue>>4)&0x1;	
	  uSetValue1=uSetValue&0xf;
		
	  	
    utemp=uChannel*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal); //ECU OPEN TO LOAD
	
		
		
		utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //ECU CONNECT TO BUSIN 
		
	  if(uWithload==1)  //with load
		{	
	  utemp=uChannel*3+2;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //LOAD CONNECT TO BUSOUT
		uKasuStatus[uChannel]=1;
		}	
		
		
	if(uSetValue1==1)   //ECU-LOAD开路故障
	{
		HV_FunRelay_Set(9); // 固态继电器断开
	}	
	else if(uSetValue1==2)  // ECU-UBATA+ 短路故障
  {
   	HV_FunRelay_Set(5); // 固态继电器闭合
	}		
	else if(uSetValue1==3)  // ECU-UBATA- 短路故障
  {
    HV_FunRelay_Set(6); // 固态继电器闭合
	}		
	else if(uSetValue1==4)  // ECU-UBATB+ 短路故障
  {
   	HV_FunRelay_Set(7); // 固态继电器闭合
	}		
	else if(uSetValue1==5)  // ECU-UBATB- 短路故障
  {
    HV_FunRelay_Set(8); // 固态继电器闭合
	}		
		
}


void ksldmn_set(u8 uChannel,u8 uSetValue)   //uChannel:0-71
{
	u8 utemp,uDqNumber,uDqPin;
	u16 uDqPinVal;
	u8 uWithload,uSetValue1;
	
	  uWithload=(uSetValue>>4)&0x1;	
	  uSetValue1=uSetValue&0xf;
		
    utemp=uChannel*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal); //ECU OPEN TO LOAD
		
		utemp=uChannel*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //ECU CONNECT TO BUSIN 
		
	  if(uWithload==1)  //with load
		{
		utemp=uChannel*3+2;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //LOAD CONNECT TO BUSOUT
		uKasuStatus[uChannel]=1;
    }
	
	if(uSetValue1==1)   //ECU-LOAD漏电模拟
	{
		HV_FunRelay_Set(10); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}	
	else if(uSetValue1==2)  // ECU-UBATA+ 漏电模拟
  {
   	HV_FunRelay_Set(5); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}		
	else if(uSetValue1==3)  // ECU-UBATA- 短路故障
  {
    HV_FunRelay_Set(6); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}		
	else if(uSetValue1==4)  // ECU-UBATB+ 短路故障
  {
   	HV_FunRelay_Set(7); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}		
	else if(uSetValue1==5)  // ECU-UBATB- 短路故障
  {
    HV_FunRelay_Set(8); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}		
	else if(uSetValue1==7)  // ECU-LOAD- 电流
  {
    HV_FunRelay_Set(11); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
	}		
	
	
}

	 
void ksldmn_two_set(u8 uChannel1,u8 uChannel2,u8 type)   //
{
	u8 utemp,uDqNumber,uDqPin;
	u16 uDqPinVal;
	

    utemp=uChannel1*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal); //ECU1 OPEN TO LOAD
		
		utemp=uChannel1*3+1;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //ECU1 CONNECT TO BUSIN 
		
		
    utemp=uChannel2*3+0;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,0); //ECU2 CONNECT TO LOAD2
		
		utemp=uChannel2*3+2;
	  uDqNumber=utemp/16;   
	  uDqPin=utemp%16;
		uDqPinVal=0x1<<uDqPin;
		Sn74374_fun(uDqNumber,uDqPinVal);  //LOAD2 CONNECT TO BUSOUT 
		 
	
    if(type==0)
		{	
		HV_FunRelay_Set(10); // 固态继电器闭合
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
		}	
		/*
	  if(type==1)
		{	
		HV_FunRelay_Set(9); // 固态继电器断开
		Resistance_select(uFiuCom_buf[13],uFiuCom_buf[14]);
		}	
    */
	
}




void tdreset_process(void)
{
	Sn74374_Set0();
	HV_FunRelay_Set(0);
	Resistance_select(0,0);	
	HI_FunRelay_Set(0);

}



void fuhegz_process(void)
{
	u8 i,j,utemp,uChan;
	tdreset_process();
		
	for(i=0;i<9;i++)
	{
	if(uFiuCom_buf[i+3]>0)   // channel is selected
	 {
		 for(j=0;j<8;j++)
		 {
			 utemp=(uFiuCom_buf[i+3]>>j)&0x1;   
			 uChan=8*i+j;  //channel number:0-71
			 if(utemp==1)
			 {
        fuhegz_set(uChan,uFiuCom_buf[12]);
			 }				 
		 } 
	 } 
	}
  for (i=0;i<17;i++)
	{
	  uFiuCom_buf[i]=0;
  }		
	
}



void kstdgz_process(void)
{
	u8 i,j,utemp,uChan;

	tdreset_process();
	
	for(i=0;i<9;i++)
	{
	if(uFiuCom_buf[i+3]>0)   // channel is selected
	 {
		 for(j=0;j<8;j++)
		 {
			 utemp=(uFiuCom_buf[i+3]>>j)&0x1;   
			 uChan=8*i+j;  //channel number:0-71
			 if(utemp==1)
			 {
        kstdgz_set(uChan,uFiuCom_buf[12]);
				 
			 }				 
		 } 
	 } 
	} 
	for (i=0;i<17;i++)
	{
	  uFiuCom_buf[i]=0;
  }		
	
}	

void ksldmn_process(void)
{
	u8 i,j,utemp,uChan,t,uChanSet[4];
	u8 uFiuCom_Buftemp;
	
	uFiuCom_Buftemp=uFiuCom_buf[12]&0xf;
	
	tdreset_process();
			 
	t=0;
	if(((uFiuCom_Buftemp>=0)&&(uFiuCom_Buftemp<6))|(uFiuCom_Buftemp==7))
	{
   for(i=0;i<9;i++)
	 {
	 if(uFiuCom_buf[i+3]>0)   // channel is selected
	  {
		 for(j=0;j<8;j++)
		 {
			 utemp=(uFiuCom_buf[i+3]>>j)&0x1;   
			 uChan=8*i+j;  //channel number:0-71
			 if(utemp==1)
			 {
        ksldmn_set(uChan,uFiuCom_buf[12]);
			 }				 
		 } 
	  } 
	 } 
  } 	

	
	if (uFiuCom_Buftemp==0x06 )  //ecu-ecu connect
	 {	
	 for(i=0;i<9;i++)
	  {
	 if(uFiuCom_buf[i+3]>0)   // channel is selected
	   {
		 for(j=0;j<8;j++)
		  {
			 utemp=(uFiuCom_buf[i+3]>>j)&0x1;   
			 uChan=8*i+j;  //channel number:0-71
			 if(utemp==1)
			 {
				 uChanSet[t++]=uChan;
			 }				 
		  } 
	   } 
	  }
		  if(t==2)
	    ksldmn_two_set(uChanSet[0],uChanSet[1],0);
	}	
	 
		
	for (i=0;i<17;i++)
	{
	  uFiuCom_buf[i]=0;
  }		
	
}	



void ddltdgz_process(void)
{
	u8 i,j,utemp,uChan;
	
	
	tdreset_process();
	
	
	for(i=0;i<9;i++)
	{
	if(uFiuCom_buf[i+3]>0)   // channel is selected
	 {
		 for(j=0;j<8;j++)
		 {
			 utemp=(uFiuCom_buf[i+3]>>j)&0x1;   
			 uChan=8*i+j;  //channel number:0-71
			 if(utemp==1)
			 {
        ddlgz_set(uChan,uFiuCom_buf[12]);
			 }				 
		 } 
	 } 
	}
  for (i=0;i<17;i++)
	{
	  uFiuCom_buf[i]=0;
  }		
	
}	





void FiuCom_Process(u8 uType)
{
	u8 i;
 // if(uType==0)    // ethernet command
 // {	
	// for (i=0;i<17;i++)
	// {
	 // uFiuCom_buf[i]=udp_demo_recvbuf[i];
  // }	
 // }	
 // else if(uType==1)  // can command
 // {
  // for (i=0;i<17;i++)
	// {
	 // uFiuCom_buf[i]=cancommandbuf[i];
  // }	
 // }	 
 
  for (i=0;i<17;i++)
	{
	 uFiuCom_buf[i]=g_FiuByte17Data[i];
	}
		
	if((uFiuCom_buf[0]==0xbe) && (uFiuCom_buf[1]==0xbe) && (uFiuCom_buf[15]==0xed) && (uFiuCom_buf[16]==0xed))
	{
		
		switch (uFiuCom_buf[2])
			{
        case 0:fuhegz_process();break;  //复合故障
        case 1:kstdgz_process();break;  //快速通道故障
        case 2:ksldmn_process();break;  //快速漏电模拟
        case 3:ddltdgz_process();break;  //大电流故障
				case 4:tdreset_process();break;  //?
        default:printf("error\n");break;
      }
		

  } 		
	
	//HAL_UART_Transmit(&UART1_Handler,uFiuCom_buf,14,1000);	//发送接收到的数据
	//while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);		//等待发送结束	
	

	

	
}	



void HI_FunRelay_Set(u8 uSetValue)
{
  if (uSetValue==0)  //复位
	{
		
		CTRLBUS2_RESET;
		CTRLBUS14_RESET;
		CTRLBUS15_RESET;
		CTRLBUS16_RESET;
		CTRLBUS17_RESET;	
		
	}
	else if(uSetValue==1)  //复合故障,对UBATC+
	{
	  CTRLBUS2_RESET;
		CTRLBUS14_RESET;
		CTRLBUS15_RESET;
		CTRLBUS16_RESET;
		CTRLBUS17_RESET;	
	}
	else if(uSetValue==2)  //复合故障,对UBATC-
	{
	  CTRLBUS2_RESET;
		CTRLBUS14_RESET;
		CTRLBUS15_RESET;
		CTRLBUS16_SET;
		CTRLBUS17_RESET;	
	}
	
}	



void HV_FunRelay_Set(u8 uSetValue)
{
  if (uSetValue==0)  //复位
	{
		CTRLBUS1_RESET;
		CTRLBUS2_RESET;
		CTRLBUS3_RESET;
		CTRLBUS4_RESET;
		CTRLBUS5_RESET;
	  CTRLBUS6_RESET;
		CTRLBUS7_RESET;
		CTRLBUS8_RESET;
		CTRLBUS9_RESET;
		CTRLBUS10_RESET;
		CTRLBUS11_RESET;
		CTRLBUS12_RESET;
		CTRLBUS13_RESET;
		CTRLBUS14_RESET;
		CTRLBUS15_RESET;
		CTRLBUS16_RESET;
		CTRLBUS17_RESET;	
		
	}
	else if(uSetValue==1)  //复合故障,对UBATA+
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_RESET;	
  CTRLBUS6_RESET;			
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_SET;	
  CTRLBUS13_SET;		
	}
	else if(uSetValue==2)  //复合故障,对UBATA-
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_RESET;
  CTRLBUS6_RESET;		
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_SET;	
  CTRLBUS13_RESET;		
	}
	else if(uSetValue==3)  //复合故障,对UBATB+
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_RESET;		
  CTRLBUS6_RESET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_RESET;	
  CTRLBUS12_SET;		
	}
	else if(uSetValue==4)  //复合故障,对UBATB-
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_RESET;	
  CTRLBUS6_RESET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_RESET;	
  CTRLBUS12_RESET;		
	}
	
	
	else if(uSetValue==5)  //快速切换,对UBATA+
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_SET;	
  CTRLBUS13_SET;		
	}
	else if(uSetValue==6)  //快速切换,对UBATA-
	{
	CTRLBUS1_SET;
  CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;		
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_SET;	
  CTRLBUS13_RESET;		
	}
	else if(uSetValue==7)  //快速切换,对UBATB+
	{
	CTRLBUS1_SET;
  CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;		
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_RESET;	
  CTRLBUS12_SET;		
	}
	else if(uSetValue==8)  //快速切换,对UBATB-
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;	
	CTRLBUS7_RESET;  		
	CTRLBUS10_SET;	
  CTRLBUS11_RESET;	
  CTRLBUS13_RESET;		
	}
	
	else if(uSetValue==9)  //快速切换,对ECU OPEN TO LOAD
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_RESET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_RESET;	
  CTRLBUS11_RESET;	
  CTRLBUS13_RESET;		
	}
	

	else if(uSetValue==10)  //快速切换,对ECU CONNECT TO LOAD
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_RESET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_RESET;	
  CTRLBUS11_RESET;	
  CTRLBUS13_RESET;		
	}
	
	else if(uSetValue==11)  //快速切换,电流测量
	{
	CTRLBUS1_SET;
	CTRLBUS2_RESET;	
	CTRLBUS3_SET;	
	CTRLBUS4_RESET;	
  CTRLBUS5_SET;	
  CTRLBUS6_SET;				
	CTRLBUS7_RESET;  		
	CTRLBUS10_RESET;	
  CTRLBUS11_RESET;	
  CTRLBUS13_RESET;		
	}
	
	
}	



void RelayIO_Init(void)
{
	  u8 i,ufuseresult;
	
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
	  __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOC时钟
	  __HAL_RCC_GPIOD_CLK_ENABLE();           //开启GPIOD时钟
	  __HAL_RCC_GPIOE_CLK_ENABLE();           //开启GPIOD时钟
	  __HAL_RCC_GPIOF_CLK_ENABLE();           //开启GPIOD时钟
	  __HAL_RCC_GPIOG_CLK_ENABLE();           //开启GPIOD时钟
    __HAL_RCC_GPIOH_CLK_ENABLE();           //开启GPIOH时钟
	  __HAL_RCC_GPIOI_CLK_ENABLE();           //开启GPIOI时钟
	
	  
	  GPIO_Initure.Pin=GPIO_PIN_12; //PH12 :CH_EN 
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
	
	  GPIO_Initure.Pin=GPIO_PIN_15|GPIO_PIN_2; //PH15  CHCLK1
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
	
	
	  GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_8; //PI0-PI3 :CHCLK2 - CHCLK5,CTRL_BUS3-CTRL_BUS5
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;             //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);  
	
	
	  GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6; //PA15:CHCLK6,PA0:CRTL_BUS16,PA3:RUN LED,PA5:FUSE1TEST
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;             //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
		
		GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13; //PC10-PC12 :CHCLK7 - CHCLK9
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;             //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOC,&GPIO_Initure); 
	
	
		GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7; //PD3-PD7 :CHCLK10- CHCLK14
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOD,&GPIO_Initure); 
		
		
		
		
		
		
		GPIO_Initure.Pin=GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1; //PB13-PB15 :RELAY_EN1- RELAY_EN3,FUSETEST3-4
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;         //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure); 
		
		
		
		GPIO_Initure.Pin=GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13; //PD11-PD13 :RELAY_EN4- RELAY_EN6
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOD,&GPIO_Initure); 
		
		
	  GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7; //PG3,PG6-PG7 :RELAY_EN7- RELAY_EN9
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOG,&GPIO_Initure); 
		
	
	  GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9; //PC6-PC9 :RELAY_EN10- RELAY_EN13,
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;         //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOC,&GPIO_Initure); 
		
	
	  GPIO_Initure.Pin=GPIO_PIN_8; //PA8:RELAY_EN14
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
		
		
		GPIO_Initure.Pin=GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_6; //PH13-PH14 :RELAY_EN15- RELAY_EN16,
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;         //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
		

		GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_14|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10; //CTRL_BUS1,CTRL_BUS6,CTRL_BUS7
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;         //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);
		
		GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6; //CTRL_BUS9-CTRL_BUS13
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		
		GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;           //PH7-PH11
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
		
		
		GPIO_Initure.Pin=GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;    //RESCTRL1-4
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;             //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);  
		
		
				
		GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;    //RESCTRL5-11
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;             //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure); 
		
		
		
		GPIO_Initure.Pin=GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12; //RESCTRL12-15
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLDOWN;          //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOG,&GPIO_Initure); 
		
		
		
		ufuseresult=Fuse_test();
		printf("fuse test result value=%d\r\n",ufuseresult);
	
	  CH_EN_SET;    //DQ OE ENABLE
	  Sn74374_Set0();  
		HV_FunRelay_Set(0);  //reset
		for(i=0;i<CHANNUMBER;i++)
	  {uKasuStatus[i]=0;uKamnStatus[i]=0;}
		
		for(i=0;i<15;i++)
		uResistance[i]=pow(2,i);
		
		uFaultModeStatus=0xff;
		uFaultModePreStatus=0xff;
		
		
		
		
		
	
		
		while(ufuseresult>0)   //死循环
	{
  ;

  }		
		
		
		
	
	/*
	     
    GPIO_Initure.Pin=GPIO_PIN_2;           //PB2
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    
  
  */ 
	 
	  
	 
	 
}





