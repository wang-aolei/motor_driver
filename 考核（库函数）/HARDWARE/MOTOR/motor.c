#include "motor.h"
#include "sys.h"

/*
u8 Motor_angle(u16 angle,float spd)
{
	float Encode;
	float buff=0;  
	buff=angle/360.0;						//角度所占的比例
	Encode=buff*220;					//角度对应的脉冲数
	
	return Encode;
}
*/

void Motor_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}
