#include "I2C.h"
/* Internal Private Functions for Software I2C */
void I2C_soft_init(){
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Enable GPIO clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    // Configure GPIOs as Open-Drain Output for Software I2C
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN;
    GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
    GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);
}
void I2C_Delay(void) {
    volatile uint32_t i = 1000; 
    while(i--);
}

void SDA_IN(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void SDA_OUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN; // PC2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void I2C_Start(void) {
    GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN);
}

void I2C_Stop(void) {
    GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    I2C_Delay();
}

void I2C_SendByte(uint8_t byte) {
    SDA_OUT(); // 确保处于输出模式
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN);
        else GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN);
        I2C_Delay();
        GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
        I2C_Delay();
        GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN);
        byte <<= 1;
    }
    
    SDA_IN();   // 切换为输入模式，释放总线
    I2C_Delay();
    GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN); // SCL 高电平，读取应答
    I2C_Delay();
    GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN);
    SDA_OUT();  // 切换回输出模式
}

uint8_t I2C_ReadByte(uint8_t ack) {
    uint8_t byte = 0;
    
    SDA_IN(); // 【关键】读取前切换为输入模式
    
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
        I2C_Delay();
        if (GPIO_ReadInputDataBit(I2C_SDA_PORT, I2C_SDA_PIN)) byte++;
        GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN);
        I2C_Delay();
    }
    
    SDA_OUT(); // 【关键】读取完切换回输出模式，准备发应答位
    
    if (ack) GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    else GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN);
    
    GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN);
    
    return byte;
}