#include "sys.h" 
#include "usart.h" 
#include "key.h"
#include "delay.h"
#include "lcd.h"
#include "timer.h"
#include "pid.h"
#include "24cxx.h"

char keycode=16; 
extern u8 SetIndex;
//extern float s_Angle;		//�趨�Ƕ�ֵ
float s_spd=20.0;					//�趨�ٶ�
float spd;								//�ٶ�
//float Angle=0.0;				//�Ƕ�
int dir=0;								//�����־λ
u32 nu=0;									//ȡ�õ�������
float PWMVAL=0;						//


extern struct _pid
{
	float SetSpeed; 		//�����趨ֵ
	float ActualSpeed;  //����ʵ��ֵ
//	float SetAngle;			//�趨�Ƕ�
//	float ActualAngle;	//ʵ�ʽǶ�	
	float err; 					//����ƫ��ֵ
	float err_next; //������һ��ƫ��ֵ
	float err_last; //��������ǰ��ƫ��ֵ
	float Kp,Ki,Kd; //������������֡�΢��ϵ��
	float out;
	float P;
	float I;
	float D;
}pid;


void L298N_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	 //ʹ��PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;				
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		   //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);						 			 //�����趨������ʼ��GPIO
 GPIO_SetBits(GPIOA,GPIO_Pin_4);						 						 //PA4 �����
 GPIO_ResetBits(GPIOA,GPIO_Pin_3);						 					 //PA3 �����
}
 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM1_PWM_Init(u16 arr,u16 psc)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //ʹ��GPIO����ʱ��ʹ��

   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;//TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PA8�����������

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ


	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx

  TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���

	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
	
}

//����ɨ�趨ʱ���ж�
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//ʹ�ܶ�ʱ��ʱ��
	TIM_TimeBaseStructure.TIM_Prescaler=psc;//Ԥ��Ƶϵ��
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//����ģʽ
	TIM_TimeBaseStructure.TIM_Period=arr;//�Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//��ʼ����ʱ��������arr��psc
	
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE); //�����жϣ���ʱ��4�������жϣ�ENABLE����ʹ�ܶ�ʱ��4�ĸ����ж�
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;////��ʱ��x�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);//��ʼ��ʱ���жϣ�����NVIC
	
	TIM_Cmd(TIM4,ENABLE);
}

char keybuff=0;
char key=0;
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)
	{
		 keycode=Key_Scan();
		if(keycode<16)
		{
			key=1;
			keybuff=keycode;
		}else if((keycode==16)&&(key==1))
		{
			key=0;
			keyaction(keybuff);	
		}
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
}


void Show_Wave(void);
//��ʱ��5�жϷ������	 
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)!=RESET)
	{
		Show_Wave();
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
	}
}


//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
void TIM5_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig( TIM5 ,TIM_IT_Update ,ENABLE  );
	//ʹ��ָ����TIM�ж�

	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM5, ENABLE);  //ʹ��TIMx����				 
}


extern float spd;			//ʵ���ٶ�
extern float s_spd;		//�趨�ٶ�	

void Show_Wave()																	//lcd������ʾ
{
		static	u8 x=15;
		static	u8 y_start;
		static	u8 y_end=30;
		float   lcdbuff=0;
		lcdbuff=s_spd-spd;
		if((lcdbuff>=15)||(lcdbuff<=-15))										//��ȥ�տ�ʼʱ�����
		{
			LCD_DrawLine(x,30,x+1,30,BLACK);			
		}
		else
		{
			y_start=30+lcdbuff;
			LCD_DrawLine(x,y_start,x+1,y_end,BLACK);			//�����ٶ�ȥ����
			y_end=y_start;
			x++;																				//�ƶ�������
		}
		if(x==130)
		{
			LCD_Fill(15,10,130,50,WHITE);
			x=15;
		}
}


u8 time=0;
u16 flag=0;
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
	{
		time++;
		flag++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
	}
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3 ,TIM_IT_Update ,ENABLE);
	//ʹ��ָ����TIM�ж�

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =	2;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
}

