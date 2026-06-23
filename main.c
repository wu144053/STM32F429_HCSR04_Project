/**
  ******************************************************************************
  * @file    Template/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "delay.h"
#include "hcsr04.h"
#include "pir.h"
#include "uart.h"
#include "timer.h"

/** @addtogroup Template
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ALARM_LED_GPIO_CLK  RCC_AHB1Periph_GPIOG
#define ALARM_LED_GPIO_PORT GPIOG
#define ALARM_LED_PIN       GPIO_Pin_14

#define ALARM_DISTANCE_X10_CM 300U
#define ALARM_LED_ON_STATION_TIME 3 //alarm station time s

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 alarm_time_flag; //start to time for led_on statio when check the event happen and the led not on;ture = 1 ; flase = 0;
u8 alarm_time_count; // the cout of time cycyle for led on
u8 alarm_led_on_flag;//the flag of led on 
u8 alarm_on = 0;
/* Private function prototypes -----------------------------------------------*/
static void AlarmLED_Init(void);
static void AlarmLED_Set(u8 is_on);
static void UART1_SendU32(u32 value);
static void Security_SendStatus(u32 distance_x10_cm, u8 pir_detected, u8 alarm_on);

/* Private functions ---------------------------------------------------------*/
static void AlarmLED_Init(void)
{
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

static void AlarmLED_Set(u8 is_on)
{
  if (is_on)
  {
    GPIO_SetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
  }
  else
  {
    GPIO_ResetBits(ALARM_LED_GPIO_PORT, ALARM_LED_PIN);
  }
}

static void UART1_SendU32(u32 value)
{
  char buffer[10];
  u8 index = 0;

  if (value == 0)
  {
    UART1_SendByte('0');
    return;
  }

  while (value > 0)
  {
    buffer[index++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while (index > 0)
  {
    UART1_SendByte((u8)buffer[--index]);
  }
}

static void Security_SendStatus(u32 distance_x10_cm, u8 pir_detected, u8 alarm_on)
{
  UART1_SendString("Distance: ");
  UART1_SendU32(distance_x10_cm / 10U);
  UART1_SendByte('.');
  UART1_SendU32(distance_x10_cm % 10U);
  UART1_SendString(" cm, PIR: ");
  UART1_SendString(pir_detected ? "BODY" : "NONE");
  UART1_SendString(", ALARM: ");
  UART1_SendString(alarm_on ? "ON\r\n" : "OFF\r\n");
}

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  u32 distance_x10_cm = 0;
  u8 pir_detected = 0;
  u8 alarm_flag_new = 0,alarm_flag_old = 0,alarm_flag = 0;

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f429_439xx.s) before to branch to application main. 
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */  
  
  delay_init(180);
  UART1_Init(115200);
  HCSR04_Init();
  TIM3_OneSecond_Init();
  PIR_Init();
  AlarmLED_Init();
  

  UART1_SendString("HC-SR04 + HC-SR501 security demo start\r\n");
  

  /* Infinite loop */
  while (1)
  {
      pir_detected = PIR_IsDetected();
     // UART1_SendByte(pir_detected);
    
    if (HCSR04_ReadDistanceX10(&distance_x10_cm)&&alarm_time_flag == 0)//justice when the distance small than distance and the led_on time is not over
    {
      alarm_flag_old = alarm_flag_new ;
      alarm_flag_new = pir_detected ;
      if(alarm_flag_new != alarm_flag_old){
        alarm_flag = pir_detected;
        if (alarm_flag == 1) alarm_time_flag = 1; // trig the TIM count for led alarm on
      }else { 
        alarm_flag = 0;
      } 
      Security_SendStatus(distance_x10_cm, pir_detected, alarm_on);
    }
    else
    {
      pir_detected = PIR_IsDetected();
      UART1_SendString("Distance: timeout, PIR: ");
      UART1_SendString(pir_detected ? "BODY" : "NONE");
      UART1_SendString(", ALARM: OFF\r\n");
    }
    AlarmLED_Set(alarm_on);
    //delay_ms(100);
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
// TIM3 中断服务函数

void TIM3_IRQHandler(void)
{
  
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    if(alarm_time_flag == 1){
      alarm_time_count++;
      if(alarm_time_count >= ALARM_LED_ON_STATION_TIME){
        alarm_time_count = 0;
        alarm_time_flag = 0;
        alarm_on = 0; // turn down led when the time over 
      }
      else {
        alarm_on = 1; //turn on led when the alarm_tieme_flag= 1;
      }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志
  }
}