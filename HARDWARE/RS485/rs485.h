#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"	 								  


#define   XLocation    -3000000
#define   YLocation    2000000
#define   ZLocation    1000000
	  		  	
extern u8 RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
extern u8 RS485_RX_CNT;   			//接收到的数据长度

extern u8 Rs485_Buff[22]; 
extern u8 Rx_Cnt;


//模式控制
//#define RS485_TX_EN		PDout(7)	//485模式控制.0,接收;1,发送.
#define RS485_TX_EN		PAout(11)
//如果想串口中断接收，请不要注释以下宏定义
#define EN_USART2_RX 	1			//0,不接收;1,接收.

extern u8  RS485_RX_BUF[64];
extern u8  RS485_RX_CNT;
extern u16 LocationDataBuff[10];
extern u8  R_W_Ctrl;
extern u8  Z_L_Ctrl;
extern int Encode_Value[4];
extern int Now_Encode_Value;
extern u8  Return_Location_Flag;

void RS485_Init(u32 bound);              //480初始化
void RS485_Send_Data(u8 *buf,u8 len);    //
void RS485_Receive_Data(u8 *buf,u8 *len);
void Write_Slave_Word(u8 HostAdd,u16 SlaveAdd,u16 Dat);
void Read_Slave_Word(u8 HostAdd,u16 SlaveAdd,u8 C_L);
void Reciver_Encode(u8 SlaveAdd);
u8   Judge_Location(u8 SlaveAdd);
void Write_Slave_Many_Word(u8 SlaveAdd,u16 RegAdd,u16 *Data,u8 Len);
unsigned int crc_chk(unsigned char* data, unsigned char length);

void Motor_Init(u8 SlaveAdd);
void Motor_Speed_Task(u8 SlaveAdd,int SpeedValue);
void Motor_Zero_Task(u8 SlaveAdd);
void Moter_Location_Task(u8 SlaveAdd,int SpeedValue,float Location);
void Cali_Zero_Task(u8 SlaveAdd);
u16  Delay_Value(int SpeedValue,float Location);
u8   Motor_Finsh_Flag(u8 SlaveAdd,float Location);
#endif	   
















