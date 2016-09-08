
#include "usart.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_usart.h"

#include "stdio.h"
#include "stdlib.h"

#define TRUE         1
#define FALSE        0

char txt[100];

extern void delay_us(unsigned int us);

uint8_t pMetricBuff[USART_MTR_BUFF_SZ];
volatile uint8_t MetricBuffFlag = FALSE;
volatile uint8_t ServoTxFlag = FALSE;
//volatile uint16_t g_counterTX = 0;
CUsart *gpUsart1 = 0;

uint8_t CheckMetricFlag(void)
{
   return MetricBuffFlag;
}

uint8_t *GetMetricDate(void)
{
   MetricBuffFlag = FALSE;
   return (uint8_t *)&(pMetricBuff[0]);
}

void copyMetricBuff(uint8_t *pSrcBff, uint8_t *pDstBff)
{
   if((pSrcBff != 0) && (pDstBff != 0))
   {
      for(uint16_t i=0; i<USART_MTR_BUFF_SZ; i++)
      {
         pDstBff[i] = pSrcBff[i];
      };
      //SWO_PrintString("copy Metrics\n");
   };
}

CUsart::CUsart() :
   pBffTX(0),
   pBffRX(0),
   counterTX(0),
   readTX(0),
   writeTX(0)
{
   pBffTX = new CBufferSpx();
   pBffRX = new CBufferSpx();
};

CUsart::~CUsart()
{
   if(pBffRX != 0) delete pBffRX;
   if(pBffTX != 0) delete pBffTX;
};

void CUsart::onInit(void)
{
   // Объявляем переменные
   GPIO_InitTypeDef gpio;
   USART_InitTypeDef usart;
   NVIC_InitTypeDef NVIC_InitStructure;

   // Инициализация нужных пинов контроллера, для USART1 –
   // PA9 и PA10
   GPIO_StructInit(&gpio);

   gpio.GPIO_Mode = GPIO_Mode_AF;
   gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
   gpio.GPIO_Speed = GPIO_Speed_50MHz;
   gpio.GPIO_OType = GPIO_OType_PP;
   gpio.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &gpio);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

   // А теперь настраиваем модуль USART
   USART_StructInit(&usart);
   usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   usart.USART_BaudRate = 19200; /** SPL - ERROR COMPUTE BRR !!!**/
   //usart.USART_BaudRate = 57600;
   usart.USART_WordLength = USART_WordLength_8b;
   usart.USART_StopBits = USART_StopBits_1;
   usart.USART_Parity = USART_Parity_No;
   usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_Init(USART1, &usart);

   /**
   Fck = 168000000/4 = 42000000Hz
   div = 42000000Hz/(16*19200) = 136.71875
   136 -> HEX 0x88
   0.71875*16 = 11.5; 12 -> HEX 0xC
   BRR = 0x88C
   **/
   USART1->BRR = 0x88C; // Baud Rate 19200 bps

   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   USART_Cmd(USART1, ENABLE);

   gpUsart1 = this;
};

void CUsart::enableUSART(void)
{
   // Включаем прерывания и запускаем USART
   //NVIC_EnableIRQ(USART1_IRQn);
   SWO_PrintString("Enable USART\n");
   // Включаем прерывание по окончанию передачи
   //USART_ITConfig(USART1, (USART_IT_TC | USART_IT_RXNE), ENABLE);
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   USART_ITConfig(USART1, USART_IT_TC, ENABLE);
//   delay_us(1000);
//   USART_SendData(USART1, 0xAA);
//   USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//   USART_ClearITPendingBit(USART1, USART_IT_TC);
//   SWO_PrintString("Enable USART B\n");
}

void CUsart::disableUSART(void)
{
   // Включаем прерывание по окончанию передачи
   USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
   USART_ITConfig(USART1, USART_IT_TC, DISABLE);
}

void CUsart::onSendTXD(uint8_t *date, uint16_t len)
{
   pBffTX->onAddDate(date, len);
}

