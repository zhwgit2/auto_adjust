#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "OUT.h"
#include "rs485.h"
#include "ADC.h" 
#include "IN.h"

int main(void)
{		
	//u16   t;
	u8    Send_Buff[8];      //�������е����ݷ�������
	int   Send_Val;          //��ǰXYλ���м����ֵ
	u8    Calibrating=0;     //�궨��㣬��������
	u16   ADC_Value_temp=0;  //ADֵ�м����
	u16   SendAD;
	
	delay_init();	        //��ʱ������ʼ��	  
	LED_Init();		  	    //��ʼ����LED���ӵ�Ӳ���ӿ�
	uart_init(115200);    //��ʼ������
	Relay_Init();	        //�̵����ӿڳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	RS485_Init(9600);     //Modbus��ʼ��
	Adc_Init();           //ADC��ʼ��
	Input_Init();         //��е���غ͹�翪�س�ʼ��
	//DMA_Cmd(DMA1_Channel1, ENABLE);  //DMA��ʼ��
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	 //�����������
	TIM3_Int_Init(4999,7199);     //��ʱ����ʼ��
	
	delay_ms(200);
	Write_Slave_Word(Zid,0x040e,0x00ff);    //����Z���ŷ�����ֹ����
	Motor_Init(Xid);
	Motor_Init(Yid);
	Motor_Init(Zid);
	Motor_Init(ZBid);
	Motor_Init(Punchid);
	delay_ms(200);
	Motor_Init(Xid);
	Motor_Init(Yid);
	Motor_Init(Zid);
	Motor_Init(ZBid);
	Motor_Init(Punchid);
	delay_ms(200);
	Motor_Init(Xid);
	Motor_Init(Yid);
	Motor_Init(Zid);
	Motor_Init(ZBid);
	Motor_Init(Punchid);
	delay_ms(200);
	Motor_Init(Xid);
	Motor_Init(Yid);
	Motor_Init(Zid);
	Motor_Init(ZBid);
	Motor_Init(Punchid);
	
	Ctrl_Break(S1,Open);   //�����ɿ������ɲ��
	delay_ms(200);
	Ctrl_Break(S2,Open);
	delay_ms(200);
	Ctrl_Break(S3,Open);
	delay_ms(200);
	Ctrl_Break(S4,Open);
	
	//delay_ms(1000); 
	delay_ms(1000);     //�ȴ�һ���ӣ�����ϵͳ

	while(1)
	{
/*	���궨����
		��λ�����ͱ궨ָ�������ת���궨������
		���ο�ʼ�궨XYZ�����
		���ȣ��궨X�ᡣX�����翪���˶�������󴥷��ⲿ�жϣ���ʱֹͣX���˶�����ȡ��ǰ������ֵ����¼����ΪX����㣻
		Ȼ��������Y���Z�ᣬԭ����ͬ��
		�궨��ɺ������궨����
*/
		if(System_Mode==Cali_Zero)     //���α궨X��Y��Z��ZB�����
		{
			if(Cali_Complete==0 && Calibrating==0)
			{
				Cali_Zero_Task(Xid);
				Calibrating=1;
			}
			else if(Cali_Complete==1 && Calibrating==0)
			{
				Cali_Zero_Task(Yid);
				Calibrating=1;
			}
			else if(Cali_Complete==2 && Calibrating==0)
			{
				Cali_Zero_Task(Zid);
				Calibrating=1;
			}
			
			if(Calibration_Add!=0)
			{
				switch(Calibration_Add)
				{
					case Xid:Read_Slave_Word(Xid,0x0520,Cal_Mode);delay_ms(50);Cali_Complete=1;Calibrating=0;break;
					case Yid:Read_Slave_Word(Yid,0x0520,Cal_Mode);delay_ms(50);Cali_Complete=2;Calibrating=0;break;
					case Zid:Read_Slave_Word(Zid,0x0520,Cal_Mode);delay_ms(50);Cali_Complete=3;Calibrating=0;break;
				}
				Calibration_Add=0;
				if(Cali_Complete==3)
				{
					LED=1;
					BELL=1;
					System_Mode=0;
					Cali_Complete=0;
					delay_ms(300);
					BELL=0;	
					LED=0;					
				}				
			}		
		}
/*
		��ѹ�Ա�����	
*/
		else if(System_Mode==Compare_AD)  //�����ѹ�Ա�ģʽ
		{
			if(CompareADC_Flag==1)
			{
				Send_Buff[0]=0x55;
				Send_Buff[1]=0x08;
				Send_Buff[2]=ADC_Cnt>>8;
				Send_Buff[3]=ADC_Cnt>>0;
				
				ADC_Value_temp=(u16)Adc_Cal_Val(0);			
				Send_Buff[4]=(u8)(ADC_Value_temp>>8);
				Send_Buff[5]=(u8)(ADC_Value_temp>>0);	
				
				ADC_Value_temp=(u16)Adc_Cal_Val(1);
				Send_Buff[6]=(u8)(ADC_Value_temp>>8);
				Send_Buff[7]=(u8)(ADC_Value_temp>>0);
				
				Feedback(Send_Buff);
				CompareADC_Flag=0;
			}
			if(ADC_Cnt>=CompareADC[2])
			{
				TIM_Cmd(TIM3, DISABLE);
				System_Mode=0;
				ADC_Cnt=0;
			}
		}
/*
		�ػ���λ����
*/
		else if(System_Mode==Exit_System) //�ػ�ǰ������ȫ����λ
		{
			Moter_Location_Task(Zid,300,0);     //Z���λ
			while(Motor_Finsh_Flag(Zid,0)==0)   //�ȴ���λ���
				delay_ms(50); 
			Moter_Location_Task(Xid,300,0);   //X���λ
			Moter_Location_Task(Yid,300,0);   //Y���λ
			while(Motor_Finsh_Flag(Xid,0)==0) //�ȴ���λ���
				delay_ms(50);
			while(Motor_Finsh_Flag(Yid,0)==0) //�ȴ���λ���
				delay_ms(50);
			Moter_Location_Task(ZBid,300,0);  //��˿���λ
			LED=1;
			delay_ms(500);
			LED=0;

			System_Mode=0x00;			
		}
/*
			��ѹ��������
*/		
		else if(System_Mode==Debug_V)   
		{
			if(Debug_StateCtrl==1)   //��ʼ��ѹ����ģʽ
			{
				Ctrl_Relay(JA,Open);   //��JA��ʹѹ������
				while(Pressure_Cal_Val(1)<PrsuAndInte[0]);   //�ȴ�ѹ���������ѹ��ֵ
				Ctrl_Relay(JA,Close);  //�ر�JA
				delay_ms(50);          
				Ctrl_Relay(JD,Open);   //��JD
				Ctrl_Relay(JE,Open);   //��JE
				while(Pressure_Cal_Val(1)>PrsuAndInte[1]);  //�ȴ�ѹ�����������ұ�ֵ
				Ctrl_Relay(JD,Close);				                //�ر�JD��ѹ�����ٽ���
				while(Pressure_Cal_Val(1)>PrsuAndInte[2])   //��ѹ�������䷶Χ��ʱ��200HZ�ϴ����ݣ�
				{
					//��200HZ�ϴ�AD1��AD2����
					Send_Buff[0]=0x55;
					Send_Buff[1]=0x0a;
					Send_Buff[2]=0x00;
					Send_Buff[3]=0x00;
					
					SendAD=Adc_Cal_Val(0);
					
					Send_Buff[4]=(u8)(SendAD>>8);
					Send_Buff[5]=(u8)(SendAD>>0);
					
					SendAD=Adc_Cal_Val(1);
					Send_Buff[6]=(u8)(SendAD>>8);
					Send_Buff[7]=(u8)(SendAD>>0); 
					
					Feedback(Send_Buff);
					
				}
				Ctrl_Relay(JD,Open);
				delay_ms(1000);
				delay_ms(1000);         //��PBС��PUʱ��
				Ctrl_Relay(JD,Close);   //�ر�JD
				Ctrl_Relay(JE,Close);   //�ر�JE				
			}
		}
/*
		�Զ���Уģʽ
		ϵͳĬ�����Զ���У�����У������������ⲿ��������������ִ���Զ���У����
		ͨ��״̬����תʵ�֣��Զ���У����
*/	
		else if(System_Mode==Auto_Cal)
		{
			if(AutoCalibration.BeginAutoCalibrationFlag==1 && AutoCalibration.PauseAutoCalibrationFlag==1)
			{
				if(AutoCalibration.StateCtrl==0)    //��һ״̬
				{
					Ctrl_Relay(JA,Open);  //һ�δ�JA��JB��JE��ʹ�о߼н�����
					delay_ms(500);
					Ctrl_Relay(JE,Open);
					delay_ms(800);
					Ctrl_Relay(JB,Open);
					delay_ms(100);
					Send_Com_Task(0x01);  //��ȡ���XYZλ��
					AutoCalibration.StateCtrl=100;
				}
				else if(AutoCalibration.StateCtrl==1)
				{
					Moter_Location_Task(Zid,60,XYZ_Location[2]);  //�ƶ�Z��    //�ٶ�1800 
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					Moter_Location_Task(Xid,180,XYZ_Location[0]);  //�ƶ�X��  
					Moter_Location_Task(Yid,180,XYZ_Location[1]);  //�ƶ�Y��
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x02);    //��ȡѹ��ֵ  
					//delay_ms(1000);
				}
				else if(AutoCalibration.StateCtrl==2)
				{
					Ctrl_Relay(JC,Open);   //������ʹ��ѹ����
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.3); //�ȴ�ѹ��������ѹ��*1.3
					
					Ctrl_Relay(JC,Close);  //ֹͣ����
					Ctrl_Relay(JD,Open);   //��JD��ʹ��ѹ��С

					while(Pressure_Cal_Val(1)>Pressure_Value[1]/10.0); //�ȴ�ѹ��������ѹ��/10

					Ctrl_Relay(JD,Close);  //ֹͣ����
					Ctrl_Relay(JC,Open);   //��IC����ѹ����
					
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.3); //�ȴ�ѹ��������ѹ��*1.3	
					
					Ctrl_Relay(JC,Close);   //ֹͣ����
					Ctrl_Relay(JD,Open);    //��JD��ʹ��ѹ��С
					
					while(Pressure_Cal_Val(1)>Pressure_Value[1]/20.0); //�ȴ�ѹ��������ѹ��/20

					Ctrl_Relay(JD,Close);
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Dat_Task(0x05);  //��ɵ��岽,�ȴ����գ����е�һ��ʶ��
				}
				else if(AutoCalibration.StateCtrl==3)   //��ɵ�һ��ʶ��
				{
					Ctrl_Relay(JC,Open);    //��JC��ʹ��ѹ����
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.1) ;//�ȴ�ѹ��������ѹ��*1.1
					Ctrl_Relay(JC,Close);   //�ر�JC
					delay_ms(100);
					AutoCalibration.StateCtrl=100;
					Send_Dat_Task(0x07);  //��ɵ��߲����ȴ����գ����еڶ���ʶ��
				}
				else if(AutoCalibration.StateCtrl==4)  //��ɵڶ���ʶ��
				{
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x05);  //��ȡ���λ��XY����	
				}
				else if(AutoCalibration.StateCtrl==5)
				{
					Moter_Location_Task(Xid,1800,Calculate_XY[0]);  //�ƶ�X��
					Moter_Location_Task(Yid,1800,Calculate_XY[1]);  //�ƶ�Y��
					delay_ms(1000);
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x03);  //��ȡ��ͷ����			
				}
				else if(AutoCalibration.StateCtrl==6)
				{
					Motor_Speed_Task(Punchid,(u16)Drill_Value[2]);  //������׵����ת��Ϊ�趨ֵ
					//delay_ms(500);				
					Moter_Location_Task(Zid,1800,Drill_Value[0]);  //Z���ƶ�������λ��
					while(Motor_Finsh_Flag(Zid,Drill_Value[0])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					Motor_Speed_Task(Punchid,(u16)(Drill_Value[2]*6));//���ת��Ϊ6���趨ֵ
					Moter_Location_Task(Zid,Drill_Value[3],Drill_Value[1]);  //Z���ƶ���ֹ��λ��
					while(Motor_Finsh_Flag(Zid,Drill_Value[1])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);		
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);  //�˻�ԭ��λ��
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					Motor_Speed_Task(Punchid,0);    //ֹͣ�����
					AutoCalibration.StateCtrl=7;    //ֱ����ת�������֤����
//					AutoCalibration.StateCtrl=100;
//					Send_Com_Task(0x06);					
				}
				else if(AutoCalibration.StateCtrl==7)
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[0]);  //�ƶ�X��
					Moter_Location_Task(Yid,1800,XYZ_Location[1]);  //�ƶ�Y��
					AutoCalibration.StateCtrl=100;
					while(Motor_Finsh_Flag(Xid,XYZ_Location[0])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[1])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					Send_Dat_Task(13);  //��ɵ�ʮ����,�ȴ����գ����е�����ʶ��						
				}
				else if(AutoCalibration.StateCtrl==8)    //�����ȷ������˿�Ƕ�
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[3]);  //�ƶ�X��
					Moter_Location_Task(Yid,1800,XYZ_Location[4]);  //�ƶ�Y��
					while(Motor_Finsh_Flag(Xid,XYZ_Location[3])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[4])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);	
					
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);  //�ƶ�Z��
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);

					Ctrl_Relay(JF,Open);  //��JF
					//delay_ms(100);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x07);    //��ȡ��˿�Ƕ�
				
				}
				else if(AutoCalibration.StateCtrl==9)
				{
					Moter_Location_Task(ZBid,120,(u16)Angle_Value);  
					Ctrl_Relay(JE,Close);
					delay_ms(1000);	
					Moter_Location_Task(ZBid,120,(u16)Angle_Value+180);
					Ctrl_Relay(JF,Close);
					delay_ms(1000);
					delay_ms(300);				
					Moter_Location_Task(Xid,1800,0);  //�ƶ�X��   //�ٶ�1800
					Moter_Location_Task(Yid,1800,0);  //�ƶ�Y��
					Ctrl_Relay(JA,Close);
					Ctrl_Relay(JB,Close);
					
					AutoCalibration.BeginAutoCalibrationFlag=0;
					AutoCalibration.PauseAutoCalibrationFlag=0;
					AutoCalibration.StateCtrl=0;
				}
				else if(AutoCalibration.StateCtrl==10)  //��״���
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[0]);  //�ƶ�X��
					Moter_Location_Task(Yid,1800,XYZ_Location[1]);  //�ƶ�Y��
					
					while(Motor_Finsh_Flag(Xid,XYZ_Location[0])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[1])==0) //�ȴ�����Ŀ��λ��
						delay_ms(50);
					
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);
					
					AutoCalibration.BeginAutoCalibrationFlag=0;  //�ȴ�	���������Զ���У
					AutoCalibration.PauseAutoCalibrationFlag=1;
					AutoCalibration.StateCtrl=0;
				}
			}
		}

