#include "comm.h"
#include "string.h"
#include "pcf8574.h"
#include "can.h"
#include "timer.h"

#define CHANNUMBER 72

extern uint32_t    g_LocalCanId;

uint8_t		g_FiuByte17Data[17] = {0};
uint8_t 	g_HOSTCANDATABUF[CANBUFDATALEN];
uint8_t 	g_DataFromHost[8];
uint16_t 	g_DealDataCur = 0;
uint16_t 	g_IntrDataCur = 0;

uint64_t P0, P1, P2;	//P0 P1 的 72 位 P0=0~63 P1=0~7 代表通道号
						//，P1=8~11 代表故障类型，P1=12~15 代表是否带载 
						//（对应 FIU命令.docx文档）
uint16_t P3_0, P3_1, P3_2, P3_3, P4_0, P4_1;	//，p4_0  代表电阻值

uint16_t g_Ptime;
uint8_t  g_Pfreq;
uint8_t  g_Pduty;
uint8_t  g_CRC;
uint8_t  g_Cmd3Err3Cnt;

uint8_t g_bLoose;
uint8_t g_bFreq;
uint8_t g_bDuty;
uint8_t g_bTime;

uint8_t g_HostConfigPass;
uint8_t g_HostConfigErr[ERRORNUM];

uint8_t g_SendTxcmd;
uint8_t FIU_CurCmd;
int8_t 	FIU_Cmd0_CurErrorType;
uint8_t FIU_Cmd0_CurPinFlag[ERRORNUM];
uint8_t FIU_Cmd1_Flag;
uint8_t FIU_Cmd2_Flag;
uint8_t FIU_Cmd2_ErrorType0Flag;
uint8_t FIU_Cmd2_ErrorType0FlagWithLoadFlag;
uint8_t FIU_Cmd2_ErrorType0Cnt;
uint8_t FIU_Cmd3_Flag;
uint8_t FIU_Cmd3_ErrorType3Flag;
uint8_t FIU_Cmd3_ErrorType3Cnt;
uint8_t FIU_Cmd4_FreqDutyFlag;
uint8_t FIU_Cmd4_ResistanceFlag;
uint8_t FIU_Cmd2_Err0;

uint8_t 	g_WorkMode = 0;

extern uint8_t g_Error;

void GlobalDataInit(void);
void FIU_Cmd0_CurPinFlag_init(void);
uint64_t McuFault64BitValuePinClr(uint64_t ToClrValue, uint8_t PinNum);
uint64_t McuFault64BitValuePinSet(uint64_t ToSetValue, uint8_t PinNum);
void P_init(void);
uint8_t McuGetHostConfigInfo(void);
uint8_t HostConfigCheck(uint8_t canByte0,uint8_t canByte1,uint8_t canByte2,uint8_t canByte3,uint8_t canByte4,uint8_t canByte5,uint8_t canByte6,uint8_t canByte7);
uint8_t DataAtPxToFiuBytes17(void);
void FiuCom_Process(void);
extern uint8_t Fuse_check(void);

extern void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);
extern void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite);

void startFault(void)
{	
	FiuCom_Process();
	
	if (g_bLoose == 1) {
		TIM5->ARR = (1/g_Pfreq*1000000)*g_Pduty/100;
		HAL_TIM_Base_Start_IT(&TIM5_Handler);
		
		HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);
		TIM2->ARR = 1/g_Pfreq*1000000;
		HAL_TIM_Base_Start_IT(&TIM2_Handler);
	}	
	if(g_Ptime > 0){
		TIM5->ARR = g_Ptime;
		HAL_TIM_Base_Start_IT(&TIM3_Handler); //g_Ptime ms后中断中复位
	}
}