void Encoder_Init_TIM2(void)													//stm32f103������ģʽ
{
		GPIO_InitTypeDef        GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //��ʹ�� GPIOA ʱ��
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1; //
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //�������� 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   //ʹ�� TIM2 ʱ��
		TIM2->ARR = 65536-1;           //�趨��������װֵ����Ϊû��ʹ�ø����ж����Խ������������ֵ����֤1s��ʱ���������   
		TIM2->PSC  = 0;                //Ԥ��Ƶ��
		TIM2->CR1 &=~(3<<8);           // ѡ��ʱ�ӷ�Ƶ������Ƶ
		TIM2->CR1 &=~(3<<5);           //���ض��루����dirλ���ϻ������¼�����
																	 //��ʱ��4���óɱ�����ģʽ��˫���ش���
		TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge ,TIM_ICPolarity_BothEdge);
		TIM_SetCounter(TIM2, 0);  //����������
		
		TIM_Cmd(TIM2, ENABLE);  //������ʹ�ܣ���ʼ����
}

//��ʱ��2�жϷ������	 
void TIM2_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ
   {
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ
   }
}


//��ʱ��6�жϷ������	 
void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)!=RESET)
	{
			dir=(TIM2->CR1&0x0010)>4;                                 //ȡ�����־λ
			if(dir>0) 																								//���¼���
				{
					nu = (65536-TIM_GetCounter(TIM2))/4;                  //����4����Ϊһ���������������4��
					spd=nu*100/334.0;
					printf("down spd:%.1f r/s ",nu*100/334.0);	    //����334����Ϊ������תһȦ��334������
				}
				else																											//���ϼ���
				{
					nu = TIM_GetCounter(TIM2)/4;                              
					spd=nu*100/334.0;
					printf("up spd:%.1f r/s ",nu*100/334.0);	    //����334����Ϊ������תһȦ��334������
				}
			pid.ActualSpeed=spd;						//��ʵ��ֵ��pid����
			PWMVAL=PID_realize(s_spd);			//pid����	
			TIM_SetCompare1(TIM1,PWMVAL);		//�ı�PWMռ�ձ�	
			TIM_SetCounter(TIM2, 0); 
			TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
	}
}


//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
void TIM6_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig( TIM6 ,TIM_IT_Update ,ENABLE  );
	//ʹ��ָ����TIM�ж�

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM6, ENABLE);  //ʹ��TIMx����						 
}


u8 Kp_usart[]={"A"};
u8 Ki_usart[]={"B"};
u8 Kd_usart[]={"C"};
u8 Setspd_usart[]={"D"};
u8 uart_index=0;


void compare(u8 *USART_RX_BUF)
{
			if(*USART_RX_BUF=='A')  {uart_index=1;USART_RX_BUF++;}
			else if(*USART_RX_BUF=='B'){uart_index=2;USART_RX_BUF++;}
			else if(*USART_RX_BUF=='C'){uart_index=3;USART_RX_BUF++;}
			else if(*USART_RX_BUF=='D'){uart_index=4;USART_RX_BUF++;}
}

int usartbuff=0;
float usarttemp;

void Setpid(u8 *USART_RX_BUF)														//��λ���޸�pid����
{
	static int  usartbuff1=0;
	if(uart_index>0)	
	{
		while(*USART_RX_BUF)
		{
			if((*USART_RX_BUF>='0')&&(*USART_RX_BUF<='9'))		//��ָ������ȡ����
			{
				usartbuff=*USART_RX_BUF-0x30;										//ת��
				usartbuff1=usartbuff1*10+usartbuff;	
			}
			USART_RX_BUF++;
		}
		switch (uart_index)
		{
			case 1:	usarttemp=usartbuff1/10.0; pid.Kp=usarttemp; uart_index=0; usartbuff=0; usartbuff1=0; break;
			case 2:	usarttemp=usartbuff1/10.0; pid.Ki=usarttemp; uart_index=0; usartbuff=0; usartbuff1=0; break;
			case 3:	usarttemp=usartbuff1/10.0; pid.Kd=usarttemp; uart_index=0; usartbuff=0; usartbuff1=0; break;
			case 4:	usarttemp=usartbuff1/10.0; pid.SetSpeed=usarttemp;uart_index=0;usartbuff=0;usartbuff1=0; break;
			default :break;
		}
	}
	USART_RX_STA=0;																//�����־λ
}

void Refresh(void)															//ˢ����λ���޸�
{
	if(USART_RX_STA)
	{
		compare(USART_RX_BUF);
		Setpid(USART_RX_BUF);
	}
}

