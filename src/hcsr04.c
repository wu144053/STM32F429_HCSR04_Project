#include "hcsr04.h"
#include "delay.h"

#define HCSR04_TRIG_GPIO_CLK  RCC_AHB1Periph_GPIOB
#define HCSR04_TRIG_GPIO_PORT GPIOB
#define HCSR04_TRIG_PIN       GPIO_Pin_0

#define HCSR04_ECHO_GPIO_CLK  RCC_AHB1Periph_GPIOB
#define HCSR04_ECHO_GPIO_PORT GPIOB
#define HCSR04_ECHO_PIN       GPIO_Pin_1

#define HCSR04_TIMEOUT_US     30000U

static u32 hcsr04_cycles_per_us;

static u32 HCSR04_DwtNow(void)
{
  return DWT->CYCCNT;
}

static u8 HCSR04_WaitEcho(BitAction state, u32 timeout_us)
{
  u32 start = HCSR04_DwtNow();
  u32 timeout_cycles = timeout_us * hcsr04_cycles_per_us;

  while (GPIO_ReadInputDataBit(HCSR04_ECHO_GPIO_PORT, HCSR04_ECHO_PIN) != (uint8_t)state)
  {
    if ((HCSR04_DwtNow() - start) > timeout_cycles)
    {
      return 0;
    }
  }

  return 1;
}

void HCSR04_Init(void)
{
  GPIO_InitTypeDef gpio_init_struct;

  RCC_AHB1PeriphClockCmd(HCSR04_TRIG_GPIO_CLK | HCSR04_ECHO_GPIO_CLK, ENABLE);

  gpio_init_struct.GPIO_Pin = HCSR04_TRIG_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_OType = GPIO_OType_PP;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(HCSR04_TRIG_GPIO_PORT, &gpio_init_struct);
  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);

  gpio_init_struct.GPIO_Pin = HCSR04_ECHO_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(HCSR04_ECHO_GPIO_PORT, &gpio_init_struct);

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  hcsr04_cycles_per_us = SystemCoreClock / 1000000U;
}

u8 HCSR04_ReadDistanceX10(u32 *distance_x10_cm)
{
  u32 echo_start;
  u32 echo_cycles;
  u32 echo_us;

  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);
  delay_us(2);
  GPIO_SetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);
  delay_us(10);
  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);

  if (!HCSR04_WaitEcho(Bit_SET, HCSR04_TIMEOUT_US))
  {
    return 0;
  }

  echo_start = HCSR04_DwtNow();

  if (!HCSR04_WaitEcho(Bit_RESET, HCSR04_TIMEOUT_US))
  {
    return 0;
  }

  echo_cycles = HCSR04_DwtNow() - echo_start;
  echo_us = echo_cycles / hcsr04_cycles_per_us;
  *distance_x10_cm = (echo_us * 10U) / 58U;

  return 1;
}