void HostCanDataAnalysisNoLoopBuf(uint8_t *data) //udp deal
{
    uint8_t result;
	uint8_t tmp_flash[FLASH_PROC_SIZE];
	uint8_t tmp_flash_can[FLASH_PROC_SIZE_CAN];
	int i;
	
        memcpy(g_DataFromHost,data,8);
		{
            if((0x00 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error; //将错误返回到 8 字节协议组对应位置
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
                }
            }
            
            if((0x01== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
                }
            }
            
            if((0x02== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
                }
            }
            
            if((0x03== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
                }
            }
            
            if((0x04== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[2] = result;
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[2] = result;
                }
            }
            
            if((0x5 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host config done
            {
				
                g_Ptime = (g_DataFromHost[3] << 8) | g_DataFromHost[2];
                if(g_Error != 0){
                   g_DataFromHost[2] = g_Error;
                }
                else if(g_Ptime >65500)
                {
                    g_DataFromHost[2] = g_HostConfigErr[1];
					P_init();
					FIU_Cmd0_CurPinFlag_init();
                }
                else
                {
					if (g_bTime == 0) {
						g_bTime = 1;
					}
                    result = DataAtPxToFiuBytes17();
                    if(result == g_HostConfigPass)
                    {
                        g_DataFromHost[2] = result;
						FIU_Cmd0_CurPinFlag_init();
                    }
                    else
                    {
                        g_DataFromHost[2] = result;
						P_init();
						FIU_Cmd0_CurPinFlag_init();
                    }
                }
            }

            if((0x10 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host start fault .
            {
				//FiuCom_Process();
				startFault();
                if(g_Error != 0)
                    g_DataFromHost[2] = g_Error;
            }
            
            if((0x11 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host reset relay
            {
				g_FiuByte17Data[0] = 0xbe;
				g_FiuByte17Data[1] = 0xbe;
				
				g_FiuByte17Data[2] = 4;
				
				g_FiuByte17Data[15] = 0xed;
				g_FiuByte17Data[16] = 0xed;
				
				FiuCom_Process();				
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else{
                    g_DataFromHost[2] = 0;
                    {
                        P_init();
                        FIU_Cmd0_CurPinFlag_init();
                    }
                }
            }
			
			if((0x1A == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host test fuses
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else{
                    {
                        P_init();
                        FIU_Cmd0_CurPinFlag_init();
                    }
                    result = Fuse_check();
                    if(g_Error != 0){
                         g_DataFromHost[2] = g_Error;
                     }
                    else if(result == 0x1f)
                    { 
                        g_DataFromHost[2] = 0;
                        g_DataFromHost[3] = 1;
                        g_DataFromHost[4] = 1;
                        g_DataFromHost[5] = 1;
                        g_DataFromHost[6] = 1;
                        g_DataFromHost[7] = 1;
                  //      g_FuseStatus = 0;
                    }
                   else
                   {
                        g_DataFromHost[2] = g_HostConfigErr[51];
                        g_DataFromHost[3] = (result&(1<<0));
                        g_DataFromHost[4] = (result&(1<<1))>>1;
                        g_DataFromHost[5] = (result&(1<<2))>>2;
                        g_DataFromHost[6] = (result&(1<<3))>>3;
                        g_DataFromHost[7] = (result&(1<<4))>>4;
//                        g_FuseStatus = 0;
                    }
                }   				
            }
			
			if((0x15 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host will set ip
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else{
					STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp_flash,FLASH_PROC_SIZE/4);
					for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
						tmp_flash[i] = 0xaa; 
						tmp_flash[i+1] = 0x55;
					}
					
					tmp_flash[64] = g_DataFromHost[2];
					tmp_flash[65] = g_DataFromHost[3];
					tmp_flash[66] = g_DataFromHost[4];
					tmp_flash[67] = g_DataFromHost[5];					
					
					STMFLASH_Write(FLASH_PROCHEAD,(u32*)tmp_flash,FLASH_PROC_SIZE/4);

                    g_DataFromHost[2] = 0;
                }
            }
			
            if((0x1C == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host will RESTART 
            {
				__set_FAULTMASK(1);
				HAL_NVIC_SystemReset();
            }
			
			if((0x17 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host set can send id
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else {
					STMFLASH_Read(FLASH_PROCHEAD_CAN,(u32 *)tmp_flash_can,FLASH_PROC_SIZE_CAN/4);
					for (i=0;i<FLASH_PROCHEAD_SIZE_CAN;i=i+2) {
						tmp_flash_can[i] = 0xaa; 
						tmp_flash_can[i+1] = 0x55;
					}
					tmp_flash_can[48] = g_DataFromHost[2];
					tmp_flash_can[49] = g_DataFromHost[3];
					tmp_flash_can[50] = g_DataFromHost[4];
					tmp_flash_can[51] = g_DataFromHost[5];
					
					STMFLASH_Write(FLASH_PROCHEAD_CAN,(u32*)tmp_flash_can,FLASH_PROC_SIZE_CAN/4);
					
					//g_LocalCanId = (g_DataFromHost[5]<<24) | (g_DataFromHost[4]<<16) | (g_DataFromHost[3]<<8) | (g_DataFromHost[2]); //重启后再生效
                    g_DataFromHost[2] = 0;
				}
			}
#if 0
			if((0x1E == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host set can res 
            {
				#if 0
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
              //      CAN_TX_EID_Message(g_CanSendId);
                }
                else if(g_DataFromHost[2] == 0x1)
                {
//                    PLIB_PORTS_PinWrite( PORTS_ID_0 , PORT_CHANNEL_D , PORTS_BIT_POS_9,1);//connect
//                    Delay_ByTimer3(4000);
//                    g_DataFromHost[2] = 0;
//                    CAN_TX_EID_Message(g_CanSendId);
                }
                else if(g_DataFromHost[2] == 0x0)
                {
//                    PLIB_PORTS_PinWrite( PORTS_ID_0 , PORT_CHANNEL_D , PORTS_BIT_POS_9,0);//disconnect
//                    Delay_ByTimer3(4000);
//                    g_DataFromHost[2] = 0;
//                    CAN_TX_EID_Message(g_CanSendId);
                }
                else
                {
//                    g_DataFromHost[2] = g_HostConfigErr[48];
//                    CAN_TX_EID_Message(g_CanSendId);
                }
				#endif
            }
			
			if((0x1B == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host will SELFtest 
            {
//                McuSendSelfTest2();
//                result = McuRecvSelfTest2();
//                if(result == g_HostConfigPass)
//                {
                   // McuSendSelfTest1();
                   // result = McuRecvSelfTest2();
                    //if(g_Error != 0){
                    //    g_DataFromHost[2] = g_Error;
                 //       CAN_TX_EID_Message(g_CanSendId);
                  //  }
                   // else if(result == g_HostConfigPass)
                 //   {
                 //       g_DataFromHost[2] = result;
                    //    CAN_TX_EID_Message(g_CanSendId);
                //    }
                 //   else
                //    {
                //        g_DataFromHost[2] = result;
                 //       CAN_TX_EID_Message(g_CanSendId);
                //    }
//                }
//                else
//                {
//                    g_DataFromHost[2] = result;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }                 
            }
			
            if(0x12 == g_DataFromHost[0])                                       //host want workmode
            {
                if(g_Error != 0){
                    g_DataFromHost[1] = g_Error;
              //      CAN_TX_EID_Message(g_CanSendId);
                }
                else{
                    g_DataFromHost[1] = 0;
                    g_DataFromHost[2] = g_WorkMode;
               //     CAN_TX_EID_Message(g_CanSendId);
                }
            }

            if(0x13 == g_DataFromHost[0])                                   //host set workmode
            {
//                g_I2CDATA[3] = g_WorkMode = g_DataFromHost[1];
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
             //       CAN_TX_EID_Message(g_CanSendId);
                }
                else {
//                    g_I2CDATA[3] = g_DataFromHost[1];
//                    EE_SEQU_Write(0x0000,16,g_I2CDATA);      
//                    Delay_ByTimer3(1000); 
//                    EE_SEQU_Read(0x0000,16,g_I2CDATANOUSED);

                    g_DataFromHost[2] = 0;
             //       CAN_TX_EID_Message(g_CanSendId);
                }
            }

            if(0x14 == g_DataFromHost[0])                                       //host want ip
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
              //      CAN_TX_EID_Message(g_CanSendId);
                }
                else{
                    g_DataFromHost[2] = 0;
//                    g_DataFromHost[3] = g_I2CDATA[15];
//                    g_DataFromHost[4] = g_I2CDATA[14];
//                    g_DataFromHost[5] = g_I2CDATA[13];
//                    g_DataFromHost[6] = g_I2CDATA[12];

//                    CAN_TX_EID_Message(g_CanSendId);
                }
            }
            

            if((0x16 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host want can send id
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                 //   CAN_TX_EID_Message(g_CanSendId);
                }
                else {
                    g_DataFromHost[2] = 0;
//                    g_DataFromHost[6] = ((g_CanSendId >> 24) & 0xff);
//                    g_DataFromHost[5] = ((g_CanSendId >> 16) & 0xff);
//                    g_DataFromHost[4] = ((g_CanSendId >> 8) & 0xff);
//                    g_DataFromHost[3] = (g_CanSendId & 0xff);

//                    CAN_TX_EID_Message(g_CanSendId);
                }
            }
            
            

            if((0x18 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host want can recv id
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                //    CAN_TX_EID_Message(g_CanSendId);
                }
                else{
                    g_DataFromHost[2] = 0;
//                    g_DataFromHost[6] = ((g_CanRecvId >> 24) & 0xff);
//                    g_DataFromHost[5] = ((g_CanRecvId >> 16) & 0xff);
//                    g_DataFromHost[4] = ((g_CanRecvId >> 8) & 0xff);
//                    g_DataFromHost[3] = (g_CanRecvId & 0xff);

//                    CAN_TX_EID_Message(g_CanSendId);
                }
            }
            if((0x19 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host set can recv id
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                   // CAN_TX_EID_Message(g_CanSendId);
                }
                else{
//                    g_I2CDATA[11] = g_DataFromHost[2];
//                    g_I2CDATA[10] = g_DataFromHost[3];
//                    g_I2CDATA[9] = g_DataFromHost[4];
//                    g_I2CDATA[8] = g_DataFromHost[5];
//    //                g_CanRecvId = (g_I2CDATA[8]<<24) | (g_I2CDATA[9]<<16) | (g_I2CDATA[10]<<8) | (g_I2CDATA[11]);
//                    g_I2CDATA[1] = 1;
//                    EE_SEQU_Write(0x0000,16,g_I2CDATA);  
//                    Delay_ByTimer3(1000);
//                    EE_SEQU_Read(0x0000,16,g_I2CDATANOUSED);

//                    g_DataFromHost[2] = 0;
//                    CAN_TX_EID_Message(g_CanSendId);
                }
            }

            if((0x1D == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host want fpga workstatus 
            {
				#if 0
                McuSendFpgaStatusCheck();
                result = McuRecvFpgaStatusCheck();
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
              //      CAN_TX_EID_Message(g_CanSendId);
                }
                else if((result == 0x1) || (result == 0x2))
                {
                    g_DataFromHost[2] = 0;
                    g_DataFromHost[3] = result - 1;
              //      CAN_TX_EID_Message(g_CanSendId);
                }
                else
                {
                    g_DataFromHost[2] = result;
             //       CAN_TX_EID_Message(g_CanSendId);
                }
				#endif
            }

            if((0x20 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host want fpga verison . 
            {
//                McuSendGetFpgaVersionAndTime();
//                result = McuRecvFpgaVersionAndTime();
//                if(g_Error != 0){
//                    g_DataFromHost[2] = g_Error;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
//                else if(result == g_HostConfigPass)
//                {
//                    g_DataFromHost[2] = 0;
//                    memcpy(&g_DataFromHost[3],&g_FROMFPGADATABUF[8],4);
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
//                else
//                {
//                    g_DataFromHost[2] = result;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
//                
            }
            
            if((0x21 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host want fpga time . 
            {
//                McuSendGetFpgaVersionAndTime();
//                result = McuRecvFpgaVersionAndTime();
//                if(g_Error != 0){
//                    g_DataFromHost[2] = g_Error;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
//                else if(result == g_HostConfigPass)
//                {
//                    g_DataFromHost[2] = 0;
//                    memcpy(&g_DataFromHost[3],&g_FROMFPGADATABUF[4],4);
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
//                else
//                {
//                    g_DataFromHost[2] = result;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
            }
            
            if((0x22 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//get config history . 
            {
                //TODO
//                McuSendGetHistoryConfigInfo();
//                result = McuRecvHistoryConfigInfo();
//                if(result == g_HostConfigPass)
//                {
                  //  g_DataFromHost[2] = 0;
//                    memcpy(&g_DataFromHost[3],&g_FROMFPGADATABUF[4],4);
                 //   CAN_TX_EID_Message(g_CanSendId);
//                }
//                else
//                {
//                    g_DataFromHost[2] = result;
//                    CAN_TX_EID_Message(g_CanSendId);
//                }
            }
#endif
        }
		
		memcpy(data,g_DataFromHost,8);
}

int HostCanDataAnalysis(void) //can deal
{
    uint8_t result;
	uint8_t tmp_flash[FLASH_PROC_SIZE];
	uint8_t tmp_flash_can[FLASH_PROC_SIZE_CAN];
	int i;
    
    if(g_IntrDataCur != g_DealDataCur)
    {
        memcpy(g_DataFromHost,&g_HOSTCANDATABUF[g_DealDataCur],8);
		{
            if((0x00 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error; //将错误返回到 8 字节协议组对应位置
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x01== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x02== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x03== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[3] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[3] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x04== g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))
            {
                result = McuGetHostConfigInfo();
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(result == g_HostConfigPass)
                {
                    g_DataFromHost[2] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else
                {
                    FIU_Cmd0_CurPinFlag_init();
                    g_DataFromHost[2] = result;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x5 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host config done
            {
				
                g_Ptime = (g_DataFromHost[2] << 8) | g_DataFromHost[3];
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else if(g_Ptime >65500)
                {
                    g_DataFromHost[2] = g_HostConfigErr[1];
					CAN1_Send_Msg(g_DataFromHost,8);
                    g_DealDataCur += 8;
                    g_DealDataCur %= CANBUFDATALEN;
                    {
                        P_init();
                        FIU_Cmd0_CurPinFlag_init();
                    }
                    return 0;
                }
                else
                {
                    g_bTime = 1;
                    result = DataAtPxToFiuBytes17();
                    if(result == g_HostConfigPass)
                    {
                        g_DataFromHost[2] = result;
						CAN1_Send_Msg(g_DataFromHost,8);
                        g_DealDataCur += 8;
                        g_DealDataCur %= CANBUFDATALEN;
                            
						FIU_Cmd0_CurPinFlag_init();
                        
						return 1;
                    }
                    else
                    {
                        g_DataFromHost[2] = result;
						CAN1_Send_Msg(g_DataFromHost,8);
                        g_DealDataCur += 8;
                        g_DealDataCur %= CANBUFDATALEN;
                        {
                            P_init();
                            FIU_Cmd0_CurPinFlag_init();
                        }
                        return 0;
                    }
                }
            }

            if((0x10 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host start fault .
            {
				//FiuCom_Process();
				startFault();
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
            
            if((0x11 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host reset relay
            {
				g_FiuByte17Data[0] = 0xbe;
				g_FiuByte17Data[1] = 0xbe;
				
				g_FiuByte17Data[2] = 4;
				
				g_FiuByte17Data[15] = 0xed;
				g_FiuByte17Data[16] = 0xed;
				
				FiuCom_Process();				
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else{
                    g_DataFromHost[2] = 0;
					CAN1_Send_Msg(g_DataFromHost,8);
                    {
                        P_init();
                        FIU_Cmd0_CurPinFlag_init();
                    }
                }
            }
			
			if((0x1A == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host test fuses
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else{
                    {
                        P_init();
                        FIU_Cmd0_CurPinFlag_init();
                    }
                    result = Fuse_check();
                    if(g_Error != 0){
                         g_DataFromHost[2] = g_Error;
						CAN1_Send_Msg(g_DataFromHost,8);
                     }
                    else if(result == 0x1f)
                    { 
                        g_DataFromHost[2] = 0;
                        g_DataFromHost[3] = 1;
                        g_DataFromHost[4] = 1;
                        g_DataFromHost[5] = 1;
                        g_DataFromHost[6] = 1;
                        g_DataFromHost[7] = 1;
						CAN1_Send_Msg(g_DataFromHost,8);
                  //      g_FuseStatus = 0;
                    }
                   else
                   {
                        g_DataFromHost[2] = g_HostConfigErr[51];
                        g_DataFromHost[3] = (result&(1<<0));
                        g_DataFromHost[4] = (result&(1<<1))>>1;
                        g_DataFromHost[5] = (result&(1<<2))>>2;
                        g_DataFromHost[6] = (result&(1<<3))>>3;
                        g_DataFromHost[7] = (result&(1<<4))>>4;
						CAN1_Send_Msg(g_DataFromHost,8);
//                        g_FuseStatus = 0;
                    }
                }   				
            }
			
			if((0x15 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host will set ip
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
                else{
					STMFLASH_Read(FLASH_PROCHEAD,(u32 *)tmp_flash,FLASH_PROC_SIZE/4);
					for (i=0;i<FLASH_PROCHEAD_SIZE;i=i+2) {
						tmp_flash[i] = 0xaa; 
						tmp_flash[i+1] = 0x55;
					}
					
					tmp_flash[64] = g_DataFromHost[2];
					tmp_flash[65] = g_DataFromHost[3];
					tmp_flash[66] = g_DataFromHost[4];
					tmp_flash[67] = g_DataFromHost[5];					
					
					STMFLASH_Write(FLASH_PROCHEAD,(u32*)tmp_flash,FLASH_PROC_SIZE/4);

                    g_DataFromHost[2] = 0;
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
			
            if((0x1C == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host will RESTART 
            {
				__set_FAULTMASK(1);
				HAL_NVIC_SystemReset();
            }
			
			if((0x17 == g_DataFromHost[0]) && (g_WorkMode == g_DataFromHost[1]))//host set can send id
            {
                if(g_Error != 0){
                    g_DataFromHost[2] = g_Error;
                }
                else {
					STMFLASH_Read(FLASH_PROCHEAD_CAN,(u32 *)tmp_flash_can,FLASH_PROC_SIZE_CAN/4);
					for (i=0;i<FLASH_PROCHEAD_SIZE_CAN;i=i+2) {
						tmp_flash_can[i] = 0xaa; 
						tmp_flash_can[i+1] = 0x55;
					}
					tmp_flash_can[48] = g_DataFromHost[2];
					tmp_flash_can[49] = g_DataFromHost[3];
					tmp_flash_can[50] = g_DataFromHost[4];
					tmp_flash_can[51] = g_DataFromHost[5];
					
					STMFLASH_Write(FLASH_PROCHEAD_CAN,(u32*)tmp_flash_can,FLASH_PROC_SIZE_CAN/4);
					
					//g_LocalCanId = (g_DataFromHost[5]<<24) | (g_DataFromHost[4]<<16) | (g_DataFromHost[3]<<8) | (g_DataFromHost[2]); //重启后再生效
                    g_DataFromHost[2] = 0;
                
					CAN1_Send_Msg(g_DataFromHost,8);
                }
            }
		}

        g_DealDataCur += 8;
        g_DealDataCur %= CANBUFDATALEN;
		
		return 1;
    }
    
    return 0;
}

uint8_t McuGetHostConfigInfo(void) //组织数据,仿照老版 fiu 同名函数
{
    uint8_t result = 0;
    uint8_t nCurCommandVal = g_DataFromHost[0];
    uint8_t nCurPinNum = g_DataFromHost[2];
    uint8_t nCurPinNum1 = g_DataFromHost[5];
    uint8_t nCurErrorType = g_DataFromHost[3];
    uint8_t nCurWithLoad = g_DataFromHost[4];
    uint8_t nCurCanDataByte6 = g_DataFromHost[6];
    uint8_t nCurCanDataByte7 = g_DataFromHost[7];
    
    result = HostConfigCheck(nCurCommandVal,g_DataFromHost[1],nCurPinNum,nCurErrorType,nCurWithLoad,nCurPinNum1,nCurCanDataByte6,nCurCanDataByte7);
    if(result != g_HostConfigPass)
    {
        GlobalDataInit();
        return (result);//failed
    }

    if(nCurCommandVal == 0)	//复合故障
    {        
        if(nCurErrorType == 0) // 开路
        {
			//pin
			if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			//errT
			P1 = McuFault64BitValuePinSet(P1,8); //开路对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x1 = b0001
        }
        else if(nCurErrorType == 1)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,9); //对UbattA+ 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x2 = b0010
        
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //对UbattA+ 对应 FIU命令.docx 的 Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[2];
            }
        }
        else if(nCurErrorType == 2)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,9); //对UbattA- 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x3 = b0011
			P1 = McuFault64BitValuePinSet(P1,8);
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[3];
            }
        }
        else if(nCurErrorType == 3)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,10); //对UbattB+短路 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x4 = b0100
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[4];
            }
        }
        else if(nCurErrorType == 4)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,10); //对UbattB- 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x5 = b0101
			P1 = McuFault64BitValuePinSet(P1,8);
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[5];
            }
        }
        else
        {
            P_init();
            return g_HostConfigErr[6];
        }
    }
    else if(nCurCommandVal == 1) //快速通道故障
    {
        if(nCurErrorType == 0) // 开路
        {
			//pin
			if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			//errT
			P1 = McuFault64BitValuePinSet(P1,8); //开路对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x1 = b0001
        }
        else if(nCurErrorType == 1)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,9); //对UbattA+ 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x2 = b0010
        
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //对UbattA+ 对应 FIU命令.docx 的 Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[7];
            }
        }
        else if(nCurErrorType == 2)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,9); //对UbattA- 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x3 = b0011
			P1 = McuFault64BitValuePinSet(P1,8);
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[8];
            }
        }
        else if(nCurErrorType == 3)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,10); //对UbattB+短路 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x4 = b0100
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[9];
            }
        }
        else if(nCurErrorType == 4)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,10); //对UbattB- 对应 FIU命令.docx 的 Byte9的Bit0~Bit3:0x5 = b0101
			P1 = McuFault64BitValuePinSet(P1,8);
        
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); //Byte9的Bit4~Bit7: with load,1 = b0001xxxx
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else
            {
                P_init();
                return g_HostConfigErr[10];
            }
        }
        else
        {
            P_init();
            return g_HostConfigErr[11];
        }
    }
    else if(nCurCommandVal == 2) //快速切换 漏电模拟
    {
        if(nCurErrorType == 0) //ecu-load
        {
            if(nCurWithLoad == 1)
            {
                if (nCurPinNum < CHANNUMBER)	
					P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
				else
					P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
				
				P1 = McuFault64BitValuePinSet(P1,8); 
				P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0)
            {
                if (nCurPinNum < CHANNUMBER)	
					P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
				else
					P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
				
				P1 = McuFault64BitValuePinSet(P1,8);
				P1 = McuFault64BitValuePinClr(P1,12);       
			}
			else
				return g_HostConfigErr[12];
        }
        else if(nCurErrorType == 1) //带电阻带载 ,ecu-ubattA+
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,9); 
			P1 = McuFault64BitValuePinSet(P1,12); 
        }
        else if(nCurErrorType == 2)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			
			P1 = McuFault64BitValuePinSet(P1,8); 
			P1 = McuFault64BitValuePinSet(P1,9);
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[13];
            }
        }
        else if(nCurErrorType == 3)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,10);
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[14];
            }
        }
        else if(nCurErrorType == 4)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,10);
			P1 = McuFault64BitValuePinSet(P1,8);
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[15];
            }
        }
        else if(nCurErrorType == 5)
        {
            if (nCurPinNum < CHANNUMBER)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,10);
			P1 = McuFault64BitValuePinSet(P1,9);
			//load
            if(nCurWithLoad == 1){
				P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12);
            }                
            else  //只能是 0 或 1
            {
                P_init();
                return g_HostConfigErr[16];
            }
        }
        else if(nCurErrorType == 6)	//新版硬件协议不支持
        {
			P_init();
            return g_HostConfigErr[2];
			
//            P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
//            P1 = McuFault64BitValuePinSet(P1,nCurPinNum);
//            P2 = McuFault64BitValuePinSet(P2,nCurPinNum);

//            if(g_WorkMode == 0)
//            {
//                P3_3 = 0x0035;
//            }
//            else if(g_WorkMode == 1)
//            {
//                P3_3 = 0x00F3;
//            }
//            else
//                P3_3 = 0x0043;
        }
        else
        {
            P_init();
            return g_HostConfigErr[17];
        }
    }
    else if(nCurCommandVal == 3)	//高电压 共 18 通道
    {
        if(nCurErrorType == 0)
        {
            if (nCurPinNum < 18)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,8);
        }
        else if(nCurErrorType == 1)
        {
            if (nCurPinNum < 18)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,9);

            if(nCurWithLoad == 1){
                P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12); 
            }
            else
            {
                P_init();
                return g_HostConfigErr[17];
            }
        }
        else if(nCurErrorType == 2)
        {
            if (nCurPinNum < 18)	
				P0 = McuFault64BitValuePinSet(P0,nCurPinNum);
			else
				P1 = McuFault64BitValuePinSet(P1,nCurPinNum - 64);
			 
			P1 = McuFault64BitValuePinSet(P1,8);
			P1 = McuFault64BitValuePinSet(P1,9);

            if(nCurWithLoad == 1){
                P1 = McuFault64BitValuePinSet(P1,12); 
            }
            else if(nCurWithLoad == 0){
                P1 = McuFault64BitValuePinClr(P1,12); 
            }
            else
            {
                P_init();
                return g_HostConfigErr[18];
            }
        }
        else if(nCurErrorType == 3) //新版硬件协议不支持
        {
			P_init();
            return g_HostConfigErr[2];
			
//            if(g_Cmd3Err3Cnt == 0){
//                setbit(P3_1,nCurPinNum);
//                g_Cmd3Err3Cnt++;
//            }
//            else{
//                setbit(P3_2,nCurPinNum);
//                g_Cmd3Err3Cnt = 0;
//            }
//            
//            if(nCurWithLoad == 1){
//                P0 = clrbit(P3_0,nCurPinNum);
//            }
//            else if(nCurWithLoad == 0){
//                P0 = setbit(P3_0,nCurPinNum);
//            }
//            else
//            {
//                P_init();
//                return g_HostConfigErr[20];
//            }

//            if(g_WorkMode == 0)
//            {
//                P3_3 = 0x2000;
//            }
//            else if(g_WorkMode == 1)
//            {
//                P3_3 = 0xB000;
//            }
//            else
//                P3_3 = 0x8002;
        }
        else 
        {
            P_init();
            return g_HostConfigErr[19];
        }
    }
    else if(nCurCommandVal == 4)
    {
        if(g_DataFromHost[2])
        {   
			g_bLoose = 1;
            g_bFreq = 1;
            g_bDuty = 1;
            g_Pfreq = g_DataFromHost[3];
            g_Pduty = g_DataFromHost[4];
        }   
        else
        {
            g_Ptime = 0;
            g_Pfreq = 0;
            g_Pduty = 0;
        }
        if(g_DataFromHost[5])
        {
            P4_0 = (g_DataFromHost[6] << 8) | g_DataFromHost[7];//resistance
        }
    }
    
    return g_HostConfigPass;
}

