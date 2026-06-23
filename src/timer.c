#include "stm32f4xx.h"

volatile uint8_t one_second_flag = 0; // 一秒到达标志位

// 初始化 TIM3 为一秒中断一次
void TIM3_OneSecond_Init(void)
{
  TIM_TimeBaseInitTypeDef tim_init_struct;
  NVIC_InitTypeDef nvic_init_struct;
  RCC_ClocksTypeDef rcc_clocks;

  // 1. 开启 TIM3 时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // 2. 动态计算 APB1 时钟频率
  RCC_GetClocksFreq(&rcc_clocks);
  uint32_t apb1_timer_clk = rcc_clocks.PCLK1_Frequency;
  if (RCC->CFGR & RCC_CFGR_PPRE1) {
    apb1_timer_clk *= 2;
  }

  // 3. 配置 TIM3 定时参数
  // 我们使用 10000 分频，把时钟频率拉低。
  // 目标溢出频率为 1Hz (1s)
  tim_init_struct.TIM_Prescaler = 10000 - 1; 
  tim_init_struct.TIM_Period = (apb1_timer_clk / 10000) - 1; // 1秒溢出一次
  tim_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;
  tim_init_struct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &tim_init_struct);

  // 4. 清除挂起中断并使能更新（溢出）中断
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  // 5. 配置中断向量表 NVIC
  nvic_init_struct.NVIC_IRQChannel = TIM3_IRQn;
  nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级（略低于超声波）
  nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
  nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic_init_struct);

  // 6. 开启 TIM3
  TIM_Cmd(TIM3, ENABLE);
}

// // TIM3 中断服务函数
// void TIM3_IRQHandler(void)
// {
//   if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
//   {
//     one_second_flag = 1; // 置位一秒标志
//     TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志
//   }
// }