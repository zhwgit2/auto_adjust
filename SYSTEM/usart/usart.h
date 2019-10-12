#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	
#define  Zid      0x03
#define  Punchid  0x04
#define  Xid      0x01
#define  Yid      0x02
#define  ZBid     0x05

#define  Cali_Zero    0x01
#define  Compare_AD   0x02
#define  Exit_System  0x03
#define	 Debug_V      0x04
#define  Auto_Cal     0x05

#define  Cal_Mode     0x01
#define  Loc_Mode     0x02


typedef struct
{
	u8   BeginAutoCalibrationFlag;
	u8   PauseAutoCalibrationFlag;
	u8   StateCtrl;
}Calibration;

extern Calibration AutoCalibration;
//extern u8  AutoCalibrationFlag;
extern   u8  Usart_Num;
extern   u8  Uart_ReciverFlag;
extern   u8  Usart_Msg[8];	
extern   u8  SendMsg[8];
extern float XYZ_Location[6];
extern float Pressure_Value[5];
extern float Drill_Value[5];
extern float Angle_Value;
extern float Servo_Para[6];
extern  u16  PrsuAndInte[3];
extern  u16  CompareADC[3];
extern   u8  XYZ_Position_Flag[2];
extern float Calculate_XY[2];
extern float VP_Value;
extern   u8  System_Mode;
extern   u8  Debug_StateCtrl;
extern   u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern   u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义

void uart_init(u32 bound);

void Send_Com_Task(u8 com);
void Send_Dat_Task(u8 com);

void Deal_Rx_Task(u8* Msg);
void Feedback(u8* Msg);
//上位机发给下位机的激励,包括读取数据与控制信号
void Shake_Hands(u8* Msg);
void Read_ADC_Relay(u8* Msg);
void Ctrl_Punch(u8* Msg);
void Ctrl_Four_Dir(u8* Msg);
void Ctrl_Switch_State(u8* Msg);
void Ctrl_Change_State(u8* Msg);
void Send_Punch_Result(u8* Msg);
void Compare_ADC_Value(u8* Msg);
void Send_ADC_Value(void);
void Ctrl_Six_Dir(u8* Msg);
void Set_Servo_Para(u8* Msg);
void Set_VP_Value(u8* Msg);
void Send_XYZ_Position(float,float,float);

//下位机发给上位机的激励
void Get_XYZ(u8* Msg);
void Get_Pressure(u8* Msg);
void Get_Drill_Para(u8* Msg);
void Get_Result_XY(u8* Msg);
void Get_Spring_Angle(u8* Msg);
void Get_XY_State(u8* Msg);
void Get_CameraXY(u8* Msg);
void Get_Angle(u8* Msg);
void Get_PrsuAndInte(u8* Msg);
//void Mode_Auto_Calibration(u8* Msg);
//void Get_Pressure(u8* Msg);
//void Get_Drill_Para(u8* Msg);


#endif