uint8_t HostConfigCheck(uint8_t canByte0,uint8_t canByte1,uint8_t canByte2,uint8_t canByte3,uint8_t canByte4,uint8_t canByte5,uint8_t canByte6,uint8_t canByte7)
{
    unsigned short resistance = 0;
    unsigned char CurPinNum = canByte2;
    
    if(canByte1 > 16) //workmode = 0,此版本不考虑多台联动
    {
        return g_HostConfigErr[23];
    }
    
    if(canByte0 != 0x04)	//松动漏电阻
    {
        if((FIU_CurCmd == 10) || (FIU_CurCmd == canByte0))
        {
            FIU_CurCmd = canByte0;
        }
        else
            return g_HostConfigErr[24];
    }
	
    if((canByte0 == 0x00) && (canByte3 != 0))
    {
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[27];
        }
        if(canByte3 > 4)
        {
            return g_HostConfigErr[25];
        }
        if((FIU_Cmd0_CurErrorType == -1) || (FIU_Cmd0_CurErrorType == canByte3))
        {
            if(FIU_Cmd0_CurPinFlag[CurPinNum])
                return g_HostConfigErr[28];
            else
            {
                FIU_Cmd0_CurErrorType = canByte3;
                FIU_Cmd0_CurPinFlag[CurPinNum] = 1;
                return g_HostConfigPass;
            }
        }
    }
    if((canByte0 == 0x00) && (canByte3 == 0))
    {
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[29];
        }
        if(FIU_Cmd0_CurPinFlag[CurPinNum] == 1)
            return g_HostConfigErr[30];
        else
        {
            FIU_Cmd0_CurPinFlag[CurPinNum] = 1;
            return g_HostConfigPass;
        }
    }
    
    if((canByte0 == 0x01) && (canByte3 == 0))
    {
        if(canByte4 != 0)
        {
            return g_HostConfigErr[26];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[31];
        }
        if(FIU_Cmd1_Flag)
            return g_HostConfigErr[32];
        else
        {
            FIU_Cmd1_Flag = 1;
        //    FIU_Cmd4_FreqDutyFlag = 0;//can be used
            return g_HostConfigPass;
        }
    }
    if((canByte0 == 0x01) && (canByte3 != 0))
    {
        if(canByte4 > 1)
        {
            return g_HostConfigErr[50];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[31];
        }
        if(FIU_Cmd1_Flag)
            return g_HostConfigErr[32];
        else
        {
            FIU_Cmd1_Flag = 1;
         //   FIU_Cmd4_FreqDutyFlag = 0;//can be used
            return g_HostConfigPass;
        }
    }
    
    if((canByte0 == 0x02) && (canByte3 != 0) && (canByte3 != 6))
    {
        if(canByte4 > 1)
        {
            return g_HostConfigErr[54];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[33];
        }
        if(FIU_Cmd2_Flag)
            return g_HostConfigErr[34];
        else
        {
            FIU_Cmd2_Flag = 1;
        //    FIU_Cmd4_FreqDutyFlag = 0;
        //    FIU_Cmd4_ResistanceFlag = 0;
            return g_HostConfigPass;
        }
    }
    if((canByte0 == 0x02) && (canByte3 == 6))
    {
        if(canByte4 != 0)
        {
            return g_HostConfigErr[53];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[33];
        }
        if(FIU_Cmd2_Flag)
            return g_HostConfigErr[34];
        else
        {
            FIU_Cmd2_Flag = 1;
            return g_HostConfigPass;
        }
    }
    if((canByte0 == 0x02) && (canByte3 == 0))
    {
        if(canByte4 > 1)
        {
            return g_HostConfigErr[51];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[35];
        }
        if(!FIU_Cmd2_ErrorType0Flag)
        {
            FIU_Cmd2_Flag = 1;        
            FIU_Cmd2_ErrorType0Cnt++;
            if(canByte4 == 1){
          //      FIU_Cmd2_ErrorType0FlagWithLoadFlag++;
            }
            if(FIU_Cmd2_ErrorType0Cnt >= 2)
            {
                FIU_Cmd2_ErrorType0Flag = 1;
            }
//            if(FIU_Cmd2_ErrorType0FlagWithLoadFlag == 2)
//            {
//                FIU_Cmd4_FreqDutyFlag = 0;
//                FIU_Cmd4_ResistanceFlag = 0;
//            }
            return g_HostConfigPass;
        }
        else
        {
            return g_HostConfigErr[36];
        }
    }

    if((canByte0 == 0x03) && (canByte3 != 3) && (canByte3 != 0))
    {
        if(canByte4 > 1)
        {
            return g_HostConfigErr[57];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[37];
        }
        if(FIU_Cmd3_Flag)
            return g_HostConfigErr[38];
        else
        {
            FIU_Cmd3_Flag = 1;
            return g_HostConfigPass;
        }
    }
    if((canByte0 == 0x03) && (canByte3 == 0))
    {
        if(canByte4 != 0)
        {
            return g_HostConfigErr[56];
        }
        if(CurPinNum > CHANNUMBER)
        {
            return g_HostConfigErr[37];
        }
        if(FIU_Cmd3_Flag)
            return g_HostConfigErr[38];
        else
        {
            FIU_Cmd3_Flag = 1;
            return g_HostConfigPass;
        }
    }
    if((canByte0 == 0x03) && (canByte3 == 3))
    {
        if(canByte4 >1)
        {
            return g_HostConfigErr[55];
        }
        if(!FIU_Cmd3_ErrorType3Flag)
        {
            FIU_Cmd3_Flag = 1;        
            FIU_Cmd3_ErrorType3Cnt++;
            if(FIU_Cmd3_ErrorType3Cnt >= 2)
            {
                FIU_Cmd3_ErrorType3Flag = 1;
            }
            return g_HostConfigPass;
        }
        else
        {
            return g_HostConfigErr[39];
        }
    }
    
    if(canByte0 == 0x04)
    {
        if(canByte2 == 0x1)
        {
//            if(FIU_Cmd4_FreqDutyFlag)
//            {
//                return g_HostConfigErr[40];//fail
//            }   
//            else
//            {
                FIU_Cmd4_FreqDutyFlag = 1;
                if((canByte3 < 0x2) || (canByte3 > 99))//freq
                {
                    return g_HostConfigErr[40];
                }
                if(canByte3 == 2)
                {
                    if((canByte4 < 30) || (canByte4 > 50))//duty
                        return g_HostConfigErr[42];
                }
                else
                {
                    if((canByte4 < 1) || (canByte4 > 99))//duty
                    {
                        return g_HostConfigErr[43];
                    }
                }
//            }
        }
        
        if(canByte5 == 0x1)
        {
            if(FIU_Cmd4_ResistanceFlag == 0)
            {
                resistance = (canByte7 << 8) | canByte6;
                if(resistance > 32000)
                    return g_HostConfigErr[44];
                else
                    return g_HostConfigPass;
            }
            else
            {
                return g_HostConfigErr[45];
            }
        }
    }
    
    return g_HostConfigPass;
}

