#include "sys.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "24cxx.h"
#include "mprintf.h"
u8 SetIndex=0;
u8 MODE=0;
//float s_Angle=360;			//�趨ת���Ƕ�
//extern float Angle;

void Read(void);
extern struct _pid
{
	float SetSpeed; 		//�����趨ֵ
	float ActualSpeed;  //����ʵ��ֵ
	float SetAngle;			//�趨�Ƕ�
	float ActualAngle;	//ʵ�ʽǶ�	
	float err; //����ƫ��ֵ
	float err_next; //������һ��ƫ��ֵ
	float err_last; //��������ǰ��ƫ��ֵ
	float Kp,Ki,Kd; //������������֡�΢��ϵ��
	float out;
	float P;
	float I;
	float D;
}pid;


//�����ĳ�ʼ��	
void Key_Init(void)
{
	RCC->APB2ENR|=1<<3;						//ʹ��Bʱ��	
	RCC->APB2ENR|=1<<4;						//ʹ��Cʱ��	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);     //����JTAG
	GPIOB->CRL&=0x0000FFFF;				//��ʼ��B4��B7
	GPIOB->CRL|=0x88880000;				//B4-B7����Ϊ��������
	GPIOB->ODR|=0xf<<4;							//B4-B7����Ϊ��������
	
	GPIOC->CRL&=0xFFFF0000;					//��ʼ��PC0-3
	GPIOC->CRL|=0x00003333;				  //PC0-3����Ϊ�������
	GPIOC->ODR|=0x00;							  
}

u8 Key_Scan(void)
{
		char keyValue=16;//����ֵ
		GPIOC->ODR&=0x0000;
		GPIOC->ODR|=0x0007;			//��PC0~3���0111
      switch(GPIOB->IDR & 0x00f0)
        {
            case 0x0070 : keyValue=15;break;
            case 0x00b0 : keyValue=14;break;
            case 0x00d0 : keyValue=13;break;
            case 0x00e0 : keyValue=12;break;  
				}
			GPIOC->ODR&=0x0000;
			GPIOC->ODR|=0x000b;			
		//GPIOC->ODR & 0x0000| 0x000b;//��PC0~3�������������
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
		//GPIOC->ODR & 0x0000| 0x000d;//��PC0~3��������ڶ���
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
    //GPIOC->ODR & 0x0000| 0x000e;//��PC0~3���������һ��
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


//���ƺ���
void SetRightShift()
{
		switch (SetIndex)
		{
			case 0: SetIndex=1;  break;			//������ʾ
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
	if(Keycode<10)			//������ֵ
	{
		change(Keycode,SetIndex);
	}
	if(Keycode==10)			
	{
		m_printf("%d\n",10);
		SetRightShift();				//��������
		Esc=1;
		keytemp=0;									//ȡ����
	}
	else if(Keycode==11)			//eepromд
	{
	}
	else if(Keycode==12)			//eeprom��
	{
	}
	else if(Keycode==13)																		//�л�ģʽ��MODE��
	{
		printf("kp:%.1f\n",pid.Kp);
		printf("ki:%.1f\n",pid.Ki);
		printf("kd:%.1f\n",pid.Kd);
		printf("spd:%.1f\n",pid.SetSpeed);
		/*
		Angle=0;
		pid.Kp=0.355;																					//�Ƕ�pid����
		pid.Ki=0.4;
		pid.Kd=2.0;
		*/
	}
	else if(Keycode==14)
	{
		Esc=1;						//ȡ����
		m_printf("%d\n",14);
	}
	else if(Keycode==15)
	{
		m_printf("%d\n",15);//ȷ�ϼ�	
		switch (SetIndex)
		{
			case 1:	s_spd=keytemp; Esc=1;SetIndex=0;keytemp1[0]=keytemp*10;AT24CXX_Write(30,(u8*)keytemp1,1);Read_index=1;LCD_ShowNum1(30,60,pid.SetSpeed,4,BLACK);break;//�趨�ٶ�
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