void CUsart::onPutUSART(uint8_t charD)
{
   USART_ITConfig(USART1, USART_IT_TC, DISABLE);
   /** IF data is not transferred to the shift register (not ready) **/
   if((counterTX > 0) || (!(USART1->SR & 0x00000080)))
   {
      pBuffTX[writeTX] = charD;
      if(++writeTX >= USART_BFF_SZ) writeTX = 0;
      ++counterTX;
   }
   else
   {
      /* Transmit Data */
      USART1->DR = ((uint16_t)charD & (uint16_t)0x01FF);
   };
   USART_ITConfig(USART1, USART_IT_TC, ENABLE);
}

void CUsart::onCheckSendTX(void)
{
   if((0 < pBffTX->onCheck()) && (USART1->SR & 0x00000080))
   {
      uint16_t d = (uint16_t)pBffTX->onGetChar();
      USART_SendData(USART1, d);
   };
}

char CUsart::onCheckIsEmptyTX(void)
{
   if(USART1->SR & 0x00000040) return TRUE;
   else return FALSE;
}

void CUsart::onSendFromBuff(void)
{
   if(counterTX > 0)
   {
      /* Transmit Data */
      USART1->DR = ((uint16_t)pBuffTX[readTX] & (uint16_t)0x01FF);
      if(++readTX >= USART_BFF_SZ) readTX = 0;
      --counterTX;
   };
}

// Обработчик прерывания
void USART1_IRQHandler()
{
   /** If TX buffer - empty */
   if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
   {
      if(gpUsart1 != 0) gpUsart1->onSendFromBuff();
      USART_ClearITPendingBit(USART1, USART_IT_TC);
   };

   /** check if the USART1 receive interrupt flag was set */
	if(USART_GetITStatus(USART1, USART_IT_RXNE))
   {
      USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		uint8_t rxd = USART1->DR;
		if(gpUsart1 != 0) gpUsart1->pBffRX->onAddChar(rxd);
	};
}

CBufferSpx::CBufferSpx() :
   m_counter(0),
   m_write(0),
   m_read(0)
{
}

CBufferSpx::~CBufferSpx()
{
}

void CBufferSpx::onAddDate(uint8_t *date, uint16_t len)
{
   for(uint16_t i=0; i<len; i++)
   {
      if(m_counter < SPX_BFF_SZ)
      {
         m_Buff[m_write] = date[i];
         if(++m_write >= SPX_BFF_SZ) m_write = 0;
         ++m_counter;
      }
      else break;
   };
}

void CBufferSpx::onAddChar(uint8_t &date)
{
   if(m_counter < SPX_BFF_SZ)
   {
      m_Buff[m_write] = date;
      if(++m_write >= SPX_BFF_SZ) m_write = 0;
      ++m_counter;
   };
}

uint16_t CBufferSpx::onCheck(void)
{
   return m_counter;
}

uint8_t CBufferSpx::onGetChar(void)
{
   uint8_t res = 0;
   USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
   if(m_counter > 0)
   {
      res = m_Buff[m_read];
      if(++m_read >= SPX_BFF_SZ) m_read = 0;
      --m_counter;
   };
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   return res;
}

void CBufferSpx::onGetArray(uint8_t *date, uint16_t &len)
{
   len = m_counter;
   if(m_counter > 0)
   {
      for(uint16_t i=0; i<len; i++)
      {
         date[i] = m_Buff[m_read];
         if(++m_read >= SPX_BFF_SZ) m_read = 0;
         --m_counter;
      };
   };
}

CSpeexPack::CSpeexPack() :
   m_switchAB(0),
   m_cntA(0),
   m_cntB(0),
   m_flagA(0),
   m_flagB(0),
   m_NetPackCntr(0)
{
}

CSpeexPack::~CSpeexPack()
{
}

void CSpeexPack::onWriteChar(uint8_t date, uint16_t pos)
{
   if(pos == 0)
   {
      if(m_switchAB == 0) m_switchAB = 1;
      else m_switchAB = 0;
   };
   if(m_switchAB == 0)  // write to buffA
   {
      if(pos < SPX_PACK_SZ) m_PackA[pos] = date;
      if((pos + 1) == SPX_PACK_SZ) m_flagA = 1;
   }
   else                 // write to buffB
   {
      if(pos < SPX_PACK_SZ) m_PackB[pos] = date;
      if((pos + 1) == SPX_PACK_SZ) m_flagB = 1;
   };
}