/*
		���ݷ�������
		��X��Y���ж���ʱ������ʵʱλ������
*/
		if(Return_Location_Flag==1)
		{
			Send_Buff[0]=0x55;
			Send_Buff[1]=0x04;
			
			Read_Slave_Word(Xid,0x0520,Loc_Mode);
			delay_ms(50);
			Read_Slave_Word(Xid,0x0520,Loc_Mode);
			delay_ms(50);
			Send_Val=-(Now_Encode_Value-Encode_Value[0]);
			if(Send_Val<0)
				Send_Val=0;
				
			Send_Buff[2]=Send_Val>>16;
			Send_Buff[3]=Send_Val>>8;
			Send_Buff[4]=Send_Val>>0;  
			
			Read_Slave_Word(Yid,0x0520,Loc_Mode);
			delay_ms(50);
			Send_Val=(Now_Encode_Value-Encode_Value[1]);
			delay_ms(50);
			Send_Val=(Now_Encode_Value-Encode_Value[1]);
			if(Send_Val<0)
				Send_Val=0;
			
			Send_Buff[5]=Send_Val>>16;
			Send_Buff[6]=Send_Val>>8;
			Send_Buff[7]=Send_Val>>0;  
			
			Feedback(Send_Buff);
			
			Return_Location_Flag=0;
		}
