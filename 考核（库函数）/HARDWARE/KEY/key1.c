#include "sys.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "24cxx.h"
#include "mprintf.h"
u8 SetIndex=0;
u8 MODE=0;
//float s_Angle=360;			//设定转动角度
//extern float Angle;

void Read(void);
extern struct _pid
{
	float SetSpeed; 		//定义设定值
	float ActualSpeed;  //定义实际值
	float SetAngle;			//设定角度
	float ActualAngle;	//实际角度	
	float err; //定义偏差值
	float err_next; //定义上一个偏差值
	float err_last; //定义最上前的偏差值
	float Kp,Ki,Kd; //定义比例、积分、微分系数
	float out;
	float P;
	float I;
	float D;
}pid;


//按键的初始化	
void Key_Init(void)
{
	RCC->APB2ENR|=1<<3;						//使能B时钟	
	RCC->APB2ENR|=1<<4;						//使能C时钟	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);     //禁用JTAG
	GPIOB->CRL&=0x0000FFFF;				//初始化B4—B7
	GPIOB->CRL|=0x88880000;				//B4-B7配置为上拉输入
	GPIOB->ODR|=0xf<<4;							//B4-B7配置为上拉输入
	
	GPIOC->CRL&=0xFFFF0000;					//初始化PC0-3
	GPIOC->CRL|=0x00003333;				  //PC0-3配置为推挽输出
	GPIOC->ODR|=0x00;							  
}

u8 Key_Scan(void)
{
		char keyValue=16;//按键值
		GPIOC->ODR&=0x0000;
		GPIOC->ODR|=0x0007;			//让PC0~3输出0111
      switch(GPIOB->IDR & 0x00f0)
        {
            case 0x0070 : keyValue=15;break;
            case 0x00b0 : keyValue=14;break;
            case 0x00d0 : keyValue=13;break;
            case 0x00e0 : keyValue=12;break;  
				}
			GPIOC->ODR&=0x0000;
			GPIOC->ODR|=0x000b;			
		//GPIOC->ODR & 0x0000| 0x000b;//让PC0~3输出，检测第三行
		//		delay_ms(1);
      switch(GPIOB->IDR & 0x00f0)
        {
            case 0x0070 : keyValue=11;break;
            case 0x00b0 : keyValue=10;break;
            case 0x00d0 : keyValue=9;break;
            case 0x00e0 : keyValue=8; break; 	
					default :break;
        }
				GPIOC->ODR&=0x0000;
				GPIOC->ODR|=0x000d;			
		//GPIOC->ODR & 0x0000| 0x000d;//让PC0~3输出，检测第二行
		//		delay_ms(1);
      switch(GPIOB->IDR & 0x00f0)
        {
            case 0x0070 : keyValue=7;break;
            case 0x00b0 : keyValue=6;break;
            case 0x00d0 : keyValue=5;break;
            case 0x00e0 : keyValue=4;break; 
						default :break;
        }
		GPIOC->ODR&=0x0000;
		GPIOC->ODR|=0x000e;			
    //GPIOC->ODR & 0x0000| 0x000e;//让PC0~3输出，检测第一行
		//		delay_ms(1);
      switch(GPIOB->IDR & 0x00f0)
        {
            case 0x0070 : keyValue=3;break;
            case 0x00b0 : keyValue=2;break;
            case 0x00d0 : keyValue=1;break;
            case 0x00e0 : keyValue=0;break; 
						default :break;
        }
    return keyValue;
}


float keytemp=0;

u16    keytemp1[1];
u8		keytemp2[1];
u8    keytemp3[1];
u8		keytemp4[1];


extern char Esc;
void change (u8 keycodes,u8 SetIndex)
{
	static int addend =0;
	if(Esc==1){addend=0;Esc=0;}
	addend=addend*10+keycodes;
//	printf("%d\n",addend);
	keytemp=addend/10.0;
}


//右移函数
void SetRightShift()
{
		switch (SetIndex)
		{
			case 0: SetIndex=1;  break;			//坐标显示
			case 1: SetIndex=2;  break;		
			case 2: SetIndex=3;  break;		
			case 3: SetIndex=4;  break;		
			default:SetIndex=0;	 break;	
		}
}

u8 kptemp[4];
u8 kitemp[4];
u8 kdtemp[4];
u16 Setspdtemp[4];
u8 i,*px;
void *pf;

extern float s_spd;
char Esc=0;
char Read_index=0;

void keyaction(char Keycode)
{
	if(Keycode<10)			//更改数值
	{
		change(Keycode,SetIndex);
	}
	if(Keycode==10)			
	{
		m_printf("%d\n",10);
		SetRightShift();				//光标向后跳
		Esc=1;
		keytemp=0;									//取消键
	}
	else if(Keycode==11)			//eeprom写
	{
	}
	else if(Keycode==12)			//eeprom读
	{
	}
	else if(Keycode==13)																		//切换模式（MODE）
	{
		printf("kp:%.1f\n",pid.Kp);
		printf("ki:%.1f\n",pid.Ki);
		printf("kd:%.1f\n",pid.Kd);
		printf("spd:%.1f\n",pid.SetSpeed);
		/*
		Angle=0;
		pid.Kp=0.355;																					//角度pid参数
		pid.Ki=0.4;
		pid.Kd=2.0;
		*/
	}
	else if(Keycode==14)
	{
		Esc=1;						//取消键
		m_printf("%d\n",14);
	}
	else if(Keycode==15)
	{
		m_printf("%d\n",15);//确认键	
		switch (SetIndex)
		{
			case 1:	s_spd=keytemp; Esc=1;SetIndex=0;keytemp1[0]=keytemp*10;AT24CXX_Write(30,(u8*)keytemp1,1);Read_index=1;LCD_ShowNum1(30,60,pid.SetSpeed,4,BLACK);break;//设定速度
			case 2:	pid.Kp=keytemp;Esc=1;SetIndex=0;keytemp2[0]=keytemp*10;AT24CXX_Write(0, keytemp2,1);Read_index=2;LCD_ShowNum1(25,80,pid.Kp,4,BLACK);			break;  //kp
			case 3:	pid.Ki=keytemp;Esc=1;SetIndex=0;keytemp3[0]=keytemp*10;AT24CXX_Write(10,keytemp3,1);Read_index=3;LCD_ShowNum1(25,100,pid.Ki,3,BLACK);			break;	//ki
			case 4:	pid.Kd=keytemp;Esc=1;SetIndex=0;keytemp4[0]=keytemp*10;AT24CXX_Write(20,keytemp4,1);Read_index=4;LCD_ShowNum1(85,100,pid.Kd,3,BLACK);			break;	//kd
		}
	}
}
void Read(void)
{
		switch (Read_index)
		{
				case 1:	AT24CXX_Read(30,(u8*)Setspdtemp,1);pid.SetSpeed=Setspdtemp[0]/10.0;Read_index=0;break;
				case 2: AT24CXX_Read(0, kptemp,1);pid.Kp=kptemp[0]/10.0;Read_index=0; 						 break;
				case 3:	AT24CXX_Read(10,kitemp,1);pid.Ki=kitemp[0]/10.0;Read_index=0; 						 break;
				case 4: AT24CXX_Read(20,kdtemp,1);pid.Kd=kdtemp[0]/10.0;Read_index=0; 						 break;
		}
}




