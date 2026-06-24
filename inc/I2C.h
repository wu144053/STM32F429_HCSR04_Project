#ifndef _I2C_H_
#define  _I2C_H_
#include "stm32f4xx.h"
#include "delay.h"

/* private define*/
#define I2C_SCL_PIN         GPIO_Pin_1
#define I2C_SCL_PORT        GPIOC
#define I2C_SDA_PIN         GPIO_Pin_2
#define I2C_SDA_PORT        GPIOC

/* private function*/
void I2C_soft_init();
void I2C_Delay(void);
void SDA_IN(void) ;
void SDA_OUT(void) ;
void I2C_Start(void) ;
void I2C_Stop(void) ;
void I2C_SendByte(uint8_t byte) ;
uint8_t I2C_ReadByte(uint8_t ack) ;

#endif