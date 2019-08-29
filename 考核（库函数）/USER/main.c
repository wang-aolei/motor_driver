#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "timer.h"
#include "key.h"
#include "usart.h"
#include "pid.h"
#include "24cxx.h"
#include "myiic.h"
#include "mprintf.h"

u8 Kpbuff[]={"kp:"};		//固定位置
u8 Kibuff[]={"ki:"};
u8 Kdbuff[]={"kd:"};
u8 Setspeed[]={"spd:"};

extern struct _pid
{
	float SetSpeed; 		//定义设定值
	float ActualSpeed;  //定义实际值
//	float SetAngle;			//设定角度
//	float ActualAngle;	//实际角度	
	float err; //定义偏差值
	float err_next; //定义上一个偏差值
	float err_last; //定义最上前的偏差值
	float Kp,Ki,Kd; //定义比例、积分、微分系数
	float out;
	float P;
	float I;
	float D;
}pid;

extern u8 time;					//
extern u16 flag;				//
void Refresh(void);
void Read(void);

int main(void)
{
	delay_init();	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(9600);

	Lcd_Init();										//初始化LCD  	 	
	LCD_Clear(WHITE);
	BACK_COLOR=WHITE;
	LED_ON;	

	TIM3_Int_Init(100,719);			  //计数器(1ms)

	Key_Init();
  TIM4_Int_Init(1000,71);			  //按键扫描
	
	PID_init();										//pid初始化
	L298N_Init();								  //电机
	Encoder_Init_TIM2();					//编码器模式
	TIM1_PWM_Init(100,71);			 	  //PWM输出 72M/72/50
	TIM6_Int_Init(1000,719);			//读取速度,pid赋值(10ms)

	// AT24CXX_Init();
		LCD_ShowString(60,100,Kdbuff,BLACK);									//固定参数位置
		LCD_ShowString(0,100,Kibuff,BLACK);
		LCD_ShowString(0,80,Kpbuff,BLACK);
		LCD_ShowString(0,60,Setspeed,BLACK);
		LCD_DrawLine(10,5,10,60,BLACK);												//画出坐标轴	纵
		LCD_DrawLine(10,5,130,5,BLACK);												//画出坐标轴	横
		
		AT24CXX_Init();
		while(AT24CXX_Check())//检测不到24c02
		{
			printf("fail!\n");
		}
		 Read();																								//掉电保存
		
	TIM5_Int_Init(5000,719);
 while(1)
	{
		if(flag==500)
		{
			flag=0;			//pid参数更新
			LCD_ShowNum1(25,100,pid.Ki,3,BLACK);
			LCD_ShowNum1(85,100,pid.Kd,3,BLACK);
			LCD_ShowNum1(25,80,pid.Kp,4,BLACK);
			LCD_ShowNum1(30,60,pid.SetSpeed,4,BLACK);
			LCD_ShowNum1(80,60,pid.ActualSpeed,4,BLACK);				
		}
		Refresh();																						//刷新上位机
	}
}




