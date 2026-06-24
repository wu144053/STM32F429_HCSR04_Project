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
#include "Alarm.h"
#include "OLED.h"
#include "RFID.h"
#include "delay.h"
#include "door_status.h"
#include "hcsr04.h"
#include "math.h"
#include "mpu6050.h"
#include "pir.h"
#include "stm32f4xx.h"
#include "timer.h"
#include "uart.h"

/* --- 宏定义 --- */
#define ALARM_DURATION_SEC 3  // 报警持续时间
#define AUTH_WINDOW_SEC    5  // 刷卡授权窗口期
#define ALARM_LED_ON_STATION_TIME 5

/* --- 全局状态变量 (增加 volatile 保证中断同步) --- */
volatile u8 is_authorized = 0;    // 0:未授权, 1:已授权
volatile u8 auth_timer_flag = 0;  // 授权计时开启标志
volatile u32 auth_timer_count = 0;// 授权计时计数

volatile u8 alarm_on = 0;         // 0:停止报警, 1:开启报警
volatile u8 alarm_time_flag = 0;  // 人体感应报警标志
volatile u8 alarm_time_count = 0; // 人体感应报警计数

volatile u8 alarm_doors_flag = 0; // 门窗异常报警标志
volatile u8 alarm_door_count = 0; // 门窗报警计数

u8 door_check_cycle = 0;          // 姿态检测周期计数
u8 OLED_fresh_flag = 0;           // OLED 刷新准许标志

/* --- 私有函数声明 --- */
static void System_Init(void);
static void Display_Refresh(u32 dist, u8 pir, u8 auth, u8 door_alarm);

int main(void)
{
    u32 distance_x10_cm = 0;
    u8 pir_detected = 0;
    u8 msc_id;
   // u8 card_id[5];

    // 系统硬件初始化
    System_Init();
    MPU6050_Init(&msc_id);
    ///OLED_Clear();

    while (1)
    {
        // // 1. RFID 刷卡检测逻辑
        // if (RC522_CheckCard(card_id))
        // {
        //     is_authorized = 1;
        //     auth_timer_count = 0; // 重置计时器
        //     auth_timer_flag = 1;  
        //     alarm_on = 0;         // 刷卡消警
        //     alarm_doors_flag = 0;
        //     UART1_SendString("Card Authorized! Access Granted.\n");
        // }

        //2. 人体感应与测距报警逻辑
        pir_detected = PIR_IsDetected();
        if (HCSR04_ReadDistanceX10(&distance_x10_cm)) 
        {
            // 如果检测到人且距离在范围内，且当前没在报警
            if (pir_detected && alarm_time_flag == 0)
            {
                alarm_time_flag = 1; // 触发定时器倒计时报警
                UART1_SendString("Alert: Human Detected!\n");
            }
        }

        float yaw_rate = MPU6050_Get_Yaw_Rate();
        // 3. 门窗姿态检测逻辑 (每 10 次循环检测一次，降低 I2C 总线压力)
        if (++door_check_cycle >= 10)
        {
            door_check_cycle = 0;
            // 调用你原有的 door_status 处理逻辑
            if (_Door_Status(1)) 
            {
                alarm_doors_flag = 1; // 触发门窗异常报警
                UART1_SendString("Warning: Illegal Door Opening!\n");
              
            }
        }
  UART1_SendFloat(yaw_rate, 2);
        // 4. 报警执行逻辑
        // 只要人体报警或门窗报警有一个为1，就响铃/亮灯
        alarm_on = (alarm_time_flag || alarm_doors_flag);
        Alarm_Set(alarm_on);

        // 5. OLED 界面显示更新 (由定时器触发 OLED_fresh_flag)
        if (OLED_fresh_flag == 0)
        {
            //Display_Refresh(distance_x10_cm, pir_detected, is_authorized, alarm_doors_flag);
            OLED_fresh_flag = 1; // 等待定时器下次清零
        }
        
        delay_ms(10); // 短暂延时，提高刷卡灵敏度
    }
}

/* --- 硬件初始化汇总 --- */
static void System_Init(void) {
    delay_init(180);
    UART1_Init(115200);
    HCSR04_Init();
    TIM3_OneSecond_Init(); // 1秒中断一次
    PIR_Init();
    ALarm_init();
    //OLED_Init();
    RC522_Init();
}

/* --- OLED 刷新显示优化 --- */
static void Display_Refresh(u32 dist, u8 pir, u8 auth, u8 door_alarm) {
    // 第一行：显示门状态
    OLED_ShowString(1, 1, door_alarm ? "DOOR: ALARM" : "DOOR: SAFE ");
    
    // 第二行：距离信息
    OLED_ShowString(2, 1, "Dist:");
    OLED_ShowNum(2, 7, dist / 10, 3); // 显示厘米
    OLED_ShowString(2, 11, "cm");

    // 第三行：人体感应状态
    OLED_ShowString(3, 1, pir ? "BODY: YES " : "BODY: NONE");

    // 第四行：授权状态
    OLED_ShowString(4, 1, auth ? "MODE: AUTH" : "MODE: SECURE");
}
#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
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
        // 允许主循环刷新 OLED
        OLED_fresh_flag = 0;

        // 1. 处理人体感应报警倒计时
        if (alarm_time_flag == 1)
        {
            alarm_time_count++;
            if (alarm_time_count >= ALARM_LED_ON_STATION_TIME)
            {
                alarm_time_count = 0;
                alarm_time_flag = 0; // 报警时间结束
            }
        }

        // 2. 处理门窗异常报警倒计时
        if (alarm_doors_flag == 1)
        {
            alarm_door_count++;
            if (alarm_door_count >= ALARM_LED_ON_STATION_TIME)
            {
                alarm_door_count = 0;
                alarm_doors_flag = 0; // 门窗报警结束
            }
        }

        // 3. 处理刷卡授权时间窗口 (5秒)
        if (auth_timer_flag == 1)
        {
            auth_timer_count++;
            if (auth_timer_count >= 5) 
            {
                auth_timer_count = 0;
                auth_timer_flag = 0;
                is_authorized = 0; // 授权过期，恢复设防状态
            }
        }

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}