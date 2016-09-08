
#include "jumper.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"

void initGPIO_JMP(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
* 1 - dynamic IP, 0 - static IP
*/
char checkJMP1(void)
{
   if(GPIOE->IDR & GPIO_Pin_2) return 1;
   else return 0;
}

/**
* swap direction for transfer from LAN A->b(1) or LAN B->a(0)
*/
char checkJMP2(void)
{
   if(GPIOE->IDR & GPIO_Pin_3) return 1;
   else return 0;
}

/**
* 0 - DEBUG MODE
*/
char checkJMP3(void)
{
   if(GPIOE->IDR & GPIO_Pin_4) return 1;
   else return 0;
}

