#include "sys.h"
#include "usart.h"	 
#include "OUT.h"
#include "rs485.h"
#include "ADC.h"
#include "delay.h"
#include "led.h"
#include "IN.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
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
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

	AutoCalibration.BeginAutoCalibrationFlag=0;
	AutoCalibration.PauseAutoCalibrationFlag=1;
	AutoCalibration.StateCtrl=0;
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);	//读取接收到的数据
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
			case 0x01:Shake_Hands(Msg);break;      //握手信号
			case 0x02:Read_ADC_Relay(Msg);break;   //读取ADC值
			case 0x03:Ctrl_Punch(Msg);break;       //控制钻头
			case 0x04:Ctrl_Four_Dir(Msg);break;    //控制四向移动
			case 0x05:Ctrl_Switch_State(Msg);break;//控制启停开关
			case 0x06:Ctrl_Change_State(Msg);break;//控制状态切换
			case 0x07:Send_Punch_Result(Msg);break;//打孔结果检测
			case 0x08:Compare_ADC_Value(Msg);break;//电压对比
			case 0x0b:Ctrl_Six_Dir(Msg);break;     //六向移动控制
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
			Motor_Speed_Task(Zid,-60); //快上
	}
	else if(Msg[5]==0x01)
		Motor_Speed_Task(Zid,60);//快下
	else if(Msg[6]==0x01)
		Motor_Speed_Task(Zid,10); //慢下
	else
		Motor_Speed_Task(Zid,0);  //停止
	
	if(Msg[7]==0x01)
	{
		Punch_Speed=(Msg[2]<<8)|Msg[3];
		Motor_Speed_Task(Punchid,Punch_Speed); //打孔
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
	
	XYZ_Location[3]=(Msg[2]<<8)|Msg[3];  //游丝X轴数据
	XYZ_Location[4]=(Msg[4]<<8)|Msg[5];  //游丝Y轴数据

	if(Msg[7]==0x01)
		AutoCalibration.StateCtrl=8;         //状态机跳转
	else 
		AutoCalibration.StateCtrl=10;         //状态机跳转
	
	Feedback(Mas_Data);
}
void Compare_ADC_Value(u8* Msg)
{
	CompareADC[0]=(Msg[4]<<8)|Msg[5];    //比较时长
	CompareADC[1]=(Msg[6]<<8)|Msg[7];    //频率
	CompareADC[2]=CompareADC[1]*CompareADC[0]/10; //对比数据组数
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
		case 1:{   //X轴伺服参数
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
		case 2:{    //Y轴伺服参数
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
		case 3:{    //Z轴伺服参数
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
		//printf("当前位置:%.3f   编码器值：%d\r\n",Send_Val/100000.0,Send_Val);
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
	XYZ_Location[0]=-(((Msg[2]<<8)|Msg[3]))/100.0;  //X轴数据
	XYZ_Location[1]=((Msg[4]<<8)|Msg[5])/100.0;     //Y轴数据
	XYZ_Location[2]=((Msg[6]<<8)|Msg[7])/100.0;     //Z轴数据
	
	AutoCalibration.StateCtrl=1;         //状态机跳转
} 

void Get_Pressure(u8* Msg)
{
	Drill_Value[3]=   (Msg[2]<<8)|Msg[3];  //钻头下钻的速度
	Pressure_Value[0]=(Msg[4]<<8)|Msg[5];  //限压值
	Pressure_Value[1]=(Msg[6]<<8)|Msg[7];  //压力值
	 
	AutoCalibration.StateCtrl=2;
}

void Get_Drill_Para(u8* Msg)
{
	Drill_Value[0]=(Msg[2]<<8)|Msg[3];  //起钻值
	Drill_Value[1]=(Msg[4]<<8)|Msg[5];  //止钻值
	Drill_Value[2]=(Msg[6]<<8)|Msg[7];  //转速
	
	AutoCalibration.StateCtrl=6;
}

void Get_XY_State(u8* Msg)
{
	Calculate_XY[0]=(Msg[2]<<8)|Msg[3];
	Calculate_XY[1]=(Msg[4]<<8)|Msg[5];
//	XYZ_Location[0]=(Msg[2]<<8)|Msg[3];  //X轴数据
//	XYZ_Location[1]=(Msg[4]<<8)|Msg[5];  //Y轴数据

	if(Msg[7]==0x09)
		AutoCalibration.StateCtrl=5;         //状态机跳转
}

void Get_CameraXY(u8* Msg)
{
	XYZ_Location[0]=(Msg[2]<<8)|Msg[3];  //X轴数据
	XYZ_Location[1]=(Msg[4]<<8)|Msg[5];  //Y轴数据
	
	AutoCalibration.StateCtrl=7;         //状态机跳转
}

void Get_Angle(u8* Msg)
{
	Angle_Value=(Msg[2]<<8)|Msg[3];  //获取游丝角度
	
	AutoCalibration.StateCtrl=9;         //状态机跳转
}

void Get_PrsuAndInte(u8* Msg)
{
	PrsuAndInte[0]=(Msg[2]<<8)|Msg[3];   //最高压力值
	PrsuAndInte[1]=(Msg[4]<<8)|Msg[5];   //区间左值
	PrsuAndInte[2]=(Msg[6]<<8)|Msg[7];   //区间右值
	Debug_StateCtrl=1;
}

#endif	

