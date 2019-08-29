#ifndef _PID__H
#define _PID__H

#include "sys.h"


void PID_init(void);
float PID_realize(float speed);
extern struct _pid pid;
float PID_Angle_realize(float angle);

//#define pwmval TIM1->CCR1;


#endif

