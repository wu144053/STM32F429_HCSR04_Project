#include "buzzer.h"

/**
 * @brief init buzzer
 *
 */
void buzzer_intit()
{
    GPIO_InitTypeDef gpio_init_struct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    gpio_init_struct.GPIO_Pin = BUZZER_GPIO_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(BUZZER_GPIO_PORT, &gpio_init_struct);
    GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

void buzzer_turn(u8 buzzer_on){
    if(buzzer_on){
        GPIO_SetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
    }else{
        GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
    }
}
