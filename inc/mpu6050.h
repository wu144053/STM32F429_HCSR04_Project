#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"
#include "uart.h"
#include "delay.h"
#include "I2C.h"

/* I2C Pin Definitions - Modify according to your wiring */

/* MPU6050 Register Addresses */
#define MPU_ADDR            0xD0  // MPU6050 I2C Address (AD0 connected to GND)
#define MPU_PWR_MGMT_1      0x6B
#define MPU_SMPLRT_DIV      0x19
#define MPU_CONFIG          0x1A
#define MPU_GYRO_CONFIG     0x1B
#define MPU_ACCEL_CONFIG    0x1C
#define MPU_ACCEL_XOUT_H    0x3B
#define MPU_GYRO_XOUT_H     0x43
#define MPU_WHO_AM_I        0x75

/* Function Prototypes */
uint8_t MPU6050_Init(u8* date);
void MPU6050_Read_Raw(int16_t *accel, int16_t *gyro);
float MPU6050_Get_Yaw_Rate(void); // Used for door movement detection
u8 _Door_Status(u8 RFID_statue);
void MPU_Write_Reg(uint8_t reg, uint8_t data) ;
void MPU_Read_Bytes(uint8_t reg, uint8_t *buf, uint8_t len);

#endif