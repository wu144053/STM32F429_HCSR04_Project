#ifndef delay_H
#define delay_H
#include "stm32f4xx.h"   
 
void delay_init(u8 SYSCLK); 
void delay_s(u8 ns); 
void delay_ms(u16 nms); 
void delay_us(u32 nus); 

#endif