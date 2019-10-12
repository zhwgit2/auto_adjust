#include "sys.h"
#include "usart.h"	 
#include "OUT.h"
#include "rs485.h"
#include "ADC.h"
#include "delay.h"
#include "led.h"
#include "IN.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

u8  Usart_Num=0;
u8  Usart_Msg[8];
u8  SendMsg[8];
u8  Uart_ReciverFlag=0;
float XYZ_Location[6];
float Pressure_Value[5];
float Drill_Value[5];
float Angle_Value;
float Servo_Para[6]={1000,0,1000,0,1000,0};
float VP_Value=1;
float Calculate_XY[2]={0};
u16   PrsuAndInte[3];
u8    System_Mode;
u8    Debug_StateCtrl;
u16   CompareADC[3];
u8    XYZ_Position_Flag[2];
Calibration AutoCalibration;
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

	AutoCalibration.BeginAutoCalibrationFlag=0;
	AutoCalibration.PauseAutoCalibrationFlag=1;
	AutoCalibration.StateCtrl=0;
}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		Usart_Msg[Usart_Num++]=Res;
		
		if((Usart_Msg[0]==0x00 || Usart_Msg[0]==0x50) && (Usart_Msg[1]<0x10))
		{
			if(Usart_Num==8)
			{
				LED=!LED;
				Uart_ReciverFlag=1;
				//Deal_Rx_Task(Usart_Msg);
				Usart_Num=0;				
			}
		}		
		else
			Usart_Num=0;

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 
} 

void Send_Com_Task(u8 com)
{
	SendMsg[0]=0x05;
	SendMsg[1]=com;
	SendMsg[2]=0x00;
	SendMsg[3]=0x00;
	SendMsg[4]=0x00;
	SendMsg[5]=0x00;
	SendMsg[6]=0x00;
	SendMsg[7]=0x00;
	
	Feedback(SendMsg);
}

void Send_Dat_Task(u8 Dat)
{
	SendMsg[0]=0x05;
	SendMsg[1]=0x04;
	SendMsg[2]=0x00;
	SendMsg[3]=0x00;
	SendMsg[4]=0x00;
	SendMsg[5]=0x00;
	SendMsg[6]=0x00;
	SendMsg[7]=Dat;
	
	Feedback(SendMsg);
}

void Deal_Rx_Task(u8* Msg)
{
	if(Msg[0]==0x00)
	{
		switch(Msg[1])
		{
			case 0x01:Shake_Hands(Msg);break;      //�����ź�
			case 0x02:Read_ADC_Relay(Msg);break;   //��ȡADCֵ
			case 0x03:Ctrl_Punch(Msg);break;       //������ͷ
			case 0x04:Ctrl_Four_Dir(Msg);break;    //���������ƶ�
			case 0x05:Ctrl_Switch_State(Msg);break;//������ͣ����
			case 0x06:Ctrl_Change_State(Msg);break;//����״̬�л�
			case 0x07:Send_Punch_Result(Msg);break;//��׽�����
			case 0x08:Compare_ADC_Value(Msg);break;//��ѹ�Ա�
			case 0x0b:Ctrl_Six_Dir(Msg);break;     //�����ƶ�����
			case 0x0c:Set_Servo_Para(Msg);break;
			case 0x0d:Set_VP_Value(Msg);break;
//			case 0x09:Get_Drill_Para(Msg);break;			
		}
	}
	else if(Msg[0]==0x50)
	{
		switch(Msg[1])
		{
			case 0x01:Get_XYZ(Msg);break;        
			case 0x02:Get_Pressure(Msg);break;   
			case 0x03:Get_Drill_Para(Msg);break;
			case 0x04:                   ;break;
			case 0x05:Get_XY_State(Msg);break;
			case 0x06:Get_CameraXY(Msg);break;
			case 0x07:Get_Angle(Msg);break;
			case 0x08:Get_PrsuAndInte(Msg);break;
//			case 0x08:Get_Pressure(Msg);break;
//			case 0x09:Get_Drill_Para(Msg);break;
			
		}
	}
}

