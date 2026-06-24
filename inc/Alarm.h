#ifndef _ALARM_H
#define _ALARM_H
#include "stm32f4xx.h"
#include "buzzer.h"

//#private function define
void AlarmLED_Init(void);
void AlarmLED_Set(u8 is_on);
void Alarm_Set(u8 is_on);
void ALarm_init(void);
#endif 