void P_init(void)
{
//    int i;
    
    P0 = 0, P1 = 0, P2 = 0;
    P3_0 = 0, P3_1 = 0, P3_2 = 0, P3_3 = 0, P4_0 =0, P4_1 = 0;
    g_Ptime = 0;
    g_Pfreq = 0;
    g_Pduty = 0;
    g_CRC = 0;

    g_bFreq = 0;
    g_bDuty = 0;
    g_bTime = 0;
    
    g_Cmd3Err3Cnt = 0;

//    for(i=0;i<DATATOFPGALEN;i++)
//    {
//        g_DATATOFPGA[i] = 0x0;
//    }
}

//?64??? ToSetValue ? PinNum ?? 1
uint64_t McuFault64BitValuePinSet(uint64_t ToSetValue, uint8_t PinNum)
{
    uint64_t tmp = 0;
    uint8_t tmpArray[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    uint8_t ByteIndex = 0;
    uint8_t BitIndex = 0;
    
    ByteIndex = PinNum/8;
    BitIndex = PinNum%8;
    setbit(tmpArray[ByteIndex],BitIndex);
    
    memcpy((void *)&tmp,tmpArray,8);   
    tmp = tmp | ToSetValue;
    
    return tmp;
}

//?64??? ToSetValue ? PinNum ?? 0
uint64_t McuFault64BitValuePinClr(uint64_t ToClrValue, uint8_t PinNum)
{
    uint64_t tmp = 0;
    uint8_t tmpArray[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    uint8_t ByteIndex = 0;
    uint8_t BitIndex = 0;
    
    ByteIndex = PinNum/8;
    BitIndex = PinNum%8;
    clrbit(tmpArray[ByteIndex],BitIndex);
    
    memcpy((void *)&tmp,tmpArray,8);   
    tmp = tmp & ToClrValue;
    
    return tmp;
}

void FIU_Cmd0_CurPinFlag_init(void)//??????????????????
{
    g_SendTxcmd = 0;
    
    FIU_CurCmd = 255;

    FIU_Cmd0_CurErrorType = -1;

    FIU_Cmd1_Flag = 0;

    FIU_Cmd2_Flag = 0;
    FIU_Cmd2_ErrorType0Flag = 0;
    FIU_Cmd2_ErrorType0FlagWithLoadFlag = 0;
    FIU_Cmd2_ErrorType0Cnt = 0;

    FIU_Cmd3_Flag = 0;
    FIU_Cmd3_ErrorType3Flag = 0;
    FIU_Cmd3_ErrorType3Cnt = 0;

//    FIU_Cmd4_FreqDutyFlag = 1;
//    FIU_Cmd4_ResistanceFlag = 1;
	FIU_Cmd4_FreqDutyFlag = 0;
    FIU_Cmd4_ResistanceFlag = 0;
    
    int i;
    for(i=0;i<ERRORNUM;i++)
    {
        FIU_Cmd0_CurPinFlag[i] = 0x0;
    }
    
    FIU_Cmd2_Err0 = 0;
}

void GlobalDataInit(void)
{
//    DataInit();
    P_init();
    FIU_Cmd0_CurPinFlag_init();
}

//??? 75;0??; ERRORNUM=80;
void ErrorNumInit(void)
{
    unsigned char i;
    
    g_HostConfigPass = 0;
    for(i=0;i<ERRORNUM;i++)
    {
        g_HostConfigErr[i] =  i + 0x81;
    }
}

uint8_t DataAtPxToFiuBytes17(void)
{
	g_FiuByte17Data[0] = 0xbe;
	g_FiuByte17Data[1] = 0xbe;
	
	g_FiuByte17Data[2] = FIU_CurCmd;
	
	memcpy(&g_FiuByte17Data[3],&P0,8);	//0-7
	memcpy(&g_FiuByte17Data[11],&P1,2);	//8-9
	memcpy(&g_FiuByte17Data[13],&P4_0,2);//10-11
	
	g_FiuByte17Data[15] = 0xed;
	g_FiuByte17Data[16] = 0xed;
	
	return 0;
}























