#ifndef __RC522_H
#define __RC522_H

#include "stm32f4xx.h"

// RC522 常用命令
#define PCD_IDLE              0x00               
#define PCD_AUTHENT           0x0E               
#define PCD_RECEIVE           0x08               
#define PCD_TRANSMIT          0x04               
#define PCD_TRANSCEIVE        0x0C               
#define PCD_RESETPHASE        0x0F               
#define PCD_CALCCRC           0x03               

// RC522 寄存器
#define CommandReg            0x01    
#define ComIEnReg             0x02    
#define ComIrqReg             0x04    
#define ErrorReg              0x06    
#define FIFODataReg           0x09
#define FIFOLevelReg          0x0A
#define ControlReg            0x0C
#define BitFramingReg         0x0D
#define ModeReg               0x11
#define TxControlReg          0x14
#define TxAutoReg             0x15
#define TModeReg              0x2A
#define TPrescalerReg         0x2B
#define TReloadRegH           0x2C
#define TReloadRegL           0x2D

// 函数声明
void RC522_Init(void);
uint8_t RC522_CheckCard(uint8_t *id); // 检查是否有卡，并返回ID
uint8_t Read_RC522(uint8_t addr) ;

#endif