uint8_t *CSpeexPack::onGetPack(uint16_t &len)
{
   uint8_t *pPtr = 0;
   len = 0;
   if(m_switchAB == 0)  // write to buffA
   {
      if(m_flagB == 1)
      {
         pPtr = (uint8_t *)(&m_PackB[0]);
         m_flagB = 0;
         m_cntB = 0;
         len = SPX_PACK_SZ;
      };
   }
   else                 // write to buffB
   {
      if(m_flagA == 1)
      {
         pPtr = (uint8_t *)(&m_PackA[0]);
         m_flagA = 0;
         m_cntA = 0;
         len = SPX_PACK_SZ;
      };
   };
   return pPtr;
}

char CSpeexPack::onAddPackToNetBuff(uint8_t *date, uint16_t len)
{
   char res = FALSE;
   uint16_t pos = m_NetPackCntr*SPX_PACK_SZ;
   for(uint16_t i=0; i<len; i++)
   {
      m_NetSpxPack[pos++] = date[i];
   };
   if(++m_NetPackCntr >= NET_SPX_PACK_SZ/SPX_PACK_SZ)
   {
      res = TRUE;
      m_NetPackCntr = 0;
   };
   return res;
}

uint8_t *CSpeexPack::onGetNetPack(uint16_t &len)
{
   len = NET_SPX_PACK_SZ;
   return (uint8_t *)(&m_NetSpxPack[0]);
}

CDoubleSpxBuffer::CDoubleSpxBuffer() :
   packCount(0),
   packRead(0),
   packWrite(0)
{
   for(uint16_t i=0; i<DBUFF_SZ; i++)
   {
      dbuffA[i][0] = 0;
      dbuffA[i][1] = 'S';
      dbuffA[i][2] = 'p';
      dbuffA[i][3] = 'x';
   };

   dbuffB[0] = 0;
   dbuffB[1] = 'S';
   dbuffB[2] = 'p';
   dbuffB[3] = 'x';
   for(uint16_t i=0; i<SPX_PACK_SZ; i++)
   {
      dbuffB[i+4] = 0;
   };

}

CDoubleSpxBuffer::~CDoubleSpxBuffer()
{

}

void CDoubleSpxBuffer::SetPack(uint8_t *pack)
{
   if(packCount < DBUFF_SZ)
   {
      uint8_t *pPtr = (uint8_t *)(&dbuffA[packWrite][4]);
      for(uint16_t i=0; i<SPX_PACK_SZ; i++)
      {
         pPtr[i] = pack[i];
      };
      if(++packWrite >= DBUFF_SZ) packWrite = 0;
      ++packCount;
   };
}

uint8_t *CDoubleSpxBuffer::GetPack(uint16_t &lenD)
{
   uint8_t *res = 0;
   lenD = SPX_PACK_SZ+4;
   if(packCount > 0)
   {
      res = (uint8_t *)(&dbuffA[packRead][0]);
      if(++packRead >= DBUFF_SZ) packRead = 0;
      --packCount;
   }
   else
   {
      res = (uint8_t *)(&dbuffB[0]);
   };
   return res;
}

/************************ CUsartMetric ****************************/
uint8_t pBffTX[USART_MTR_BUFF_SZ*2];
uint8_t pBffRX[USART_MTR_BUFF_SZ*2];

CUsartMetric::CUsartMetric()
{

}

CUsartMetric::~CUsartMetric()
{

}

