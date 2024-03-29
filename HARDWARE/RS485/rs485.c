#include "sys.h"		    
#include "rs485.h"	 
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "IN.h"
#include "OUT.h"
#include "math.h"

#ifdef EN_USART2_RX   	//如果使能了接收


//接收缓存区 	
//u8 RS485_RX_BUF[64];  	//接收缓冲,最大64个字节.
////接收到的数据长度
//u8 RS485_RX_CNT=0;

u8  Rs485_Buff[22]; 
u8  Rx_Cnt=0;
u8  R_W_Ctrl=0;
u8  Z_L_Ctrl=0;
int Encode_Value[4]={0};
int Now_Encode_Value=0;

void USART2_IRQHandler(void)
{
	u8 res;	    
 
 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收到数据
	{	
		res =USART_ReceiveData(USART2); 	//读取接收到的数据
		Rs485_Buff[Rx_Cnt++]=res;
		
		if(R_W_Ctrl==1)   //从驱动器读数据
		{
			if(Rx_Cnt==9)   //接收数据完成
			{
				Rx_Cnt=0;     //清零，准备下一次接收			
				if(Rs485_Buff[1]==0x03)  //确保是读取数据指令
				{
					
					if(Z_L_Ctrl==Cal_Mode)        //标定零点
						Reciver_Encode(Rs485_Buff[0]);
					else if(Z_L_Ctrl==Loc_Mode)   //获取当前位置
						Now_Encode_Value=(Rs485_Buff[3]<<8)|(Rs485_Buff[4]<<0)|(Rs485_Buff[5]<<24)|(Rs485_Buff[6]<<16);						
				}
				R_W_Ctrl=0;  //读取数据指令完成后变为写数据
			}
		}
		else     //写数据的响应不处理
			Rx_Cnt=0;
	}
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);	
} 
#endif	

