#include "OLED.h"
#include "OLED_Font.h"

/*引脚配置*/
void OLED_Write_Cmd(uint8_t cmd) {
    I2C_Start();
    I2C_SendByte(0x78); // OLED 地址
    I2C_SendByte(0x00); // 0x00 表示指令
    I2C_SendByte(cmd);
    I2C_Stop();
}

void OLED_Write_Data(uint8_t data) {
    I2C_Start();
    I2C_SendByte(0x78); // OLED 地址
    I2C_SendByte(0x40); // 0x40 表示数据
    I2C_SendByte(data);
    I2C_Stop();
}

void OLED_Set_Pos(u8 x, u8 y) {
    OLED_Write_Cmd(0xb0 + y);
    OLED_Write_Cmd(((x & 0xf0) >> 4) | 0x10);
    OLED_Write_Cmd((x & 0x0f) | 0x01);
}

void OLED_Clear(void) {
    u8 i, n;
    for (i = 0; i < 8; i++) {
        OLED_Write_Cmd(0xb0 + i);
        OLED_Write_Cmd(0x00);
        OLED_Write_Cmd(0x10);
        for (n = 0; n < 128; n++) OLED_Write_Data(0);
    }
}

void OLED_Init(void) {
    // 1. GPIO Init is assumed to be done by MPU6050_Init 
    // If not, call your GPIO config here.
    
    delay_ms(100); // Wait for OLED stable
    
    OLED_Write_Cmd(0xAE); // Display OFF
    OLED_Write_Cmd(0x00); // Set low column address
    OLED_Write_Cmd(0x10); // Set high column address
    OLED_Write_Cmd(0x40); // Set start line address
    OLED_Write_Cmd(0x81); // Set contrast control register
    OLED_Write_Cmd(0xCF);
    OLED_Write_Cmd(0xA1); // Set segment re-map 0 to 127
    OLED_Write_Cmd(0xC8); // Set COM Output Scan Direction
    OLED_Write_Cmd(0xA6); // Set normal display
    OLED_Write_Cmd(0xA8); // Set multiplex ratio(1 to 64)
    OLED_Write_Cmd(0x3F);
    OLED_Write_Cmd(0xD3); // Set display offset
    OLED_Write_Cmd(0x00);
    OLED_Write_Cmd(0xD5); // Set display clock divide ratio/oscillator frequency
    OLED_Write_Cmd(0x80);
    OLED_Write_Cmd(0xD9); // Set pre-charge period
    OLED_Write_Cmd(0xF1);
    OLED_Write_Cmd(0xDA); // Set com pins hardware configuration
    OLED_Write_Cmd(0x12);
    OLED_Write_Cmd(0xDB); // Set vcomh
    OLED_Write_Cmd(0x40);
    OLED_Write_Cmd(0x8D); // Set Charge Pump enable/disable
    OLED_Write_Cmd(0x14);
    OLED_Write_Cmd(0xAF); // Display ON
    
    OLED_Clear();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_Write_Cmd(0xB0 | Y);					//设置Y位置
	OLED_Write_Cmd(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_Write_Cmd(0x00 | (X & 0x0F));			//设置X位置低4位
}
/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_Write_Data(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_Write_Data(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}
