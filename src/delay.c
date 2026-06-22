#include "delay.h" 
 
static u8    fac_us=0;    
  
// us delay factor 
 
void delay_init(u8 SYSCLK) 
{ 
SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
fac_us = SYSCLK / 8; 
} 
 
void delay_s(u8 n)  
  
  
// delay n seconds, n should be an int 
{ 
    while (n --)
    {
        delay_ms(1000);
        /* code */
    }
    
} 
 
void delay_ms(u16 n)   
// delay n ms, n should be an int 
{
    while ( n --)
    {
        delay_us(1000);
    }
} 
void delay_us(u32 n)    
// delay n us, n should be an int 
{ 
u32 temp; 
SysTick->LOAD = n * fac_us; 
SysTick->VAL = 0x00; 
SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; 
do 
{ 
  
temp = SysTick->CTRL; 
}while((temp & 0x01) && !(temp & (1<<16))); 
SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; 
SysTick->VAL = 0x00; 
} 