#ifndef HCSR04_H
#define HCSR04_H

#include "stm32f4xx.h"

void HCSR04_Init(void);
u8 HCSR04_ReadDistanceX10(u32 *distance_x10_cm);

#endif
