#include "uart.h"

#define UART1_GPIO_PORT GPIOA
#define UART1_TX_PIN    GPIO_Pin_9
#define UART1_RX_PIN    GPIO_Pin_10
#define UART1_AF        GPIO_AF_USART1

void UART1_Init(u32 baudrate)
{
  GPIO_InitTypeDef gpio_init_struct;
  USART_InitTypeDef usart_init_struct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  GPIO_PinAFConfig(UART1_GPIO_PORT, GPIO_PinSource9, UART1_AF);
  GPIO_PinAFConfig(UART1_GPIO_PORT, GPIO_PinSource10, UART1_AF);

  gpio_init_struct.GPIO_Pin = UART1_TX_PIN | UART1_RX_PIN;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_OType = GPIO_OType_PP;
  gpio_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(UART1_GPIO_PORT, &gpio_init_struct);

  USART_StructInit(&usart_init_struct);
  usart_init_struct.USART_BaudRate = baudrate;
  usart_init_struct.USART_WordLength = USART_WordLength_8b;
  usart_init_struct.USART_StopBits = USART_StopBits_1;
  usart_init_struct.USART_Parity = USART_Parity_No;
  usart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &usart_init_struct);

  USART_Cmd(USART1, ENABLE);
}

void UART1_SendByte(u8 data)
{
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {
  }

  USART_SendData(USART1, data);
}

void UART1_SendString(const char *str)
{
  while (*str != '\0')
  {
    UART1_SendByte((u8)*str);
    str++;
  }
}

u8 UART1_ReadByte(u8 *data)
{
  if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
  {
    *data = (u8)USART_ReceiveData(USART1);
    return 1;
  }

  return 0;
}
