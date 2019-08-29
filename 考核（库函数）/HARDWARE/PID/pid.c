#include "sys.h"
#include "pid.h"
#include "usart.h"

struct _pid
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

void PID_init()
{
		pid.SetSpeed=0.0;
		pid.ActualSpeed=0.0;
//		pid.SetAngle=360.0;			//��ʼ��Ϊ360��
//		pid.ActualAngle=0.0;
		pid.err=0.0;
		pid.err_last=0.0;
		pid.err_next=0.0;
														//�ٶ�pid����
		pid.Kp=0.95;
		pid.Ki=0.4;
		pid.Kd=0.2;
		/*
		pid.Kp=0.355;						//�Ƕ�pid����
		pid.Ki=0.4;
		pid.Kd=2.0;
		*/
}

float PID_realize(float speed)
{
	pid.SetSpeed=speed;
	pid.err=pid.SetSpeed-pid.ActualSpeed;
	
	pid.P=pid.Kp*(pid.err-pid.err_next);
	pid.I=pid.Ki*pid.err;
	pid.D=pid.Kd*(pid.err-2*pid.err_next+pid.err_last);
	
	pid.out+=pid.P+pid.I+pid.D;
	pid.err_last=pid.err_next;
	pid.err_next=pid.err;
	
	if(pid.out>=100)  pid.out=100;
	if(pid.out<=0)    pid.out=0;

	return pid.out;
}

/*
float PID_Angle_realize(float angle)													//ת���Ƕ�
{
	pid.SetAngle=angle;
	pid.err=pid.SetAngle-pid.ActualAngle;
	
	pid.P=pid.Kp*(pid.err-pid.err_next);
	pid.I=pid.Ki*pid.err;
	pid.D=pid.Kd*(pid.err-2*pid.err_next+pid.err_last);
	
	pid.out+=pid.P+pid.I+pid.D;
	pid.err_last=pid.err_next;
	pid.err_next=pid.err;
	
	if(pid.out>=100)  pid.out=100;
	if(pid.out<=0)    pid.out=0;

	return pid.out;
}
*/














