#include "pir.h"

#define PIR_GPIO_CLK   RCC_AHB1Periph_GPIOC
#define PIR_GPIO_PORT  GPIOC
#define PIR_GPIO_PIN   GPIO_Pin_4

void PIR_Init(void)
{
  GPIO_InitTypeDef gpio_init_struct;

  RCC_AHB1PeriphClockCmd(PIR_GPIO_CLK, ENABLE);

  gpio_init_struct.GPIO_Pin = PIR_GPIO_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_OType = GPIO_OType_PP;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(PIR_GPIO_PORT, &gpio_init_struct);
}

u8 PIR_IsDetected(void)
{
  return (GPIO_ReadInputDataBit(PIR_GPIO_PORT, PIR_GPIO_PIN) == Bit_SET) ? 1U : 0U;
}
