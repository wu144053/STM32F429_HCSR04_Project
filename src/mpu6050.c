#include "mpu6050.h"
#include <math.h>

/* Write a single register */
void MPU_Write_Reg(uint8_t reg, uint8_t data) {
    I2C_Start();
    I2C_SendByte(MPU_ADDR);
    I2C_SendByte(reg);
    I2C_SendByte(data);
    I2C_Stop();
}

/* Read multiple bytes from registers */
void MPU_Read_Bytes(uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C_Start();
    I2C_SendByte(MPU_ADDR);
    I2C_SendByte(reg);
    I2C_Start(); // Repeated Start
    I2C_SendByte(MPU_ADDR | 0x01); // Read mode
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = I2C_ReadByte(i < (len - 1));
    }
    I2C_Stop();
}

/**
 * @brief Initialize MPU6050
 * @return 0 for success, 1 for failure
 */
uint8_t MPU6050_Init(u8* date) {
    I2C_soft_init();
    // Initialize MPU6050 Registers
    MPU_Write_Reg(MPU_PWR_MGMT_1, 0x00);   // Wake up MPU6050
    delay_ms(100);
    MPU_Write_Reg(MPU_SMPLRT_DIV, 0x07);   // Sample rate = 1kHz
    MPU_Write_Reg(MPU_CONFIG, 0x06);       // Low pass filter
    MPU_Write_Reg(MPU_GYRO_CONFIG, 0x18);  // Gyro full scale: +/- 2000 deg/s
    MPU_Write_Reg(MPU_ACCEL_CONFIG, 0x01); // Accel full scale: +/- 2g

    // Check if sensor is alive
    uint8_t id = 0;
    MPU_Read_Bytes(MPU_WHO_AM_I, &id, 1);
    *date = id;
    if (id == 0x68) return 0;
    else return 1;
}

/**
 * @brief Read Raw Accel and Gyro data
 */
void MPU6050_Read_Raw(int16_t *accel, int16_t *gyro) {
    uint8_t buf[14];
    MPU_Read_Bytes(MPU_ACCEL_XOUT_H, buf, 14);

    // Convert bytes to 16-bit integers
    accel[0] = (int16_t)((buf[0] << 8) | buf[1]);  // Accel X
    accel[1] = (int16_t)((buf[2] << 8) | buf[3]);  // Accel Y
    accel[2] = (int16_t)((buf[4] << 8) | buf[5]);  // Accel Z
    
    gyro[0] = (int16_t)((buf[8] << 8) | buf[9]);   // Gyro X
    gyro[1] = (int16_t)((buf[10] << 8) | buf[11]); // Gyro Y
    gyro[2] = (int16_t)((buf[12] << 8) | buf[13]); // Gyro Z
}

/**
 * @brief Simplified function to detect door rotation (Yaw axis)
 * @return Z-axis angular velocity in deg/s
 */
float MPU6050_Get_Yaw_Rate(void) {
    int16_t accel[3], gyro[3];
    MPU6050_Read_Raw(accel, gyro);
    // Sensitivity for FS_SEL=3 (2000 deg/s) is 16.4 LSB/deg/s
    return (float)gyro[2] / 16.4f; 
}