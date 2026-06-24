#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"

void UART1_Init(u32 baudrate);
void UART1_SendByte(u8 data);
void UART1_SendString(const char *str);
u8 UART1_ReadByte(u8 *data);
void UART1_SendU32(u32 value);
void UART1_SendFloat(float value, int decimal_places) ;

#endif
