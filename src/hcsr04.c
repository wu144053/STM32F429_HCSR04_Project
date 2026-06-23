#include "hcsr04.h"
#include "delay.h"

#define HCSR04_TRIG_GPIO_CLK  RCC_AHB1Periph_GPIOB
#define HCSR04_TRIG_GPIO_PORT GPIOB
#define HCSR04_TRIG_PIN       GPIO_Pin_0

#define HCSR04_ECHO_GPIO_CLK  RCC_AHB1Periph_GPIOB
#define HCSR04_ECHO_GPIO_PORT GPIOB
#define HCSR04_ECHO_PIN       GPIO_Pin_1

#define HCSR04_TIMEOUT_US     30000U // 30ms 超时

// 获取 TIM2 计数器的当前值（单位：微秒）
static u32 HCSR04_TimerNow(void)
{
  return TIM2->CNT; // TIM2配置为了1us递增一次，所以直接返回 CNT 即可
}

// 高电平/低电平超时等待
static u8 HCSR04_WaitEcho(BitAction state, u32 timeout_us)
{
  u32 start = HCSR04_TimerNow();

  while (GPIO_ReadInputDataBit(HCSR04_ECHO_GPIO_PORT, HCSR04_ECHO_PIN) != (uint8_t)state)
  {
    // 32位定时器减法自带溢出保护，可安全处理越界问题
    if ((HCSR04_TimerNow() - start) > timeout_us)
    {
      return 0; // 超时退出
    }
  }

  return 1; // 等待成功
}

void HCSR04_Init(void)
{
  GPIO_InitTypeDef gpio_init_struct;
  TIM_TimeBaseInitTypeDef tim_init_struct;
  RCC_ClocksTypeDef rcc_clocks;

  // 1. 开启 GPIO 时钟
  RCC_AHB1PeriphClockCmd(HCSR04_TRIG_GPIO_CLK | HCSR04_ECHO_GPIO_CLK, ENABLE);

  // 2. 配置 Trig 引脚为推挽输出
  gpio_init_struct.GPIO_Pin = HCSR04_TRIG_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_OType = GPIO_OType_PP;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(HCSR04_TRIG_GPIO_PORT, &gpio_init_struct);
  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);

  // 3. 配置 Echo 引脚为浮空输入
  gpio_init_struct.GPIO_Pin = HCSR04_ECHO_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(HCSR04_ECHO_GPIO_PORT, &gpio_init_struct);

  // 4. 配置 TIM2 (用于 1us 级别测距计时)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // 动态计算 APB1 定时器的时钟频率
  RCC_GetClocksFreq(&rcc_clocks);
  uint32_t apb1_timer_clk = rcc_clocks.PCLK1_Frequency;
  if (RCC->CFGR & RCC_CFGR_PPRE1) {
    apb1_timer_clk *= 2; // 如果 APB1 分频系数不为1，定时器时钟等于 APB1 时钟的2倍
  }

  // 目标是将 TIM2 配置为 1MHz 计数（即 1us 递增一次）
  tim_init_struct.TIM_Prescaler = (apb1_timer_clk / 1000000U) - 1;
  tim_init_struct.TIM_Period = 0xFFFFFFFF; // 32位最大重装载值，防止短时间溢出
  tim_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;
  tim_init_struct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &tim_init_struct);

  TIM_SetCounter(TIM2, 0);
  TIM_Cmd(TIM2, ENABLE); // 开启 TIM2 开始计时
}

u8 HCSR04_ReadDistanceX10(u32 *distance_x10_cm)
{
  u32 echo_start;
  u32 echo_us;

  // 发送 10us 触发脉冲
  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);
  delay_us(2);
  GPIO_SetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);
  delay_us(10);
  GPIO_ResetBits(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_PIN);

  // 等待 Echo 引脚变高电平
  if (!HCSR04_WaitEcho(Bit_SET, HCSR04_TIMEOUT_US))
  {
    return 0;
  }

  echo_start = HCSR04_TimerNow();

  // 等待 Echo 引脚变低电平
  if (!HCSR04_WaitEcho(Bit_RESET, HCSR04_TIMEOUT_US))
  {
    return 0;
  }

  // 计算高电平持续时间 (CNT 差值即为微秒数)
  echo_us = HCSR04_TimerNow() - echo_start;
  *distance_x10_cm = (echo_us * 10U) / 58U;

  return 1;
}