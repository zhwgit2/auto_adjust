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

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //72M/6=12,ADC���ʱ�䲻�ܳ���14M    
	//PA0--ADC_IN0  PA1--ADC_IN1  PA2--ADC_IN2  PA3--ADC_IN3  PA4--ADC_IN4  PA5--ADC_IN5  PA6--ADC_IN6  PA7--ADC_IN7  
	//PB0--ADC_IN8  PB1--ADC_IN9  PC0--ADC_IN10  PC1--ADC_IN11  PC2--ADC_IN12  PC3--ADC_IN13  PC4--ADC_IN14  PC5--ADC_IN15                   
	//PA0~3  4·��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7;	 //PA0/1/2/3 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	 //PA0/1/2/3 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//------------------------------------ADC����--------------------------------------------------------

  ADC_DeInit(ADC1);  //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;        //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
  ADC_InitStructure.ADC_ScanConvMode =DISABLE;        //���ŵ�ɨ��ģʽ     ENABLE
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  //ģ��ת������������ת��ģʽ  ENABLE
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //�ⲿ����ת���ر�
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;        //ADC�����Ҷ���
  ADC_InitStructure.ADC_NbrOfChannel = 1;        //�˴���8���ŵ����ɿ���Ϊ1~16��
  ADC_Init(ADC1, &ADC_InitStructure);        //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���
        
  //ADC�����ŵ�����
  //ADC1,ADCͨ��x,�������˳��ֵΪy,����ʱ��Ϊ239.5����
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_6,  3, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_7,  4, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_8,  5, ADC_SampleTime_239Cycles5 ); 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_9,  6, ADC_SampleTime_239Cycles5 ); 

     
  // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����
  //ADC_DMACmd(ADC1, ENABLE);       //ʹ��ADC1��DMA����         
  ADC_Cmd(ADC1, ENABLE);           //ʹ��ָ����ADC1
  ADC_ResetCalibration(ADC1);        //��λָ����ADC1��У׼�Ĵ���
  while(ADC_GetResetCalibrationStatus(ADC1));        //��ȡADC1��λУ׼�Ĵ�����״̬,����״̬��ȴ�
  ADC_StartCalibration(ADC1);                //��ʼָ��ADC1��У׼״̬
  while(ADC_GetCalibrationStatus(ADC1));                //��ȡָ��ADC1��У׼����,����״̬��ȴ�
	
	//DMA_Config(DMA1_Channel1,(u32)ADC1_DR_Address,(u32)&Sensor_temp,6);  //����DMA
}	

void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{	
	DMA_InitTypeDef DMA_InitStructure;    
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
	
 	DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr =  cpar;  //DMA����ADC����ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //�ڴ���Ϊ���ݴ����Ŀ�ĵ�
  DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA��������ݵ�Ԫ��С
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //ѭ������ģʽ
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
  DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
}
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���
				 
}

u16 Get_Adc(u8 ch)   
{
	  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	//ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles );	//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    
  ADC_RegularChannelConfig(ADC1, ch,  1, ADC_SampleTime_239Cycles5 ); 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
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

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
		CompareADC_Flag=1;
		ADC_Cnt++;
	}
}




