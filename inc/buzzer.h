#ifndef _BUZZER_H
#define _BUZZER_H
#include "stm32f4xx.h"

/*private define*/
#define  BUZZER_GPIO_PORT GPIOC
#define BUZZER_GPIO_PIN GPIO_Pin_3

/*private function */
void buzzer_turn(u8 buzzer_on);
void buzzer_intit();

#endif 