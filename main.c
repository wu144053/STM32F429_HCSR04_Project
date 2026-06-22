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
#include "uart.h"

/** @addtogroup Template
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void UART1_SendU32(u32 value);
static void Security_SendDistance(u32 distance_x10_cm, u8 alarm_on);

/* Private functions ---------------------------------------------------------*/
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

static void Security_SendDistance(u32 distance_x10_cm, u8 alarm_on)
{
  UART1_SendString("Distance: ");
  UART1_SendU32(distance_x10_cm / 10U);
  UART1_SendByte('.');
  UART1_SendU32(distance_x10_cm % 10U);
  UART1_SendString(" cm, ALARM: ");
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
  u8 alarm_on = 0;

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f429_439xx.s) before to branch to application main. 
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */  
  
  delay_init(180);
  UART1_Init(115200);
  HCSR04_Init();
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDOff(LED4);

  UART1_SendString("HC-SR04 security demo start\r\n");
    
  /* Infinite loop */
  while (1)
  {
    if (HCSR04_ReadDistanceX10(&distance_x10_cm))
    {
      alarm_on = (distance_x10_cm <= 300U);

      if (alarm_on)
      {
        STM_EVAL_LEDOn(LED4);
      }
      else
      {
        STM_EVAL_LEDOff(LED4);
      }

      Security_SendDistance(distance_x10_cm, alarm_on);
    }
    else
    {
      STM_EVAL_LEDOff(LED4);
      UART1_SendString("Distance: timeout, ALARM: OFF\r\n");
    }

    delay_ms(200);
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
