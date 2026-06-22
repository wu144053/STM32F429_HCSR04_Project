#include "key.h"
#include "delay.h" 

void key_init(void)
{  
GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    // Initialize the key input system
}

u8 KEY_Scan(u8 mode)
{
    static u8 key_up = 1;
    // 标志着按键是否松开，0 表示按键未松开，1 表示松开
    if (mode)
        key_up = 1;
    // mode=0，表示不支持连按；mode=1，表示支持连按
    if (key_up && (GPIOA->IDR & 0x00000001) == 1)
    {
        delay_ms(10);
        // 按键防抖，防止 10ms 内的按键信号抖动
        key_up = 0;
        return 1;
    }
    else if ((GPIOA->IDR & 0x00000001) == 0)
        key_up = 1;
    return 0;
    // 返回 0，表示按键没有按下
}