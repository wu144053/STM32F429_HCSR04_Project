#include "Alarm.h"
#include "stm32f4xx.h"


/* privati Define  */
#define ALARM_LED_GPIO_CLK RCC_AHB1Periph_GPIOG
#define ALARM_LED_GPIO_PORT GPIOG
#define ALARM_LED_PIN GPIO_Pin_14

#define ALARM_DISTANCE_X10_CM 300U

void AlarmLED_Init(void){
    GPIO_InitTypeDef gpio_init_struct;

    RCC_AHB1PeriphClockCmd(ALARM_LED_GPIO_CLK, ENABLE);

    gpio_init_struct.GPIO_Pin = ALARM_LED_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(ALARM_LED_GPIO_PORT, &gpio_init_struct);

    AlarmLED_Set(0);
}

void AlarmLED_Set(u8 is_on)
{
    if (is_on)
    {
        GPIO_SetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
        buzzer_turn(is_on);
    }
    else
    {
        GPIO_ResetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
        buzzer_turn(is_on);
    }
}

void Alarm_Set(u8 is_on)
{
    if (is_on)
    {
        GPIO_SetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
        buzzer_turn(is_on);
    }
    else
    {
        GPIO_ResetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
        buzzer_turn(is_on);
    }
}

void ALarm_init(void)
{
    AlarmLED_Init();
    buzzer_intit();
}