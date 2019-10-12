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
	u8    Send_Buff[8];      //主函数中的数据反馈数组
	int   Send_Val;          //当前XY位置中间变量值
	u8    Calibrating=0;     //标定零点，辅助变量
	u16   ADC_Value_temp=0;  //AD值中间变量
	u16   SendAD;
	
	delay_init();	        //延时函数初始化	  
	LED_Init();		  	    //初始化与LED连接的硬件接口
	uart_init(115200);    //初始化串口
	Relay_Init();	        //继电器接口初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	RS485_Init(9600);     //Modbus初始化
	Adc_Init();           //ADC初始化
	Input_Init();         //机械开关和光电开关初始化
	//DMA_Cmd(DMA1_Channel1, ENABLE);  //DMA初始化
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	 //启动软件触发
	TIM3_Int_Init(4999,7199);     //定时器初始化
	
	delay_ms(200);
	Write_Slave_Word(Zid,0x040e,0x00ff);    //开启Z轴伺服，防止跌落
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
	
	Ctrl_Break(S1,Open);   //依次松开各轴的刹车
	delay_ms(200);
	Ctrl_Break(S2,Open);
	delay_ms(200);
	Ctrl_Break(S3,Open);
	delay_ms(200);
	Ctrl_Break(S4,Open);
	
	//delay_ms(1000); 
	delay_ms(1000);     //等待一分钟，进入系统

	while(1)
	{
/*	零点标定任务
		上位机发送标定指令，程序跳转到标定任务中
		依次开始标定XYZ轴零点
		首先，标定X轴。X轴向光电开关运动，到达后触发外部中断，此时停止X轴运动，获取当前编码器值并记录，作为X轴零点；
		然后依次是Y轴和Z轴，原理相同。
		标定完成后，跳出标定任务！
*/
		if(System_Mode==Cali_Zero)     //依次标定X、Y、Z、ZB轴零点
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
		电压对比任务	
*/
		else if(System_Mode==Compare_AD)  //进入电压对比模式
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
		关机归位任务
*/
		else if(System_Mode==Exit_System) //关机前，各轴全部归位
		{
			Moter_Location_Task(Zid,300,0);     //Z轴归位
			while(Motor_Finsh_Flag(Zid,0)==0)   //等待归位完成
				delay_ms(50); 
			Moter_Location_Task(Xid,300,0);   //X轴归位
			Moter_Location_Task(Yid,300,0);   //Y轴归位
			while(Motor_Finsh_Flag(Xid,0)==0) //等待归位完成
				delay_ms(50);
			while(Motor_Finsh_Flag(Yid,0)==0) //等待归位完成
				delay_ms(50);
			Moter_Location_Task(ZBid,300,0);  //游丝轴归位
			LED=1;
			delay_ms(500);
			LED=0;

			System_Mode=0x00;			
		}
/*
			电压调试任务
*/		
		else if(System_Mode==Debug_V)   
		{
			if(Debug_StateCtrl==1)   //开始电压调试模式
			{
				Ctrl_Relay(JA,Open);   //打开JA，使压力增大
				while(Pressure_Cal_Val(1)<PrsuAndInte[0]);   //等待压力超过最高压力值
				Ctrl_Relay(JA,Close);  //关闭JA
				delay_ms(50);          
				Ctrl_Relay(JD,Open);   //打开JD
				Ctrl_Relay(JE,Open);   //打开JE
				while(Pressure_Cal_Val(1)>PrsuAndInte[1]);  //等待压力降到区间右边值
				Ctrl_Relay(JD,Close);				                //关闭JD，压力慢速降低
				while(Pressure_Cal_Val(1)>PrsuAndInte[2])   //当压力在区间范围内时，200HZ上传数据；
				{
					//以200HZ上传AD1与AD2数据
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
				delay_ms(1000);         //当PB小于PU时，
				Ctrl_Relay(JD,Close);   //关闭JD
				Ctrl_Relay(JE,Close);   //关闭JE				
			}
		}
/*
		自动调校模式
		系统默认在自动调校任务中，如果软件或者外部按键启动后，依次执行自动调校动作
		通过状态机跳转实现，自动调校步骤
*/	
		else if(System_Mode==Auto_Cal)
		{
			if(AutoCalibration.BeginAutoCalibrationFlag==1 && AutoCalibration.PauseAutoCalibrationFlag==1)
			{
				if(AutoCalibration.StateCtrl==0)    //第一状态
				{
					Ctrl_Relay(JA,Open);  //一次打开JA、JB、JE，使夹具夹紧工件
					delay_ms(500);
					Ctrl_Relay(JE,Open);
					delay_ms(800);
					Ctrl_Relay(JB,Open);
					delay_ms(100);
					Send_Com_Task(0x01);  //获取相机XYZ位置
					AutoCalibration.StateCtrl=100;
				}
				else if(AutoCalibration.StateCtrl==1)
				{
					Moter_Location_Task(Zid,60,XYZ_Location[2]);  //移动Z轴    //速度1800 
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //等待到达目标位置
						delay_ms(50);
					Moter_Location_Task(Xid,180,XYZ_Location[0]);  //移动X轴  
					Moter_Location_Task(Yid,180,XYZ_Location[1]);  //移动Y轴
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x02);    //获取压力值  
					//delay_ms(1000);
				}
				else if(AutoCalibration.StateCtrl==2)
				{
					Ctrl_Relay(JC,Open);   //充气，使气压增大
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.3); //等待压力升高至压力*1.3
					
					Ctrl_Relay(JC,Close);  //停止充气
					Ctrl_Relay(JD,Open);   //打开JD，使气压减小

					while(Pressure_Cal_Val(1)>Pressure_Value[1]/10.0); //等待压力降低至压力/10

					Ctrl_Relay(JD,Close);  //停止放气
					Ctrl_Relay(JC,Open);   //打开IC，气压增大
					
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.3); //等待压力升高至压力*1.3	
					
					Ctrl_Relay(JC,Close);   //停止充气
					Ctrl_Relay(JD,Open);    //打开JD，使气压减小
					
					while(Pressure_Cal_Val(1)>Pressure_Value[1]/20.0); //等待压力降低至压力/20

					Ctrl_Relay(JD,Close);
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Dat_Task(0x05);  //完成第五步,等待拍照，进行第一次识别
				}
				else if(AutoCalibration.StateCtrl==3)   //完成第一次识别
				{
					Ctrl_Relay(JC,Open);    //打开JC，使气压增大
					while(Pressure_Cal_Val(1)<Pressure_Value[1]*1.1) ;//等待压力升高至压力*1.1
					Ctrl_Relay(JC,Close);   //关闭JC
					delay_ms(100);
					AutoCalibration.StateCtrl=100;
					Send_Dat_Task(0x07);  //完成第七步，等待拍照，进行第二次识别
				}
				else if(AutoCalibration.StateCtrl==4)  //完成第二次识别
				{
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x05);  //获取打孔位置XY坐标	
				}
				else if(AutoCalibration.StateCtrl==5)
				{
					Moter_Location_Task(Xid,1800,Calculate_XY[0]);  //移动X轴
					Moter_Location_Task(Yid,1800,Calculate_XY[1]);  //移动Y轴
					delay_ms(1000);
					delay_ms(1000);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x03);  //获取钻头参数			
				}
				else if(AutoCalibration.StateCtrl==6)
				{
					Motor_Speed_Task(Punchid,(u16)Drill_Value[2]);  //启动钻孔电机，转速为设定值
					//delay_ms(500);				
					Moter_Location_Task(Zid,1800,Drill_Value[0]);  //Z轴移动至起钻位置
					while(Motor_Finsh_Flag(Zid,Drill_Value[0])==0) //等待到达目标位置
						delay_ms(50);
					Motor_Speed_Task(Punchid,(u16)(Drill_Value[2]*6));//钻孔转速为6倍设定值
					Moter_Location_Task(Zid,Drill_Value[3],Drill_Value[1]);  //Z轴移动至止钻位置
					while(Motor_Finsh_Flag(Zid,Drill_Value[1])==0) //等待到达目标位置
						delay_ms(50);		
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);  //退回原来位置
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //等待到达目标位置
						delay_ms(50);
					Motor_Speed_Task(Punchid,0);    //停止打孔轴
					AutoCalibration.StateCtrl=7;    //直接跳转到打孔验证步骤
