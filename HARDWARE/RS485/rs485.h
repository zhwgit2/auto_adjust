#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"	 								  


#define   XLocation    -3000000
#define   YLocation    2000000
#define   ZLocation    1000000
	  		  	
extern u8 RS485_RX_BUF[64]; 		//���ջ���,���64���ֽ�
extern u8 RS485_RX_CNT;   			//���յ������ݳ���

extern u8 Rs485_Buff[22]; 
extern u8 Rx_Cnt;


//ģʽ����
//#define RS485_TX_EN		PDout(7)	//485ģʽ����.0,����;1,����.
#define RS485_TX_EN		PAout(11)
//����봮���жϽ��գ��벻Ҫע�����º궨��
#define EN_USART2_RX 	1			//0,������;1,����.

extern u8  RS485_RX_BUF[64];
extern u8  RS485_RX_CNT;
extern u16 LocationDataBuff[10];
extern u8  R_W_Ctrl;
extern u8  Z_L_Ctrl;
extern int Encode_Value[4];
extern int Now_Encode_Value;
extern u8  Return_Location_Flag;

void RS485_Init(u32 bound);              //480��ʼ��
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
