/*
		��XYZ����λ�÷����仯ʱ��������Ӧ������ֵ
*/
		if(XYZ_Position_Flag[1]==1)
		{
			Send_Buff[0]=0x55;
			Send_Buff[1]=0x0b;
			Send_Buff[3]=0x00;
			Send_Buff[4]=0x00;
			switch(XYZ_Position_Flag[0])
			{
				case 1:{
									Read_Slave_Word(Xid,0x0520,Loc_Mode);
									delay_ms(50);
									Read_Slave_Word(Xid,0x0520,Loc_Mode);
									delay_ms(50);
									Send_Val=-(Now_Encode_Value-Encode_Value[0]);
									if(Send_Val<0)
										Send_Val=0;
									
									Send_Buff[2]=XYZ_Position_Flag[0];
									Send_Buff[5]=Send_Val>>16;
									Send_Buff[6]=Send_Val>>8;
									Send_Buff[7]=Send_Val>>0;
								};break;
				case 2:{
									Read_Slave_Word(Yid,0x0520,Loc_Mode);
									delay_ms(50);
									Read_Slave_Word(Yid,0x0520,Loc_Mode);
									delay_ms(50);
									Send_Val=(Now_Encode_Value-Encode_Value[1]);
									if(Send_Val<0)
										Send_Val=0;
									
									Send_Buff[2]=XYZ_Position_Flag[0];
									Send_Buff[5]=Send_Val>>16;
									Send_Buff[6]=Send_Val>>8;
									Send_Buff[7]=Send_Val>>0;
								};break;
				case 3:{
									Read_Slave_Word(Zid,0x0520,Loc_Mode);
									delay_ms(50);
									Read_Slave_Word(Zid,0x0520,Loc_Mode);
									delay_ms(50);
									Send_Val=(Now_Encode_Value-Encode_Value[2]);
									if(Send_Val<0)
										Send_Val=0;
									
									Send_Buff[2]=XYZ_Position_Flag[0];
									Send_Buff[5]=Send_Val>>16;
									Send_Buff[6]=Send_Val>>8;
									Send_Buff[7]=Send_Val>>0;
								};break;
			}			
			Feedback(Send_Buff);
			XYZ_Position_Flag[1]=0;
		}
		
/*
		��λ������
		��λ������ָ��ͨ���жϴ���
		�����ִ�к������������н���
*/
		if(Uart_ReciverFlag==1)
		{
			Deal_Rx_Task(Usart_Msg);
			Uart_ReciverFlag=0;
		}
/*
		��ͣ������ʱ����
*/
		while(K3==0)
			BELL=1;
		BELL=0;
		
//		if(G4==1)    //��ӦX��
//			Motor_Speed_Task(Xid,0);
//		
//		if(G3==1)   //��ӦY��
//			Motor_Speed_Task(Yid,0);
//		
//		if(G2==1)   //��ӦZ��
//			Motor_Speed_Task(Zid,0);
	}
}



