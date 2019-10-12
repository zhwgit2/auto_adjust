#include "ADC.h"
#include "delay.h"
#include "usart.h"

//float  AD_Sensor_Value[8]={0};

u8     CompareADC_Flag=0;
u16    ADC_Cnt=0;
u16    Sensor_temp[6];
float  AD_Sensor_Value[8];
float  ADCAveValue[8]={0};

void  Adc_Init(void)
{ 	
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE );

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //72M/6=12,ADC最大时间不能超过14M    
	//PA0--ADC_IN0  PA1--ADC_IN1  PA2--ADC_IN2  PA3--ADC_IN3  PA4--ADC_IN4  PA5--ADC_IN5  PA6--ADC_IN6  PA7--ADC_IN7  
	//PB0--ADC_IN8  PB1--ADC_IN9  PC0--ADC_IN10  PC1--ADC_IN11  PC2--ADC_IN12  PC3--ADC_IN13  PC4--ADC_IN14  PC5--ADC_IN15                   
	//PA0~3  4路作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7;	 //PA0/1/2/3 作为模拟通道输入引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	 //PA0/1/2/3 作为模拟通道输入引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//------------------------------------ADC设置--------------------------------------------------------

  ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;        //ADC工作模式:ADC1和ADC2工作在独立模式
  ADC_InitStructure.ADC_ScanConvMode =DISABLE;        //多信道扫描模式     ENABLE
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  //模数转换工作在连续转换模式  ENABLE
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //外部触发转换关闭
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;        //ADC数据右对齐
  ADC_InitStructure.ADC_NbrOfChannel = 1;        //此处开8个信道（可开的为1~16）
  ADC_Init(ADC1, &ADC_InitStructure);        //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
        
  //ADC常规信道配置
  //ADC1,ADC通道x,规则采样顺序值为y,采样时间为239.5周期
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_6,  3, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_7,  4, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_8,  5, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_9,  6, ADC_SampleTime_239Cycles5 ); 

     
  // 开启ADC的DMA支持（要实现DMA功能，还需独立配置DMA通道等参数）
  //ADC_DMACmd(ADC1, ENABLE);       //使能ADC1的DMA传输         
  ADC_Cmd(ADC1, ENABLE);           //使能指定的ADC1
  ADC_ResetCalibration(ADC1);        //复位指定的ADC1的校准寄存器
  while(ADC_GetResetCalibrationStatus(ADC1));        //获取ADC1复位校准寄存器的状态,设置状态则等待
  ADC_StartCalibration(ADC1);                //开始指定ADC1的校准状态
  while(ADC_GetCalibrationStatus(ADC1));                //获取指定ADC1的校准程序,设置状态则等待
	
	//DMA_Config(DMA1_Channel1,(u32)ADC1_DR_Address,(u32)&Sensor_temp,6);  //配置DMA
}	

void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{	
	DMA_InitTypeDef DMA_InitStructure;    
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
	
 	DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
  DMA_InitStructure.DMA_PeripheralBaseAddr =  cpar;  //DMA外设ADC基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //内存作为数据传输的目的地
  DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的数据单元大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //循环工作模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
  DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道
}
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
				 
}

u16 Get_Adc(u8 ch)   
{
	  	//设置指定ADC的规则组通道，一个序列，采样时间
	//ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度			    
  ADC_RegularChannelConfig(ADC1, ch,  1, ADC_SampleTime_239Cycles5 ); 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
//float  Adc_Cal_Val(u8 Chx) 
//{
//	u32 temp_val=0;
//	u8 t;
//	u8 times=50;
//	for(t=0;t<times;t++)
//	{
//		temp_val+=Get_Adc(Chx);
//		delay_us(1);
//	}
//	return temp_val*1.32/times;
//}
u16  value_buff[N];
float  Adc_Cal_Val(u8 ch)
{
	static int count=0;
	int i=0;
	float sum=0;
	u32 temp_val=0;
	u8 t;
	u8 times=5;
	for(t=0;t<times;t++)
	{
		temp_val+=(Get_Adc(ch)*1.0/times);
		delay_us(1);
	}
	value_buff[count%N]=temp_val;
	count++;
	for(i=0;i<N;i++)
		sum+=value_buff[i]*1.0/N;
	
	return (sum*1.32);
}


//float  Adc_Cal_Val(u8 ch)
//{
//	u8 i,j,k;
//	u16  value_buff[N];
//	u16 sum,temp;
//	for(k=0;k<N;k++)
//	{
//		value_buff[k]=Get_Adc(ch);
//		delay_us(5);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buff[i]>value_buff[i+1])
//			{
//				temp=value_buff[i];
//				value_buff[i]=value_buff[i+1];
//				value_buff[i+1]=temp;
//			}
//		}
//	}
//	for(k=3;k<N-4;k++)
//		sum+=value_buff[k];
//	
//	return (sum*1.32)/(N-7);
//}

float  Pressure_Cal_Val(u8 Chx)
{
	float  Pressure;
	Pressure=Adc_Cal_Val(Chx)*10.0/VP_Value;
	return  Pressure;
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
		CompareADC_Flag=1;
		ADC_Cnt++;
	}
}