void CUsartMetric::onInit(void)
{
   USART_InitTypeDef    USART_InitStruct;
   //------------------------------------
   GPIO_InitTypeDef gpio;
   GPIO_StructInit(&gpio);

   gpio.GPIO_Mode = GPIO_Mode_AF;
   gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
   gpio.GPIO_Speed = GPIO_Speed_50MHz;
   gpio.GPIO_OType = GPIO_OType_PP;
   gpio.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &gpio);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

   //====================================
   //Инициализация USART2
   USART_InitStruct.USART_BaudRate = 9600; /** SPL - ERROR COMPUTE BRR !!! **/

   USART_InitStruct.USART_WordLength = USART_WordLength_8b;
   USART_InitStruct.USART_StopBits = USART_StopBits_1;
   USART_InitStruct.USART_Parity = USART_Parity_No ;
   USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART2, &USART_InitStruct);

   /**
   Fck = 168000000/4 = 42000000Hz
   div = 42000000Hz/(16*9600) = 273.4375
   273 -> HEX 0x111
   0.4375*16 = 7; 7 -> HEX 0x7
   BRR = 0x1117
   **/
   USART2->BRR = 0x1117; /** Baud Rate 9600 bps **/

   USART_Cmd(USART2, ENABLE); //Включаем USART2


   //=====================================

   //USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

   // Настройка DMA1
   // Для USART2_RX нужно использовать связку Stream5/Channel4
	DMA_DeInit(DMA1_Stream5);
	DMA_InitTypeDef DMA_InitStructure;

	DMA_InitStructure.DMA_Channel            = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize         = USART_MTR_BUFF_SZ*2;         // размер массива

	DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;             // циклический режим DMA_Mode_Normal
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
   DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);     // Это регистр данных для USART2
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t)&(pBffRX[0]);	   // Указатель на массив
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);                             // применим настройки DMA
	DMA_Cmd(DMA1_Stream5, ENABLE);		                                    // Включение DMA

	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);

	// Для USART2_TX нужно использовать связку Stream6/Channel4
	DMA_DeInit(DMA1_Stream6);

	DMA_InitStructure.DMA_Channel            = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize         = USART_MTR_BUFF_SZ;         // размер массива

	DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;             // циклический режим DMA_Mode_Normal (DMA_Mode_Circular)
	//DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;             // циклический режим DMA_Mode_Normal (DMA_Mode_Circular)
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
   DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);     // Это регистр данных для USART2
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t)&(pBffTX[0]);	   // Указатель на массив
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);                             // применим настройки DMA
	DMA_Cmd(DMA1_Stream6, ENABLE);		                                    // Включение DMA

	//DMA_ITConfig(DMA1_Stream6, DMA_IT_TC | DMA_IT_HT, ENABLE);
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);

	USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

	// Enable DMA2 channel IRQ Channel
	NVIC_InitTypeDef NVIC_InitStructure;

   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   NVIC_EnableIRQ(DMA1_Stream5_IRQn);

   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

/**
* IRQ: USART2 RX, METRICS DATA from USART MECHANICS to the DMA buffer
*/
void DMA1_Stream5_IRQHandler (void)
{
   char tstr[100];
   uint8_t *pBff = 0;
   uint8_t *pDstBff = 0;
   if (DMA1->HISR & DMA_HISR_TCIF5)                    // передача завершена
   {
      DMA1->HIFCR = DMA_HIFCR_CTCIF5;                 // сброс флага события TCIF
      DMA_Cmd(DMA1_Stream5, ENABLE);
      pBff = (uint8_t *)&(pBffRX[USART_MTR_BUFF_SZ]);
      pDstBff = (uint8_t *)&(pMetricBuff[0]);
      copyMetricBuff(pBff, pDstBff);
      MetricBuffFlag = TRUE;
   }
   else if (DMA1->HISR & DMA_HISR_HTIF5)               // половина буфера была передана
   {
      DMA1->HIFCR = DMA_HIFCR_CHTIF5;                 // сброс флага события HTIF
      pBff = (uint8_t *)&(pBffRX[0]);
      pDstBff = (uint8_t *)&(pMetricBuff[0]);
      copyMetricBuff(pBff, pDstBff);
      MetricBuffFlag = TRUE;
   }
   else                                               // ошибка FIFO буфера
   {
      //DMA1->HIFCR = 0xFFFFFFFF;
      //DMA1->LIFCR = 0xFFFFFFFF;
//      DMA1->LIFCR = DMA_LIFCR_CFEIF0;                 // сброс флага события FEIF
//      DMA1->LIFCR = DMA_LIFCR_CDMEIF0;
//      DMA1->LIFCR = DMA_LIFCR_CTEIF0;                 // сброс флага события TEIF

      //SWO_PrintString("USART RX - DMA IRQ event =>> 3\n");
//      uint32_t tdt = DMA1->LISR;
//      sprintf(tstr, "0x%08X\n", tdt);
//
//      SWO_PrintString(tstr);
//
//      tdt = DMA1->HISR;
//      sprintf(tstr, "0x%08X\n", tdt);
//      SWO_PrintString(tstr);
   };
}

