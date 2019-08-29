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
//extern float s_Angle;		//设定角度值
float s_spd=20.0;					//设定速度
float spd;								//速度
//float Angle=0.0;				//角度
int dir=0;								//方向标志位
u32 nu=0;									//取得的脉冲数
float PWMVAL=0;						//


extern struct _pid
{
	float SetSpeed; 		//定义设定值
	float ActualSpeed;  //定义实际值
//	float SetAngle;			//设定角度
//	float ActualAngle;	//实际角度	
	float err; 					//定义偏差值
	float err_next; //定义上一个偏差值
	float err_last; //定义最上前的偏差值
	float Kp,Ki,Kd; //定义比例、积分、微分系数
	float out;
	float P;
	float I;
	float D;
}pid;


void L298N_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	 //使能PA端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;				
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		   //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);						 			 //根据设定参数初始化GPIO
 GPIO_SetBits(GPIOA,GPIO_Pin_4);						 						 //PA4 输出高
 GPIO_ResetBits(GPIOA,GPIO_Pin_3);						 					 //PA3 输出低
}
 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM1_PWM_Init(u16 arr,u16 psc)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //使能GPIO外设时钟使能

   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;//TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PA8复用推挽输出

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位


	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx

  TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器

	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
	
}

//按键扫描定时器中断
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//使能定时器时钟
	TIM_TimeBaseStructure.TIM_Prescaler=psc;//预分频系数
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;//自动重装值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//初始化定时器，配置arr，psc
	
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE); //开启中断（定时器4，更新中断，ENABLE），使能定时器4的更新中断
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;////定时器x中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);//开始定时器中断，配置NVIC
	
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
//定时器5中断服务程序	 
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)!=RESET)
	{
		Show_Wave();
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
	}
}


//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM5_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig( TIM5 ,TIM_IT_Update ,ENABLE  );
	//使能指定的TIM中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM5, ENABLE);  //使能TIMx外设				 
}


extern float spd;			//实际速度
extern float s_spd;		//设定速度	

void Show_Wave()																	//lcd波形显示
{
		static	u8 x=15;
		static	u8 y_start;
		static	u8 y_end=30;
		float   lcdbuff=0;
		lcdbuff=s_spd-spd;
		if((lcdbuff>=15)||(lcdbuff<=-15))										//滤去刚开始时候的震荡
		{
			LCD_DrawLine(x,30,x+1,30,BLACK);			
		}
		else
		{
			y_start=30+lcdbuff;
			LCD_DrawLine(x,y_start,x+1,y_end,BLACK);			//根据速度去划线
			y_end=y_start;
			x++;																				//移动横坐标
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
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
	}
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3 ,TIM_IT_Update ,ENABLE);
	//使能指定的TIM中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =	2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
}

void Encoder_Init_TIM2(void)													//stm32f103编码器模式
{
		GPIO_InitTypeDef        GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //①使能 GPIOA 时钟
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1; //
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   //使能 TIM2 时钟
		TIM2->ARR = 65536-1;           //设定计数器重装值（因为没有使用更新中断所以将计数器设最大值，保证1s延时不会溢出）   
		TIM2->PSC  = 0;                //预分频器
		TIM2->CR1 &=~(3<<8);           // 选择时钟分频：不分频
		TIM2->CR1 &=~(3<<5);           //边沿对齐（根据dir位向上或者向下计数）
																	 //定时器4配置成编码器模式，双边沿触发
		TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge ,TIM_ICPolarity_BothEdge);
		TIM_SetCounter(TIM2, 0);  //计数器清零
		
		TIM_Cmd(TIM2, ENABLE);  //计数器使能，开始工作
}

//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源
   {
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源
   }
}


//定时器6中断服务程序	 
void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)!=RESET)
	{
			dir=(TIM2->CR1&0x0010)>4;                                 //取方向标志位
			if(dir>0) 																								//向下计数
				{
					nu = (65536-TIM_GetCounter(TIM2))/4;                  //除以4是因为一对脉冲计数器计数4次
					spd=nu*100/334.0;
					printf("down spd:%.1f r/s ",nu*100/334.0);	    //除以334是因为编码器转一圈是334个脉冲
				}
				else																											//向上计数
				{
					nu = TIM_GetCounter(TIM2)/4;                              
					spd=nu*100/334.0;
					printf("up spd:%.1f r/s ",nu*100/334.0);	    //除以334是因为编码器转一圈是334个脉冲
				}
			pid.ActualSpeed=spd;						//赋实际值到pid里面
			PWMVAL=PID_realize(s_spd);			//pid运算	
			TIM_SetCompare1(TIM1,PWMVAL);		//改变PWM占空比	
			TIM_SetCounter(TIM2, 0); 
			TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
	}
}


//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM6_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig( TIM6 ,TIM_IT_Update ,ENABLE  );
	//使能指定的TIM中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM6, ENABLE);  //使能TIMx外设						 
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

void Setpid(u8 *USART_RX_BUF)														//上位机修改pid参数
{
	static int  usartbuff1=0;
	if(uart_index>0)	
	{
		while(*USART_RX_BUF)
		{
			if((*USART_RX_BUF>='0')&&(*USART_RX_BUF<='9'))		//把指定数字取出来
			{
				usartbuff=*USART_RX_BUF-0x30;										//转换
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
	USART_RX_STA=0;																//清除标志位
}

void Refresh(void)															//刷新上位机修改
{
	if(USART_RX_STA)
	{
		compare(USART_RX_BUF);
		Setpid(USART_RX_BUF);
	}
}

