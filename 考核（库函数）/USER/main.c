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

u8 Kpbuff[]={"kp:"};		//�̶�λ��
u8 Kibuff[]={"ki:"};
u8 Kdbuff[]={"kd:"};
u8 Setspeed[]={"spd:"};

extern struct _pid
{
	float SetSpeed; 		//�����趨ֵ
	float ActualSpeed;  //����ʵ��ֵ
//	float SetAngle;			//�趨�Ƕ�
//	float ActualAngle;	//ʵ�ʽǶ�	
	float err; //����ƫ��ֵ
	float err_next; //������һ��ƫ��ֵ
	float err_last; //��������ǰ��ƫ��ֵ
	float Kp,Ki,Kd; //������������֡�΢��ϵ��
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(9600);

	Lcd_Init();										//��ʼ��LCD  	 	
	LCD_Clear(WHITE);
	BACK_COLOR=WHITE;
	LED_ON;	

	TIM3_Int_Init(100,719);			  //������(1ms)

	Key_Init();
  TIM4_Int_Init(1000,71);			  //����ɨ��
	
	PID_init();										//pid��ʼ��
	L298N_Init();								  //���
	Encoder_Init_TIM2();					//������ģʽ
	TIM1_PWM_Init(100,71);			 	  //PWM��� 72M/72/50
	TIM6_Int_Init(1000,719);			//��ȡ�ٶ�,pid��ֵ(10ms)

	// AT24CXX_Init();
		LCD_ShowString(60,100,Kdbuff,BLACK);									//�̶�����λ��
		LCD_ShowString(0,100,Kibuff,BLACK);
		LCD_ShowString(0,80,Kpbuff,BLACK);
		LCD_ShowString(0,60,Setspeed,BLACK);
		LCD_DrawLine(10,5,10,60,BLACK);												//����������	��
		LCD_DrawLine(10,5,130,5,BLACK);												//����������	��
		
		AT24CXX_Init();
		while(AT24CXX_Check())//��ⲻ��24c02
		{
			printf("fail!\n");
		}
		 Read();																								//���籣��
		
	TIM5_Int_Init(5000,719);
 while(1)
	{
		if(flag==500)
		{
			flag=0;			//pid��������
			LCD_ShowNum1(25,100,pid.Ki,3,BLACK);
			LCD_ShowNum1(85,100,pid.Kd,3,BLACK);
			LCD_ShowNum1(25,80,pid.Kp,4,BLACK);
			LCD_ShowNum1(30,60,pid.SetSpeed,4,BLACK);
			LCD_ShowNum1(80,60,pid.ActualSpeed,4,BLACK);				
		}
		Refresh();																						//ˢ����λ��
	}
}




