#ifndef __OUT_H
#define __OUT_H	 
#include "sys.h"


#define BEEP PBout(8)	// BEEP,蜂鸣器接口		   

#define  Open   0x01
#define  Close  0x00

#define  JA  0x01
#define  JB  0x02
#define  JC  0x03
#define  JD  0x04
#define  JE  0x05
#define  JF  0x06

#define  S1  0x01
#define  S2  0x02
#define  S3  0x03
#define  S4  0x04

void Relay_Init(void);	//初始化
u16  Get_Relay_State(void);		
void Ctrl_Relay(u8 RelayID,u8 Action);
void Ctrl_Break(u8 RelayID,u8 Action);

#endif

