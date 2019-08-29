#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/07
//�汾��V1.2
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************
//V1.1 20140306 
//����TIM1_CH1��PWM�������������� 
//V1.2 20140307
//����TIM2���벶���ʼ������TIM2_Cap_Init�����жϴ���
////////////////////////////////////////////////////////////////////////////////// 	  
 
//ͨ���ı�TIM1->CCR1��ֵ���ı�ռ�ձ�
#define PWM_VAL TIM1->CCR1   

void L298N_Init(void);

void TIM4_Int_Init(u16 arr,u16 psc);

void TIM5_Int_Init(u16 arr,u16 psc);

void TIM6_Int_Init(u16 arr,u16 psc);

void Encoder_Init_TIM2(void);

void TIM1_PWM_Init(u16 arr,u16 psc);

void TIM3_Int_Init(u16 arr,u16 psc);

void SetRightShift(void);

void Refresh(void);					

void Setpid(u8 *USART_RX_BUF);	

#endif

