//					AutoCalibration.StateCtrl=100;
//					Send_Com_Task(0x06);					
				}
				else if(AutoCalibration.StateCtrl==7)
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[0]);  //移动X轴
					Moter_Location_Task(Yid,1800,XYZ_Location[1]);  //移动Y轴
					AutoCalibration.StateCtrl=100;
					while(Motor_Finsh_Flag(Xid,XYZ_Location[0])==0) //等待到达目标位置
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[1])==0) //等待到达目标位置
						delay_ms(50);
					Send_Dat_Task(13);  //完成第十三步,等待拍照，进行第三次识别						
				}
				else if(AutoCalibration.StateCtrl==8)    //打孔正确，上游丝角度
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[3]);  //移动X轴
					Moter_Location_Task(Yid,1800,XYZ_Location[4]);  //移动Y轴
					while(Motor_Finsh_Flag(Xid,XYZ_Location[3])==0) //等待到达目标位置
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[4])==0) //等待到达目标位置
						delay_ms(50);	
					
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);  //移动Z轴
					while(Motor_Finsh_Flag(Zid,XYZ_Location[2])==0) //等待到达目标位置
						delay_ms(50);

					Ctrl_Relay(JF,Open);  //打开JF
					//delay_ms(100);
					AutoCalibration.StateCtrl=100;
					Send_Com_Task(0x07);    //获取游丝角度
				
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
					Moter_Location_Task(Xid,1800,0);  //移动X轴   //速度1800
					Moter_Location_Task(Yid,1800,0);  //移动Y轴
					Ctrl_Relay(JA,Close);
					Ctrl_Relay(JB,Close);
					
					AutoCalibration.BeginAutoCalibrationFlag=0;
					AutoCalibration.PauseAutoCalibrationFlag=0;
					AutoCalibration.StateCtrl=0;
				}
				else if(AutoCalibration.StateCtrl==10)  //打孔错误
				{
					Moter_Location_Task(Xid,1800,XYZ_Location[0]);  //移动X轴
					Moter_Location_Task(Yid,1800,XYZ_Location[1]);  //移动Y轴
					
					while(Motor_Finsh_Flag(Xid,XYZ_Location[0])==0) //等待到达目标位置
						delay_ms(50);
					while(Motor_Finsh_Flag(Yid,XYZ_Location[1])==0) //等待到达目标位置
						delay_ms(50);
					
					Moter_Location_Task(Zid,1800,XYZ_Location[2]);
					
					AutoCalibration.BeginAutoCalibrationFlag=0;  //等待	重新启动自动调校
					AutoCalibration.PauseAutoCalibrationFlag=1;
					AutoCalibration.StateCtrl=0;
				}
			}
		}

/*
		数据反馈任务
		当X、Y轴有动作时，反馈实时位置坐标
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
		当XYZ轴有位置发生变化时，反馈对应的坐标值
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
		上位机命令
		上位机发送指令通过中断触发
		命令的执行函数在主函数中进行
*/
		if(Uart_ReciverFlag==1)
		{
			Deal_Rx_Task(Usart_Msg);
			Uart_ReciverFlag=0;
		}
/*
		急停按键临时报警
*/
		while(K3==0)
			BELL=1;
		BELL=0;
		
//		if(G4==1)    //对应X轴
//			Motor_Speed_Task(Xid,0);
//		
//		if(G3==1)   //对应Y轴
//			Motor_Speed_Task(Yid,0);
//		
//		if(G2==1)   //对应Z轴
//			Motor_Speed_Task(Zid,0);
	}
}