void Feedback(u8* Msg)
{
	u8 i=0;
	for(i=0;i<8;i++)
	{
		USART_SendData(USART1,Msg[i]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
	}
}

void Shake_Hands(u8* Msg)
{
//	u8 Mas_Data[8]={0,0,0,0,0,0,0,0};
//	
//	Mas_Data[0]=0x55;
//	Mas_Data[1]=0x01;
//	Mas_Data[2]=0x00;
//	Mas_Data[3]=0x00;
//	Mas_Data[4]=0x00;
//	Mas_Data[5]=0x00;
//	Mas_Data[6]=0x01;
//	Mas_Data[7]=Msg[7];
	
	System_Mode=Msg[7];
	
//	Feedback(Mas_Data);
}
void Read_ADC_Relay(u8* Msg)
{
	u8  Mas_Data[8]={0,0,0,0,0,0,0,0};
	u16 temp;
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x02;
	Mas_Data[2]=0x00;
	
	Ctrl_Relay(JA,(Msg[3]&0x01)>>0);
	Ctrl_Relay(JB,(Msg[3]&0x02)>>1);
	Ctrl_Relay(JC,(Msg[3]&0x04)>>2);
	Ctrl_Relay(JD,(Msg[3]&0x08)>>3);
	Ctrl_Relay(JE,(Msg[3]&0x10)>>4);
	Ctrl_Relay(JF,(Msg[3]&0x20)>>5);
	
	temp=Get_Relay_State();
	Mas_Data[3]=((u8)(temp))&0x3f;
	
	temp=Adc_Cal_Val(0);
	Mas_Data[4]=(u8)(temp>>8);
	Mas_Data[5]=(u8)(temp>>0);
	
	temp=Adc_Cal_Val(1);
	Mas_Data[6]=(u8)(temp>>8);
	Mas_Data[7]=(u8)(temp>>0);
	
	Feedback(Mas_Data);
}

void Ctrl_Punch(u8* Msg)
{
	//u8 Mas_Data[8]={0,0,0,0,0,0,0,0};
	u16 Punch_Speed=0;
	
//	Mas_Data[0]=0x55;
//	Mas_Data[1]=0x03;
//	Mas_Data[2]=0x00;
//	Mas_Data[3]=0x00;
//	Mas_Data[4]=Msg[4];
//	Mas_Data[5]=Msg[5];
//	Mas_Data[6]=Msg[6];
//	Mas_Data[7]=Msg[7];
	
	if(Msg[4]==0x01)
	{
		if(G2!=1)
			Motor_Speed_Task(Zid,-60); //����
	}
	else if(Msg[5]==0x01)
		Motor_Speed_Task(Zid,60);//����
	else if(Msg[6]==0x01)
		Motor_Speed_Task(Zid,10); //����
	else
		Motor_Speed_Task(Zid,0);  //ֹͣ
	
	if(Msg[7]==0x01)
	{
		Punch_Speed=(Msg[2]<<8)|Msg[3];
		Motor_Speed_Task(Punchid,Punch_Speed); //���
	}
	else
		Motor_Speed_Task(Punchid,0);
	
	//Feedback(Mas_Data);
}
u8  Return_Location_Flag=0;
void Ctrl_Four_Dir(u8* Msg)
{
	if(Msg[4]==0x01)
		Motor_Speed_Task(Yid,100);
	else if(Msg[5]==0x01)
	{
		if(G3!=1)
			Motor_Speed_Task(Yid,-100);
	}
	else
	{
		Motor_Speed_Task(Yid,0);
		//Return_Location_Flag=1;
	}
		
	if(Msg[6]==0x01)
	{
		if(G4!=1)
			Motor_Speed_Task(Xid,100);
	}
	else if(Msg[7]==0x01)	
		Motor_Speed_Task(Xid,-100);
	else
	{		
		Motor_Speed_Task(Xid,0);
		//Return_Location_Flag=1;
	}
	
	if(!(Msg[4]|Msg[5]|Msg[6]|Msg[7]))
		Return_Location_Flag=1;
}

void Ctrl_Switch_State(u8* Msg)
{
	u8 Mas_Data[8]={0,0,0,0,0,0,0,0};
	
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x05;
	Mas_Data[2]=0x00;
	Mas_Data[3]=0x00;
	Mas_Data[4]=0x00;
	Mas_Data[5]=Msg[5];
	Mas_Data[6]=Msg[6];
	Mas_Data[7]=Msg[7];
	
	if(Msg[5]==0x01)
	{
		AutoCalibration.BeginAutoCalibrationFlag=1;
		AutoCalibration.PauseAutoCalibrationFlag=1;
	}
	else if(Msg[6]==0x01)
		AutoCalibration.PauseAutoCalibrationFlag=!AutoCalibration.PauseAutoCalibrationFlag;
	else if(Msg[7]==0x01)
	{
		AutoCalibration.BeginAutoCalibrationFlag=0;
		AutoCalibration.PauseAutoCalibrationFlag=0;
		AutoCalibration.StateCtrl=0;
	}
	else ;
	
	Feedback(Mas_Data);
}

void Ctrl_Change_State(u8* Msg)
{
	u8 Mas_Data[8]={0,0,0,0,0,0,0,0};
	
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x06;
	Mas_Data[2]=0x00;
	Mas_Data[3]=0x00;
	Mas_Data[4]=0x00;
	Mas_Data[5]=0x00;
	Mas_Data[6]=0x00;
	Mas_Data[7]=0x00;
	
	if(Msg[7]==0x06)
		AutoCalibration.StateCtrl=3;
	else if(Msg[7]==0x08)
		AutoCalibration.StateCtrl=4;
	
	Feedback(Mas_Data);
}
void Send_Punch_Result(u8* Msg)
{
	u8 Mas_Data[8]={0,0,0,0,0,0,0,0};
	
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x06;
	Mas_Data[2]=0x00;
	Mas_Data[3]=0x00;
	Mas_Data[4]=0x00;
	Mas_Data[5]=0x00;
	Mas_Data[6]=0x00;
	Mas_Data[7]=Msg[7];
	
	XYZ_Location[3]=(Msg[2]<<8)|Msg[3];  //��˿X������
	XYZ_Location[4]=(Msg[4]<<8)|Msg[5];  //��˿Y������

	if(Msg[7]==0x01)
		AutoCalibration.StateCtrl=8;         //״̬����ת
	else 
		AutoCalibration.StateCtrl=10;         //״̬����ת
	
	Feedback(Mas_Data);
}
void Compare_ADC_Value(u8* Msg)
{
	CompareADC[0]=(Msg[4]<<8)|Msg[5];    //�Ƚ�ʱ��
	CompareADC[1]=(Msg[6]<<8)|Msg[7];    //Ƶ��
	CompareADC[2]=CompareADC[1]*CompareADC[0]/10; //�Ա���������
	System_Mode=Compare_AD;
	
	TIM3->ARR=(u16)(10000.0/CompareADC[1]);
	TIM3->PSC=7199;
	TIM_Cmd(TIM3, ENABLE);
	
}
void Send_ADC_Value(void)
{
	u8  Mas_Data[8]={0,0,0,0,0,0,0,0};
	u16 temp;
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x0a;
	Mas_Data[2]=0x00;
	Mas_Data[3]=0x00;
	
	temp=Adc_Cal_Val(0);
	Mas_Data[4]=(u8)(temp>>8);
	Mas_Data[5]=(u8)(temp>>0);
	
	temp=Adc_Cal_Val(1);
	Mas_Data[6]=(u8)(temp>>8);
	Mas_Data[7]=(u8)(temp>>0);
	
	Feedback(Mas_Data);
}
void Ctrl_Six_Dir(u8* Msg)
{
	if(Msg[2]==0x01)
	{
		Motor_Speed_Task(Zid,30);
		XYZ_Position_Flag[0]=3;
	}
	else if(Msg[3]==0x01)
	{
		XYZ_Position_Flag[0]=3;
		if(G2!=1)
			Motor_Speed_Task(Zid,-30);
	}
	else
	{
		Motor_Speed_Task(Zid,0);
	}
	
	if(Msg[4]==0x01)
	{
		Motor_Speed_Task(Yid,100);
		XYZ_Position_Flag[0]=2;
	}
	else if(Msg[5]==0x01)
	{
		XYZ_Position_Flag[0]=2;
		if(G3!=1)
			Motor_Speed_Task(Yid,-100);
	}
	else
	{
		Motor_Speed_Task(Yid,0);
	}
		
	if(Msg[6]==0x01)
	{
		XYZ_Position_Flag[0]=1;
		if(G4!=1)
			Motor_Speed_Task(Xid,100);
	}
	else if(Msg[7]==0x01)	
	{
		Motor_Speed_Task(Xid,-100);
		XYZ_Position_Flag[0]=1;
	}
	else
	{		
		Motor_Speed_Task(Xid,0);
	}
	
	if(!(Msg[2]|Msg[3]|Msg[4]|Msg[5]|Msg[6]|Msg[7]))
		XYZ_Position_Flag[1]=1;
}
void Set_Servo_Para(u8* Msg)
{
	switch(Msg[2])
	{
		case 1:{   //X���ŷ�����
							Servo_Para[0]=(Msg[4]<<8)|Msg[5];
							Servo_Para[1]=(Msg[6]<<8)|Msg[7];
							if(Msg[3]==0x01)
							{
								Servo_Para[0]=Servo_Para[0];
								Servo_Para[1]=-Servo_Para[1];
							}
							else if(Msg[3]==0x02)
							{
								Servo_Para[0]=-Servo_Para[0];
								Servo_Para[1]=Servo_Para[1];
							}
							else if(Msg[3]==0x03)
							{
								Servo_Para[0]=-Servo_Para[0];
								Servo_Para[1]=-Servo_Para[1];
							}
						};break;		
		case 2:{    //Y���ŷ�����
							Servo_Para[2]=(Msg[4]<<8)|Msg[5];
							Servo_Para[3]=(Msg[6]<<8)|Msg[7];
							if(Msg[3]==0x01)
							{
								Servo_Para[2]=Servo_Para[2];
								Servo_Para[3]=-Servo_Para[3];
							}
							else if(Msg[3]==0x02)
							{
								Servo_Para[2]=-Servo_Para[2];
								Servo_Para[3]=Servo_Para[3];
							}
							else if(Msg[3]==0x03)
							{
								Servo_Para[2]=-Servo_Para[2];
								Servo_Para[3]=-Servo_Para[3];
							}
						};break;
		case 3:{    //Z���ŷ�����
							Servo_Para[4]=(Msg[4]<<8)|Msg[5];
							Servo_Para[5]=(Msg[6]<<8)|Msg[7];
							if(Msg[3]==0x01)
							{
								Servo_Para[4]=Servo_Para[4];
								Servo_Para[5]=-Servo_Para[5];
							}
							else if(Msg[3]==0x02)
							{
								Servo_Para[4]=-Servo_Para[4];
								Servo_Para[5]=Servo_Para[5];
							}
							else if(Msg[3]==0x03)
							{
								Servo_Para[4]=-Servo_Para[4];
								Servo_Para[5]=-Servo_Para[5];
							}		
						};break;
	}	
}
void Set_VP_Value(u8* Msg)
{
	VP_Value=((Msg[6]<<8)|Msg[7])/10.0;
}
void Send_XYZ_Position(float x,float y,float z)
{
	u8  Mas_Data[8]={0,0,0,0,0,0,0,0};
	int Send_Val=0;
	
	Mas_Data[0]=0x55;
	Mas_Data[1]=0x09;
	
	if(x!=0)   
	{
		Send_Val=100*x;
		Mas_Data[2]=(u8)((int)(Send_Val/100.0)>>8); 
		Mas_Data[3]=(u8)((int)(Send_Val/100.0)>>0); 
	}
	else
	{
		Read_Slave_Word(Xid,0x0520,Loc_Mode);
		delay_ms(50);
		Send_Val=-(Now_Encode_Value-Encode_Value[0]);
		if(Send_Val<0)
			Send_Val=0;
		Mas_Data[2]=(u8)((int)(Send_Val/100.0)>>8); 
		Mas_Data[3]=(u8)((int)(Send_Val/100.0)>>0); 
		//printf("��ǰλ��:%.3f   ������ֵ��%d\r\n",Send_Val/100000.0,Send_Val);
	}
	
	if(y!=0)
	{
		Send_Val=100*y;
		Mas_Data[2]=(u8)((int)(Send_Val/100.0)>>8); 
		Mas_Data[3]=(u8)((int)(Send_Val/100.0)>>0); 
	}
	else
	{
		Read_Slave_Word(Yid,0x0520,Loc_Mode);
		delay_ms(50);
		Send_Val=(Now_Encode_Value-Encode_Value[1]);
		if(Send_Val<0)
			Send_Val=0;
		Mas_Data[4]=(u8)((int)(Send_Val/100.0)>>8);  
		Mas_Data[5]=(u8)((int)(Send_Val/100.0)>>0);  
	}
	 
	if(z!=0)
	{
		Send_Val=100*z;
		Mas_Data[2]=(u8)((int)(Send_Val/100.0)>>8); 
		Mas_Data[3]=(u8)((int)(Send_Val/100.0)>>0); 
	}
	else
	{
		Read_Slave_Word(Zid,0x0520,Loc_Mode);
		delay_ms(50);
		Send_Val=(Now_Encode_Value-Encode_Value[2]);
		if(Send_Val<0)
			Send_Val=0;
		Mas_Data[6]=(u8)((int)(Send_Val/100.0)>>8);
		Mas_Data[7]=(u8)((int)(Send_Val/100.0)>>0);
	}
		
	Feedback(Mas_Data);
}

void Get_XYZ(u8* Msg)
{
	XYZ_Location[0]=-(((Msg[2]<<8)|Msg[3]))/100.0;  //X������
	XYZ_Location[1]=((Msg[4]<<8)|Msg[5])/100.0;     //Y������
	XYZ_Location[2]=((Msg[6]<<8)|Msg[7])/100.0;     //Z������
	
	AutoCalibration.StateCtrl=1;         //״̬����ת
} 

void Get_Pressure(u8* Msg)
{
	Drill_Value[3]=   (Msg[2]<<8)|Msg[3];  //��ͷ������ٶ�
	Pressure_Value[0]=(Msg[4]<<8)|Msg[5];  //��ѹֵ
	Pressure_Value[1]=(Msg[6]<<8)|Msg[7];  //ѹ��ֵ
	 
	AutoCalibration.StateCtrl=2;
}

void Get_Drill_Para(u8* Msg)
{
	Drill_Value[0]=(Msg[2]<<8)|Msg[3];  //����ֵ
	Drill_Value[1]=(Msg[4]<<8)|Msg[5];  //ֹ��ֵ
	Drill_Value[2]=(Msg[6]<<8)|Msg[7];  //ת��
	
	AutoCalibration.StateCtrl=6;
}

void Get_XY_State(u8* Msg)
{
	Calculate_XY[0]=(Msg[2]<<8)|Msg[3];
	Calculate_XY[1]=(Msg[4]<<8)|Msg[5];
//	XYZ_Location[0]=(Msg[2]<<8)|Msg[3];  //X������
//	XYZ_Location[1]=(Msg[4]<<8)|Msg[5];  //Y������

	if(Msg[7]==0x09)
		AutoCalibration.StateCtrl=5;         //״̬����ת
}

void Get_CameraXY(u8* Msg)
{
	XYZ_Location[0]=(Msg[2]<<8)|Msg[3];  //X������
	XYZ_Location[1]=(Msg[4]<<8)|Msg[5];  //Y������
	
	AutoCalibration.StateCtrl=7;         //״̬����ת
}

void Get_Angle(u8* Msg)
{
	Angle_Value=(Msg[2]<<8)|Msg[3];  //��ȡ��˿�Ƕ�
	
	AutoCalibration.StateCtrl=9;         //״̬����ת
}

void Get_PrsuAndInte(u8* Msg)
{
	PrsuAndInte[0]=(Msg[2]<<8)|Msg[3];   //���ѹ��ֵ
	PrsuAndInte[1]=(Msg[4]<<8)|Msg[5];   //������ֵ
	PrsuAndInte[2]=(Msg[6]<<8)|Msg[7];   //������ֵ
	Debug_StateCtrl=1;
}

#endif	

