#ifndef __EXIT_H
#define __EXIT_H	 
#include "sys.h"
  	
#define R_F_Imp    GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_0)
#define F_F_Imp    GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_1)
#define L_F_Imp    GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_2)
#define L_B_Imp    GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3)
#define R_B_Imp    GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_4)

extern  u8  Remote_State;
extern  u8  Exit_Impact_State;


void EXTI_Impact_Sensor_Init(void);//外部中断初始化
void EXTI_Remote_Sensor_Init(void);

void EXTI_Impact_Main_Task(void);

#endif

