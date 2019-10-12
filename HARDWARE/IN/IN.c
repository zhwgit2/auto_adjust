#include "IN.h"
#include "led.h"
#include "rs485.h"
#include "usart.h"
#include "sys.h"
#include "delay.h"
#include "OUT.h"

u8  Calibration_Dir=0;
u8  Cali_Complete=0;
u8  Calibration_Add=0;

void Input_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��GPIOB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 |GPIO_Pin_9 |GPIO_Pin_10|GPIO_Pin_11|
																GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	//BEEP-->PB.8 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOB.8

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource8);
	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource9);
	EXTI_InitStructure.EXTI_Line=EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource10);
	EXTI_InitStructure.EXTI_Line=EXTI_Line10;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line=EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource12);
	EXTI_InitStructure.EXTI_Line=EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource13);
	EXTI_InitStructure.EXTI_Line=EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource14);
	EXTI_InitStructure.EXTI_Line=EXTI_Line14;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource15);
	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 
}

void EXTI9_5_IRQHandler(void)
{	
	if(G4==1)    //��ӦX��
	{
		Motor_Speed_Task(Xid,0);
		if(System_Mode==Cali_Zero)
			Calibration_Add=Xid;
	}
	EXTI_ClearITPendingBit(EXTI_Line8);
	
	if(G3==1)   //��ӦY��
	{
		Motor_Speed_Task(Yid,0);
		if(System_Mode==Cali_Zero)
			Calibration_Add=Yid;
	}
	EXTI_ClearITPendingBit(EXTI_Line9);
}

void EXTI15_10_IRQHandler(void)
{
	if(G2==1)   //��ӦZ��
	{
		Motor_Speed_Task(Zid,0);
		if(System_Mode==Cali_Zero)
			Calibration_Add=Zid;
	}
	EXTI_ClearITPendingBit(EXTI_Line10);
	
	if(G1==1)
	{
		
	}
	EXTI_ClearITPendingBit(EXTI_Line11);
	
	if(K1==0)
	{
		if(System_Mode==Auto_Cal)
		{
			AutoCalibration.BeginAutoCalibrationFlag=1;
			AutoCalibration.PauseAutoCalibrationFlag=1;
		}
		else if(System_Mode==Debug_V)
		{
			SendMsg[0]=0x05;
			SendMsg[1]=0x08;
			SendMsg[2]=0x00;
			SendMsg[3]=0x00;
			SendMsg[4]=0x00;
			SendMsg[5]=0x00;
			SendMsg[6]=0x00;
			SendMsg[7]=0x01;
	
			Feedback(SendMsg);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line12);
	
	if(K2==0)
	{
		if(System_Mode==Auto_Cal)
		{
			AutoCalibration.PauseAutoCalibrationFlag=!AutoCalibration.PauseAutoCalibrationFlag;
		}
		else if(System_Mode==Debug_V)
		{
			Ctrl_Relay(JA,Open);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line13);
	
	if(K3==0)
	{
		if(System_Mode==Auto_Cal)   //�Զ���Уģʽ��
		{
			Motor_Speed_Task(Xid,0);  //����ֹͣ����
			Motor_Speed_Task(Yid,0);
			Motor_Speed_Task(Zid,0);
			Motor_Speed_Task(Punchid,0);
			Motor_Speed_Task(ZBid,0);
			
			AutoCalibration.BeginAutoCalibrationFlag=0; //״̬��־λ�ָ���ֵ
			AutoCalibration.PauseAutoCalibrationFlag=1;
			AutoCalibration.StateCtrl=0;
			
			Moter_Location_Task(Zid,300,0);   //�������ι�λ�����
			delay_ms(1000);
			delay_ms(1000);
			Moter_Location_Task(Xid,300,0);
			Moter_Location_Task(Yid,300,0);
			delay_ms(1000);
			delay_ms(1000);	
			Moter_Location_Task(ZBid,300,0);
			LED=1;
			delay_ms(500);	
		}
		else if(System_Mode==Debug_V)
		{
			Ctrl_Relay(JD,Open);
		}
		Motor_Speed_Task(Xid,0);
		Motor_Speed_Task(Yid,0);
		Motor_Speed_Task(Zid,0);
		Motor_Speed_Task(Punchid,0);
		Motor_Speed_Task(ZBid,0);
		Usart_Num=0;	
	}
	EXTI_ClearITPendingBit(EXTI_Line14);
	
	if(K4==0)
	{
		if(System_Mode==Debug_V)
		{
			Ctrl_Relay(JA,Close);
			Ctrl_Relay(JB,Close);
			Ctrl_Relay(JC,Close);
			Ctrl_Relay(JD,Close);
			Ctrl_Relay(JE,Close);
			Ctrl_Relay(JF,Close);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line15);
}

