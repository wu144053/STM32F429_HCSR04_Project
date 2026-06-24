#include "RFID.h"
#include "delay.h"

// SPI1 片选控制
#define RC522_CS_LOW()  GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define RC522_CS_HIGH() GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define RC522_RST_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define RC522_RST_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_0)

#define MAXRLEN 18
#define PICC_REQIDL    0x26               // 寻天线区内未进入休眠状态的卡
// 低层 SPI 通讯
uint8_t SPI1_WriteRead(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

void Write_RC522(uint8_t addr, uint8_t val) {
    RC522_CS_LOW();
    SPI1_WriteRead((addr << 1) & 0x7E); // 地址格式：0XXXXXX0
    SPI1_WriteRead(val);
    RC522_CS_HIGH();
}

uint8_t Read_RC522(uint8_t addr) {
    uint8_t val;
    RC522_CS_LOW();
    SPI1_WriteRead(((addr << 1) & 0x7E) | 0x80); // 地址格式：1XXXXXX0
    val = SPI1_WriteRead(0x00);
    RC522_CS_HIGH();
    return val;
}

// 初始化 RC522
void RC522_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    // 1. 开启时钟 (GPIOA, GPIOB, SPI1)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // 2. 配置 SPI1 引脚 (PA5, PA6, PA7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    // 3. 配置片选 PA4 和复位 PB0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 4. SPI1 配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // 必须小于 10MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);

    // 5. RC522 复位
    RC522_RST_HIGH();
    delay_ms(50);
    Write_RC522(CommandReg, PCD_RESETPHASE);
    delay_ms(50);
    
    // 配置定时器等
    Write_RC522(TModeReg, 0x8D);
    Write_RC522(TPrescalerReg, 0x3E);
    Write_RC522(TReloadRegL, 30);
    Write_RC522(TReloadRegH, 0);
    Write_RC522(TxAutoReg, 0x40);
    Write_RC522(ModeReg, 0x3D);
    
    // 开启天线
    uint8_t temp = Read_RC522(TxControlReg);
    if (!(temp & 0x03)) Write_RC522(TxControlReg, temp | 0x03);
}

// 1. 底层通讯函数：通过 FIFO 发送和接收数据
uint8_t PcdComMF522(uint8_t Command, uint8_t *pInData, uint8_t InLenByte, uint8_t *pOutData, uint32_t *pOutLenBit) {
    uint8_t status = 0, irqEn = 0x00, waitFor = 0x00, lastBits, n;
    uint32_t i;
    if (Command == PCD_AUTHENT) { irqEn = 0x12; waitFor = 0x10; }
    if (Command == PCD_TRANSCEIVE) { irqEn = 0x77; waitFor = 0x30; }
    Write_RC522(ComIEnReg, irqEn | 0x80);
    Read_RC522(ComIrqReg); // Clear IRQ
    Write_RC522(CommandReg, PCD_IDLE);
    Write_RC522(FIFOLevelReg, 0x80); // Flush FIFO
    for (i = 0; i < InLenByte; i++) Write_RC522(FIFODataReg, pInData[i]);
    Write_RC522(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE) Write_RC522(BitFramingReg, Read_RC522(BitFramingReg) | 0x80);
    i = 60000; // Timeout
    do {
        n = Read_RC522(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    Write_RC522(BitFramingReg, Read_RC522(BitFramingReg) & 0x7F);
    if (i != 0) {
        if (!(Read_RC522(ErrorReg) & 0x1B)) {
            status = 1; // Success
            if (n & irqEn & 0x01) status = 0;
            if (Command == PCD_TRANSCEIVE) {
                n = Read_RC522(FIFOLevelReg);
                lastBits = Read_RC522(ControlReg) & 0x07;
                if (lastBits) *pOutLenBit = (n - 1) * 8 + lastBits;
                else *pOutLenBit = n * 8;
                if (n == 0) n = 1;
                for (i = 0; i < n; i++) pOutData[i] = Read_RC522(FIFODataReg);
            }
        }
    }
    return status;
}

// 2. 寻卡函数 (Request)
uint8_t PcdRequest(uint8_t req_code, uint8_t *pTagType) {
    uint8_t status;
    uint32_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN];
    Write_RC522(BitFramingReg, 0x07);
    ucComMF522Buf[0] = req_code;
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
    if ((status == 1) && (unLen == 0x10)) {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    } else status = 0;
    return status;
}

// 3. 防冲突获取序列号 (Anticoll)
uint8_t PcdAnticoll(uint8_t *pSnr) {
    uint8_t status, i, snr_check = 0;
    uint32_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN];
    Write_RC522(BitFramingReg, 0x00);
    ucComMF522Buf[0] = 0x93; // PICC_ANTICOLL1
    ucComMF522Buf[1] = 0x20;
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);
    if (status == 1) {
        for (i = 0; i < 4; i++) {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i]) status = 0;
    }
    return status;
}
/**
 * @brief  检查是否有卡片靠近并读取ID
 * @param  id: 存储读到的4字节卡号
 * @return 1: 成功读到卡, 0: 未发现卡
 */
uint8_t RC522_CheckCard(uint8_t *id) {
    uint8_t TagType[2];
    
    // 第一步：寻卡 (REQA)
    if (PcdRequest(0x26, TagType) == 1) {
        // 第二步：防冲突，获取4字节序列号
        if (PcdAnticoll(id) == 1) {
            // 读到卡了！返回成功
            return 1;
        }
    }
    return 0; // 没读到卡
}