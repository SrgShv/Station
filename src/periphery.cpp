
#include "stm32f4xx_rcc.h"
#include "periphery.h"

/**
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                       RCC_AHB1Periph_GPIOC |
                       RCC_AHB1Periph_GPIOD |
                       RCC_AHB1Periph_GPIOB,
                       ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

// Запускаем тактирование
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
**/

void PeripheryEnable(void)
{
   /* Enable or disable the AHB1 peripheral clock */
   RCC_AHB1PeriphClockCmd(
                        RCC_AHB1Periph_DMA1|
                        //RCC_AHB1Periph_DMA2|
                        RCC_AHB1Periph_GPIOA|
                        RCC_AHB1Periph_GPIOB|
                        RCC_AHB1Periph_GPIOC|
                        RCC_AHB1Periph_GPIOD|
                        RCC_AHB1Periph_GPIOE,
                        ENABLE);
	RCC_APB1PeriphClockCmd(
                        RCC_APB1Periph_USART2|
                        RCC_APB1Periph_SPI3|
                        RCC_APB1Periph_TIM2|
                        //RCC_APB1Periph_TIM3|
                        RCC_APB1Periph_I2C1|
                        RCC_APB1Periph_TIM6,
                        ENABLE);
   RCC_APB2PeriphClockCmd(
                        RCC_APB2Periph_SPI1|
                        RCC_APB2Periph_SYSCFG|
                        RCC_APB2Periph_USART1,
                        ENABLE);

}

void initLeds(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   /* Configure GPIO pin green Led */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	/* Configure GPIO pin red Led */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void setGreenLedON(void)
{
   GPIO_SetBits(GPIOE, GPIO_Pin_12);
}

void setGreenLedOFF(void)
{
   GPIO_ResetBits(GPIOE, GPIO_Pin_12);
}

void setRedLedON(void)
{
   GPIO_SetBits(GPIOE, GPIO_Pin_13);
}

void setRedLedOFF(void)
{
   GPIO_ResetBits(GPIOE, GPIO_Pin_13);
}

void InitSoundCtrlPin(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   /* Configure GPIO pin green Led */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

//	GPIO_SetBits(GPIOD, GPIO_Pin_9);
//	GPIO_ResetBits(GPIOD, GPIO_Pin_8);
}

void setAudioCommand(uint8_t cmmd)
{
   if(cmmd == 1)
   {
      GPIOD->BSRRL = GPIO_Pin_9;
      GPIOD->BSRRH = GPIO_Pin_8;
   }
   else if(cmmd == 2)
   {
      GPIOD->BSRRH = GPIO_Pin_9;
      GPIOD->BSRRL = GPIO_Pin_8;
   }
   else if(cmmd == 3)
   {
      GPIOD->BSRRL = GPIO_Pin_8;
      GPIOD->BSRRL = GPIO_Pin_9;
   }
   else
   {
      GPIOD->BSRRH = GPIO_Pin_8;
      GPIOD->BSRRH = GPIO_Pin_9;
   };
}

uint8_t CheckAudioBtn(void)
{
   uint8_t res = 0;

   if(GPIOD->IDR & GPIO_Pin_11) res |= 0x01;
   if(GPIOD->IDR & GPIO_Pin_10) res |= 0x02;

   return res;
}







