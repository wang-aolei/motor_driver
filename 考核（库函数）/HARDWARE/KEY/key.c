#include "key.h"
#include "sys.h"
#include "led.h"

uint8_t  KeyCodeMap[4][4] = { 
    { 0x31, 0x32, 0x33, 0x26 }, 
    { 0x34, 0x35, 0x36, 0x25 }, 
    { 0x37, 0x38, 0x39, 0x28 }, 
    { 0x30, 0x1B, 0x0D, 0x27 }
	};
 uint8_t KeySta[4][4] = { 
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

//按键的初始化	
void Key_Init(void)
{
	RCC->APB1ENR|=1<<3;						//使能B时钟
	
	JTAG_Set(SWD_ENABLE);					//关闭JTAG,开启SWD
	
	GPIOB->CRL&=0x00000000;				//初始化B0—B7
	GPIOB->CRL|=0x88880000;				//B4-B7配置为上拉输入
	GPIOB->CRL|=0x00003333;				//B0-B3配置为推挽输出			
	GPIOB->ODR|=0x0;							//B0-B3配置为输出低电平
	GPIOB->ODR|=0xf<<4;						//B4-B7配置为上拉输入

}
	

void KeyScan(void)
{
	unsigned char i; 
	static unsigned char keyout=0;
	static unsigned char keybuf[4][4]={
        {0x00, 0x00, 0x00, 0x00},{0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00},{0x00, 0x00, 0x00, 0x00}
};

	keybuf[keyout][0] = (keybuf[keyout][0] << 1) |PBin(0);   //按键扫描，更新按键的状态
	keybuf[keyout][1] = (keybuf[keyout][1] << 1) |PBin(1);		
	keybuf[keyout][2] = (keybuf[keyout][2] << 1) |PBin(2);	
	keybuf[keyout][3] = (keybuf[keyout][3] << 1) |PBin(3);		
	
	for(i=0;i<4;i++)
	{
		if((keybuf[keyout][i]&0x0F)==0x00)
		{
			KeySta[keyout][i]=0;			//抬起
		}
		else if((keybuf[keyout][i]&0x0F)==0x0f)
		{
			KeySta[keyout][i]=1;			//按下
		}
	}
	keyout++;
	keyout=keyout&0x03;        //逢四变一
}		

void KeyAction(uint8_t keycode)
{
	if(keycode==0x31)
	{
		LED0=~LED0;
	}
	else if(keycode==0x27)
	{
		LED1=~LED1;
	}

}


/*
For(i=0;i<4;i++)
{
	if(i!=0)
	{
		PBout(i-1)=0;
	}
	PBout(i)=1;
}*/
void KeyDriver()
{
	uint8_t i,j;
	static uint8_t backup[4][4]={
		{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}
	}; 
	for(i=0;i<4;i++)
	{
		if(i!=0)
		{
			PBout(i-1)=0;
		}
		PBout(i)=1;
		for(j=0;j<4;j++)
		{
			if(backup[i][j]!=KeySta[i][j])
			{
				if(backup[i][j]==1)
				{
					KeyAction(KeyCodeMap[i][j]);
				}
				backup[i][j]=KeySta[i][j];
			}
		}
	}
}















