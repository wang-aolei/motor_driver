#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

void Key_Init(void);
u8 Key_Scan(void);

void change(u8 keycode,u8 setindex);
void keyaction(char Keycode);

#endif