/**
* IRQ: USART2 TX, COMMAND DATA from DMA buffer to the USART MECHANICS
*/
void DMA1_Stream6_IRQHandler (void)
{
   //SWO_PrintString("USART TX - DMA Stream6 IRQ event =>>\n");
   if (DMA1->HISR & DMA_HISR_TCIF6)                    // передача завершена
   {
      DMA1->HIFCR = DMA_HIFCR_CTCIF6;                 // сброс флага события TCIF
      DMA_Cmd(DMA1_Stream6, DISABLE);
      ServoTxFlag = TRUE;
      //SWO_PrintString("USART RX - DMA IRQ event =>> 1\n");
      //pBff = (uint8_t *)&(pBffTX[USART_MTR_BUFF_SZ]);
      //pDstBff = (uint8_t *)&(pMetricBuff[0]);
      //copyMetricBuff(pBff, pDstBff);
      //MetricBuffFlag = TRUE;
   }
   else if (DMA1->HISR & DMA_HISR_HTIF6)               // половина буфера была передана
   {
      //DMA1->HIFCR = DMA_HIFCR_CHTIF6;                 // сброс флага события HTIF
      //pBff = (uint16_t *)&(AdcOutBuff[0]);
      //DMA_CallBack(pBff);
      //SWO_PrintString("USART RX - DMA IRQ event =>> 2\n");
      //pBff = (uint8_t *)&(pBffRX[0]);
      //pDstBff = (uint8_t *)&(pMetricBuff[0]);
      //copyMetricBuff(pBff, pDstBff);
      //MetricBuffFlag = TRUE;
   }
   else                                               // ошибка FIFO буфера
   {
      //DMA1->HIFCR = 0xFFFFFFFF;
      //DMA1->LIFCR = 0xFFFFFFFF;
//      DMA1->LIFCR = DMA_LIFCR_CFEIF0;                 // сброс флага события FEIF
//      DMA1->LIFCR = DMA_LIFCR_CDMEIF0;
//      DMA1->LIFCR = DMA_LIFCR_CTEIF0;                 // сброс флага события TEIF

      //SWO_PrintString("USART RX - DMA IRQ event =>> 3\n");
//      uint32_t tdt = DMA1->LISR;
//      sprintf(tstr, "0x%08X\n", tdt);
//
//      SWO_PrintString(tstr);
//
//      tdt = DMA1->HISR;
//      sprintf(tstr, "0x%08X\n", tdt);
//      SWO_PrintString(tstr);
   };
}

void CUsartMetric::enableDMA_IRQ(void)
{
   NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

void CUsartMetric::disableDMA_IRQ(void)
{
   NVIC_DisableIRQ(DMA1_Stream5_IRQn);
}

void CUsartMetric::enableUSART(void)
{
   USART_Cmd(USART2, ENABLE);
}

void CUsartMetric::disableUSART(void)
{
   USART_Cmd(USART2, DISABLE);
}

void CUsartMetric::onSendTXD(uint8_t *date, uint16_t len)
{
   if(len > USART_MTR_BUFF_SZ) len = USART_MTR_BUFF_SZ;
   DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, DISABLE);
   if(ServoTxFlag == TRUE)
   {
      ServoTxFlag = FALSE;
      for(uint16_t i=0; i<len; i++)
      {
         pBffTX[i] = date[i];
      };
      DMA_Cmd(DMA1_Stream6, ENABLE);
      //SWO_PrintString("SERVO METRICS SENT TO OUT for mechanic!!!\n");
   };
   DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
}