//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void RS485_Init(u32 bound)
{  
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD, ENABLE);//使能GPIOA,D时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 //PD7端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位
 
	
 #ifdef EN_USART2_RX		  	//如果使能了接收
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式

  USART_Init(USART2, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
   
  USART_Cmd(USART2, ENABLE);                    //使能串口 

 #endif

  RS485_TX_EN=0;			//默认为接收模式
 
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
//void RS485_Send_Data(u8 *buf,u8 len)
//{
//	u8 t;
//	RS485_TX_EN=1;			//设置为发送模式
//  	for(t=0;t<len;t++)		//循环发送数据
//	{		   
//		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
//		USART_SendData(USART2,buf[t]);
//	}	 
// 
//	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
//	RS485_RX_CNT=0;	  
//	RS485_TX_EN=0;				//设置为接收模式	
//}
//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
//void RS485_Receive_Data(u8 *buf,u8 *len)
//{
//	u8 rxlen=RS485_RX_CNT;
//	u8 i=0;
//	*len=0;				//默认为0
//	delay_ms(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
//	if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
//	{
//		for(i=0;i<rxlen;i++)
//		{
//			buf[i]=RS485_RX_BUF[i];	
//		}		
//		*len=RS485_RX_CNT;	//记录本次数据长度
//		RS485_RX_CNT=0;		//清零
//	}
//}
u8 Word_Buff[8];
void Write_Slave_Word(u8 SlaveAdd,u16 RegAdd,u16 Dat)
{
	unsigned int temp;
	u8 i;
	Word_Buff[0]=SlaveAdd;
	Word_Buff[1]=0x06;
	Word_Buff[2]=(u8)(RegAdd>>8);
	Word_Buff[3]=(u8)(RegAdd);
	Word_Buff[4]=(u8)(Dat>>8);
	Word_Buff[5]=(u8)(Dat);
	temp=crc_chk(Word_Buff,6);
	Word_Buff[6]=(u8)(temp);
	Word_Buff[7]=(u8)(temp>>8);
	
	RS485_TX_EN=1;
	delay_ms(10);
	for(i=0;i<8;i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,Word_Buff[i]);
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	
	RS485_TX_EN=0;
	delay_ms(10);
}

void Read_Slave_Word(u8 SlaveAdd,u16 RegAdd,u8 C_L)
{
	unsigned int temp;
	u8 i;
	  
	Z_L_Ctrl=C_L;     //判断是标定零点还是读位置
	Rx_Cnt=0;
	R_W_Ctrl=1;
	
	Word_Buff[0]=SlaveAdd;
	Word_Buff[1]=0x03;
	Word_Buff[2]=(u8)(RegAdd>>8);
	Word_Buff[3]=(u8)(RegAdd);
	Word_Buff[4]=0x00;
	Word_Buff[5]=0x02;
	temp=crc_chk(Word_Buff,6);
	Word_Buff[6]=(u8)(temp);
	Word_Buff[7]=(u8)(temp>>8);
	
	RS485_TX_EN=1;
	delay_ms(10);
	for(i=0;i<8;i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,Word_Buff[i]);
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	
	RS485_TX_EN=0;
	delay_ms(10);
}
void Reciver_Encode(u8 SlaveAdd)
{
	switch(SlaveAdd)
	{
		case Xid :Encode_Value[0]=(Rs485_Buff[3]<<8)|(Rs485_Buff[4]<<0)|(Rs485_Buff[5]<<24)|(Rs485_Buff[6]<<16);break;
		case Yid :Encode_Value[1]=(Rs485_Buff[3]<<8)|(Rs485_Buff[4]<<0)|(Rs485_Buff[5]<<24)|(Rs485_Buff[6]<<16);break;
		case Zid :Encode_Value[2]=(Rs485_Buff[3]<<8)|(Rs485_Buff[4]<<0)|(Rs485_Buff[5]<<24)|(Rs485_Buff[6]<<16);break;
		//case ZBid:Encode_Value[3]=(Rs485_Buff[3]<<8)|(Rs485_Buff[4]<<0)|(Rs485_Buff[5]<<24)|(Rs485_Buff[6]<<16);break;
	}
}

u8   Judge_Location(u8 SlaveAdd)
{
	u8 fed=0;
	delay_ms(100);
	Read_Slave_Word(SlaveAdd,0x005c,Cal_Mode);
	
	if(Rs485_Buff[1]==0x03 && Z_L_Ctrl==0)
	{
		fed=(Rs485_Buff[4]>>4)&0x01;
	}
	return fed;
}

u16 LocationDataBuff[10];
u8 Many_Word_Buff[20];
void Write_Slave_Many_Word(u8 SlaveAdd,u16 RegAdd,u16 *Data,u8 Len)
{
	unsigned int temp;
	u8 i,num;
	Many_Word_Buff[0]=SlaveAdd;
	Many_Word_Buff[1]=0x10;
	Many_Word_Buff[2]=(u8)(RegAdd>>8);
	Many_Word_Buff[3]=(u8)(RegAdd);
	Many_Word_Buff[4]=0x00;
	Many_Word_Buff[5]=Len;
	Many_Word_Buff[6]=Len*2;
	for(i=0;i<Len;i++)
	{
		Many_Word_Buff[2*i+7]  =(u8)(Data[i]>>8);
		Many_Word_Buff[2*i+8]  =(u8)(Data[i]);
	}
	temp=crc_chk(Many_Word_Buff,7+2*i);
	Many_Word_Buff[7+2*i]=(u8)(temp);
	Many_Word_Buff[8+2*i]=(u8)(temp>>8);
	num=9+2*i;
	RS485_TX_EN=1;
	delay_ms(10);
	for(i=0;i<num;i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,Many_Word_Buff[i]);
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	
	RS485_TX_EN=0;
	delay_ms(10);
}


unsigned int crc_chk(unsigned char* data, unsigned char length) 
{
	int j;
	unsigned int reg_crc=0xFFFF;
	while( length-- ) 
	{
		reg_crc^= *data++;
		for (j=0; j<8; j++ )
		{
			if( reg_crc & 0x01 )
			{ /*LSB(bit 0 ) = 1 */
				reg_crc = (reg_crc >> 1)^0xA001;
			} 
			else 
			{
				reg_crc = (reg_crc>>1);
			}
		}
	}
	return reg_crc;
}

void Motor_Init(u8 SlaveAdd)
{
	Write_Slave_Word(SlaveAdd,0x030c,0x00ff);    //位置命令设置为内部触�
	
	LocationDataBuff[0]=0x0011;
	LocationDataBuff[1]=0x0000;	                 //配置PR模式
	Write_Slave_Many_Word(SlaveAdd,0x0604,LocationDataBuff,2);
	
	LocationDataBuff[0]=0x0032;      //配置位置环PR模式
	LocationDataBuff[1]=0x0003;	     //
	Write_Slave_Many_Word(SlaveAdd,0x0608,LocationDataBuff,2);
	Write_Slave_Word(SlaveAdd,0x040e,0x00ff);    //开启伺服�
}
void Motor_Speed_Task(u8 SlaveAdd,int SpeedValue)
{
//	Write_Slave_Word(SlaveAdd,0x030c,0x00ff);    //位置命令设置为内部触发
	//Write_Slave_Word(SlaveAdd,0x0528,SpeedValue);
	
//	LocationDataBuff[0]=0x0011;
//	LocationDataBuff[1]=0x0000;	                 //配置PR模式
//	Write_Slave_Many_Word(SlaveAdd,0x0604,LocationDataBuff,2);
	LocationDataBuff[0]=(u16)(SpeedValue*10); 
	LocationDataBuff[1]=(u16)((SpeedValue*10)>>16);   //设置速度
	Write_Slave_Many_Word(SlaveAdd,0x0606,LocationDataBuff,2);
	
//	Write_Slave_Word(SlaveAdd,0x040e,0x00ff);    //开启伺服
//	if(SlaveAdd==Zid)
//	{
//		Ctrl_Break(S2,Open);
//		delay_ms(20);
//	}
	Write_Slave_Word(SlaveAdd,0x050e,0x0001);    //触发PR路径为路径1，也就是#Path1
}

void Motor_Zero_Task(u8 SlaveAdd)
{
	Write_Slave_Word(SlaveAdd,0x030c,0x00ff);
	Write_Slave_Word(SlaveAdd,0x0580,1000);
	
	LocationDataBuff[0]=0x0022;      
	LocationDataBuff[1]=0x0004;
	Write_Slave_Many_Word(SlaveAdd,0x060c,LocationDataBuff,2);
	LocationDataBuff[0]=0x0000;     
	LocationDataBuff[1]=0x0000;
	Write_Slave_Many_Word(SlaveAdd,0x060e,LocationDataBuff,2);
	
	Write_Slave_Word(SlaveAdd,0x040e,0x00ff);//开启伺服
	Write_Slave_Word(SlaveAdd,0x050e,0x0003);
}

void Moter_Location_Task(u8 SlaveAdd,int SpeedValue,float Location) //位置单位:mm,精度:0.0001
{
	int LocationValue;  //Servo_Para
	u8  JudgeFlag[3];
	
	if(Servo_Para[0]==1000 && Servo_Para[1]==0)
		JudgeFlag[0]=0;
	else
		JudgeFlag[0]=1;
	
	if(Servo_Para[2]==1000 && Servo_Para[3]==0)
		JudgeFlag[1]=0;
	else
		JudgeFlag[1]=1;
	
	if(Servo_Para[4]==1000 && Servo_Para[5]==0)
		JudgeFlag[2]=0;
	else
		JudgeFlag[2]=1;
	
	switch(SlaveAdd)
	{
		case Xid :{
								if(Location<=XLocation) 
									Location=XLocation;
								if(JudgeFlag[0]==1)
									LocationValue=(int)(((Location*10000)+Encode_Value[0])*(Servo_Para[0]/1000.0)+Servo_Para[1]*10);
								else
									LocationValue=(int)(Location*10000)+Encode_Value[0];
							}break;
		case Yid :{
								if(Location>=YLocation) 
									Location=YLocation;
								if(JudgeFlag[1]==1)
									LocationValue=(int)(((Location*10000)+Encode_Value[1])*(Servo_Para[2]/1000.0)+Servo_Para[3]*10);
								else
									LocationValue=(int)(Location*10000)+Encode_Value[1];
							}break;
		case Zid :{
								if(Location>=ZLocation) 
									Location=ZLocation;
								if(JudgeFlag[2]==1)
									LocationValue=(int)(((Location*10000)+Encode_Value[2])*(Servo_Para[4]/1000.0)+Servo_Para[5]*10);
								else
									LocationValue=(int)(Location*10000)+Encode_Value[2];
							}break;
	}
	
//	switch(SlaveAdd)
//	{
//		case Xid :if(Location<=XLocation) Location=XLocation;  LocationValue=(int)(((Location*10000)+Encode_Value[0])*(Servo_Para[0]/1000.0)+Servo_Para[1]*10);break;
//		case Yid :if(Location>=YLocation) Location=YLocation;  LocationValue=(int)(((Location*10000)+Encode_Value[1])*(Servo_Para[2]/1000.0)+Servo_Para[3]*10);break;
//		case Zid :if(Location>=ZLocation) Location=ZLocation;  LocationValue=(int)(((Location*10000)+Encode_Value[2])*(Servo_Para[4]/1000.0)+Servo_Para[5]*10);break;
//	}
	
//	printf("目标值%d：%d\t零点值：%d\r\n",SlaveAdd,LocationValue,Encode_Value[SlaveAdd-1]);

//	Write_Slave_Word(SlaveAdd,0x030c,0x00ff);     //设置为内部触发
	Write_Slave_Word(SlaveAdd,0x057e,SpeedValue); //设置位置环运行速度，速度值为#3
	
//	LocationDataBuff[0]=0x0032;      //配置位置环PR模式
//	LocationDataBuff[1]=0x0003;	     //
//	Write_Slave_Many_Word(SlaveAdd,0x0608,LocationDataBuff,2);
	LocationDataBuff[0]=(u16)LocationValue;
	LocationDataBuff[1]=(u16)(LocationValue>>16);   //设置目标位置
	Write_Slave_Many_Word(SlaveAdd,0x060a,LocationDataBuff,2);
	
//	Write_Slave_Word(SlaveAdd,0x040e,0x00ff);//开启伺服
//	if(SlaveAdd==Zid)
//	{
//		Ctrl_Break(S2,Open);
//		delay_ms(20);
//	}
	Write_Slave_Word(SlaveAdd,0x050e,0x0002);//触发PR路径为路径2，也就是#Path2
}

void Cali_Zero_Task(u8 SlaveAdd)
{
	if(SlaveAdd==Xid)
		Motor_Speed_Task(SlaveAdd,60);	
	else
		Motor_Speed_Task(SlaveAdd,-60);	
}

u16  Delay_Value(int SpeedValue,float Location)
{
	u16 ret;
	ret=(u16)(Location*6000/SpeedValue);
	return ret+100;
}
u8   Motor_Finsh_Flag(u8 SlaveAdd,float Location)
{
	int TargetEncode;
	Read_Slave_Word(SlaveAdd,0x0520,Loc_Mode);
	delay_ms(100);

	if(SlaveAdd==Xid)	
		TargetEncode=(int)(Location*10000)+Encode_Value[0];
	else if(SlaveAdd==Yid)
		TargetEncode=(int)(Location*10000)+Encode_Value[1];
	else if(SlaveAdd==Zid)
		TargetEncode=(int)(Location*10000)+Encode_Value[2];
	
	if((Now_Encode_Value-TargetEncode)<3 && (TargetEncode-Now_Encode_Value)<3)
		return 1;
	else
		return 0;
}













