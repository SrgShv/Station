
#include "bufferIP.h"
#include "stm32f4xx_exti.h"

CBufferIP::CBufferIP() :
   m_counter(0),
   m_writePtr(0),
   m_readPtr(0)
{
   for(uint8_t i=0; i<BUFF_PACK_COUNT; i++)
   {
      m_Buffer[i] = 0;
      m_packLen[i] = 0;
   };
}

CBufferIP::~CBufferIP()
{
   for(uint8_t i=0; i<BUFF_PACK_COUNT; i++)
   {
      if(m_Buffer[i] != 0) delete [] m_Buffer[i];
   };
}

void CBufferIP::onAddPack(uint8_t *date, uint16_t len)
{
   if(m_counter < BUFF_PACK_COUNT)
   {
      if(m_Buffer[m_writePtr] != 0) delete [] m_Buffer[m_writePtr];
      m_Buffer[m_writePtr] = new uint8_t[len];
      for(uint16_t i=0; i<len; i++)
      {
         m_Buffer[m_writePtr][i] = date[i];
      };
      ++m_counter;
      m_packLen[m_writePtr] = len;
      if(++m_writePtr >= BUFF_PACK_COUNT) m_writePtr = 0;
   };
}

uint16_t CBufferIP::onCheckBuff(void)
{
   return m_counter;
}

void CBufferIP::onGetPack(uint8_t *date, uint16_t &len)
{
   while(EXTI_GetITStatus(EXTI_Line2) != RESET) ;

   while(EXTI_GetITStatus(EXTI_Line4) != RESET) ;
   if(m_counter > 0)
   {
      --m_counter;
      len = m_packLen[m_readPtr];
      if(len > 0)
      {
         for(uint16_t i=0; i<len; i++)
         {
            date[i] = m_Buffer[m_readPtr][i];
         };
         delete [] m_Buffer[m_readPtr];
         m_Buffer[m_readPtr] = 0;
         if(++m_readPtr >= BUFF_PACK_COUNT) m_readPtr = 0;
      };
   }
   else len = 0;
}


