#ifndef USART_H
#define USART_H

#include <stdint.h>

#define SPX_BFF_SZ      2300
#define SPX_PACK_SZ     20
#define NET_SPX_PACK_SZ 1000
#define SPX_PACK_CNT    (NET_SPX_PACK_SZ/SPX_PACK_SZ)
#define USART_BFF_SZ    50
#define DBUFF_SZ        (SPX_PACK_CNT*3)

extern "C"
{
#include "swo.h"
void USART1_IRQHandler();

void DMA1_Stream5_IRQHandler (void);
void DMA1_Stream6_IRQHandler(void);
//void DMA1_Stream7_IRQHandler();
}

class CSpeexPack
{
public:
   CSpeexPack();
   ~CSpeexPack();
   //void onWriteNext(uint8_t date);
   void onWriteChar(uint8_t date, uint16_t pos);
   //uint16_t onCheckWrCntr(void);
   uint8_t *onGetPack(uint16_t &len);
   char onAddPackToNetBuff(uint8_t *date, uint16_t len);
   uint8_t *onGetNetPack(uint16_t &len);

protected:
private:
   uint8_t m_PackA[SPX_PACK_SZ];
   uint8_t m_PackB[SPX_PACK_SZ];
   uint8_t m_NetSpxPack[NET_SPX_PACK_SZ];
   char m_switchAB;
   uint16_t m_cntA;
   uint16_t m_cntB;
   char m_flagA;
   char m_flagB;
   uint8_t m_NetPackCntr;
};

class CBufferSpx
{
public:
   CBufferSpx();
   ~CBufferSpx();
   void onAddDate(uint8_t *date, uint16_t len);
   void onAddChar(uint8_t &date);
   uint16_t onCheck(void);
   uint8_t onGetChar(void);
   void onGetArray(uint8_t *date, uint16_t &len);


protected:
private:
   uint8_t m_Buff[SPX_BFF_SZ];
   uint16_t m_counter;
   uint16_t m_write;
   uint16_t m_read;
};

class CUsart
{
public:
   CUsart();
   ~CUsart();

   void onInit(void);
   void enableUSART(void);
   void disableUSART(void);
   void onSendTXD(uint8_t *date, uint16_t len);
   char onCheckIsEmptyTX(void);
   void onCheckSendTX(void);
   CBufferSpx *pBffTX;
   CBufferSpx *pBffRX;
   void onPutUSART(uint8_t charD);
   void onSendFromBuff(void);


protected:
private:
   uint8_t pBuffTX[USART_BFF_SZ];
   uint16_t counterTX;
   uint16_t readTX;
   uint16_t writeTX;
};

class CDoubleSpxBuffer
{
public:
   CDoubleSpxBuffer();
   ~CDoubleSpxBuffer();
   void SetPack(uint8_t *pack);
   uint8_t *GetPack(uint16_t &lenD);

protected:
private:
   uint8_t dbuffA[DBUFF_SZ][SPX_PACK_SZ+4];
   uint8_t dbuffB[SPX_PACK_SZ+4];
   uint16_t posPack;
   uint16_t packCount;
   uint16_t packRead;
   uint16_t packWrite;
};

#define USART_MTR_BUFF_SZ     50
class CUsartMetric
{
public:
   CUsartMetric();
   ~CUsartMetric();

   void onInit(void);
   void enableDMA_IRQ(void);
   void disableDMA_IRQ(void);
   void enableUSART(void);
   void disableUSART(void);
   void onSendTXD(uint8_t *date, uint16_t len);

protected:
private:
   //uint8_t pBffTX[USART_MTR_BUFF_SZ*2];
   //uint8_t pBffRX[USART_MTR_BUFF_SZ*2];
};

void copyMetricBuff(uint8_t *pSrcBff, uint8_t *pDstBff);
uint8_t CheckMetricFlag(void);
uint8_t *GetMetricDate(void);

#endif // USART_H
