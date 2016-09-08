
#include "flash.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx.h"

#define EEPROM_HW_ADDRESS      0xA0
#define TRUE         1
#define FALSE        0

extern void delay_us(unsigned int us);

CFlash::CFlash()
{

}

CFlash::~CFlash()
{

}

void CFlash::onInit(void)
{
   I2C_DeInit(I2C1);
   I2C_InitTypeDef i2c;

   GPIO_InitTypeDef GPIO_InitStruct;
   /*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin alternate function */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);		// SCL

	/*Configure GPIO pin alternate function */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);		// SDA

	i2c.I2C_ClockSpeed = 100000;
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_OwnAddress1 = 0x0A;
	i2c.I2C_Ack = I2C_Ack_Enable;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &i2c);

	I2C_Cmd(I2C1, ENABLE);
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}

uint8_t CFlash::onReceiveByte(uint8_t Address, uint16_t Register)
{
    uint8_t tmp;
        /* While the bus is busy */
    //while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    delay_us(10);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Transmitter);
    delay_us(10);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


    /* Send the EEPROM's internal address to read from: MSB of the address first */
    I2C_SendData(I2C1, (uint8_t)(Register & 0x00FF));
    delay_us(10);

    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);
    delay_us(10);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS | 0x01, I2C_Direction_Receiver);
    delay_us(10);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));

    tmp = I2C_ReceiveData(I2C1);
    delay_us(100);

    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C1, ENABLE);
    delay_us(1000);

    return tmp;
}

//char CFlash::onStartI2C(void)
//{
//   char res = TRUE;
//   uint32_t i = 0;
//   delay_us(10);
//   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
//   {
//      if(++i == 0xFFFFFFFF)
//      {
//         res = FALSE;
//         break;
//      };
//   };
//   if(res == TRUE) I2C_GenerateSTART(I2C1, ENABLE);
//   delay_us(10);
//   while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
//   {
//      if(++i == 0xFFFFFFFF)
//      {
//         res = FALSE;
//         break;
//      };
//   };
//   if(res == FALSE) I2C_GenerateSTOP(I2C1, ENABLE);
//   return res;                          // старт успешно сформирован
//}
//
//void CFlash::onStopI2C(void)
//{
//   I2C_GenerateSTOP(I2C1, ENABLE);
//}


