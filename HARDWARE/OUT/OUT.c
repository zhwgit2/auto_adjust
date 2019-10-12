#include "OUT.h"


void Relay_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOG, ENABLE);	 //使能GPIOB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
																GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	//BEEP-->PB.8 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
	GPIO_Init(GPIOG, &GPIO_InitStructure);	 //根据参
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

u16  Get_Relay_State(void)
{
	u16 Relay_state=0x0000;
//	u16 temp;
//	temp=GPIOD->CRL;
//	GPIOD->CRL=0x4444;   //input mode
//	
//	temp=GPIO_ReadInputData(GPIOD);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_13)<<0);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_12)<<1);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_7) <<2);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6) <<3);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_5) <<4);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_4) <<5);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3) <<6);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2) <<7);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_1) <<8);
	Relay_state|=(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0) <<9);
	
	return Relay_state;
}
//RelayID:1~6
//Action: 0关闭继电器；1打开继电器
//close    open  
void Ctrl_Relay(u8 RelayID,u8 Action)
{
	if(Action==1)
	{
		switch(RelayID)
		{
			case 0x01:GPIO_SetBits(GPIOG,GPIO_Pin_13);break;
			case 0x02:GPIO_SetBits(GPIOG,GPIO_Pin_12);break;
			case 0x03:GPIO_SetBits(GPIOD,GPIO_Pin_7 );break;
			case 0x04:GPIO_SetBits(GPIOD,GPIO_Pin_6 );break;
			case 0x05:GPIO_SetBits(GPIOD,GPIO_Pin_5 );break;
			case 0x06:GPIO_SetBits(GPIOD,GPIO_Pin_4 );break;
		}
	}
	else
	{
		switch(RelayID)
		{
			case 0x01:GPIO_ResetBits(GPIOG,GPIO_Pin_13);break;
			case 0x02:GPIO_ResetBits(GPIOG,GPIO_Pin_12);break;
			case 0x03:GPIO_ResetBits(GPIOD,GPIO_Pin_7 );break;
			case 0x04:GPIO_ResetBits(GPIOD,GPIO_Pin_6 );break;
			case 0x05:GPIO_ResetBits(GPIOD,GPIO_Pin_5 );break;
			case 0x06:GPIO_ResetBits(GPIOD,GPIO_Pin_4 );break;
		}
	}
}
void Ctrl_Break(u8 RelayID,u8 Action)
{
	if(Action==1)
	{
		switch(RelayID)
		{
			case 0x01:GPIO_SetBits(GPIOE,GPIO_Pin_2);break;
			case 0x02:GPIO_SetBits(GPIOE,GPIO_Pin_3);break;
			case 0x03:GPIO_SetBits(GPIOE,GPIO_Pin_4);break;
			case 0x04:GPIO_SetBits(GPIOE,GPIO_Pin_5);break;
		}
	}
	else
	{
		switch(RelayID)
		{
			case 0x01:GPIO_ResetBits(GPIOE,GPIO_Pin_2);break;
			case 0x02:GPIO_ResetBits(GPIOE,GPIO_Pin_3);break;
			case 0x03:GPIO_ResetBits(GPIOE,GPIO_Pin_4);break;
			case 0x04:GPIO_ResetBits(GPIOE,GPIO_Pin_5);break;
		}
	}	
}



