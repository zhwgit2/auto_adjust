#ifndef __ADC_H
#define __ADC_H
#include "sys.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)

#define   N   10

extern   u8   CompareADC_Flag;
extern   u16  ADC_Cnt;

void   Adc_Init(void);
void   DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void   TIM3_Int_Init(u16 arr,u16 psc);
float  Adc_Cal_Val(u8 Chx);
float  Pressure_Cal_Val(u8 Chx);
u16    Get_Adc(u8 ch);
#endif
