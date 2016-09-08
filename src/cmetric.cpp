
#include "cmetric.h"

CMetrics::CMetrics() :
   m_Flags(0),
   m_oldFlags(0),
   m_changeFlg(0)
{
   this->onClearBff();
}

CMetrics::~CMetrics()
{
}

void CMetrics::onSetFlags(uint32_t flags)
{
   if(m_changeFlg == 0)
   {
      if(flags != m_Flags) m_changeFlg = 1;
   };
   m_Flags = flags;
}

void CMetrics::onSetFlag(char flag, uint8_t pos)
{
   if(pos < METR_BFF_LEN)
   {
      uint32_t tFlg = m_Flags;
      if(flag) m_Flags |= (0x00000001 << pos);
      else m_Flags &= ~(0x00000001 << pos);
      if(tFlg != m_Flags) m_changeFlg = 1;
   };
}

void CMetrics::onSetMtrVal(uint32_t val, uint8_t pos)
{
   if(pos < METR_BFF_LEN)
   {
      m_ValMtr[pos] = val;
      if(m_ValMtr[pos] != m_oldValMtr[pos]) m_changeFlg = 1;
   };
}

void CMetrics::onClearBff(void)
{
   m_Flags = 0;
   m_oldFlags = 0;
   m_changeFlg = 0;
   for(uint8_t i=0; i<METR_BFF_LEN; i++)
   {
      m_ValMtr[i] = 0;
      m_oldValMtr[i] = 0;
   };
}

char CMetrics::onCheckChangeInBff(void)
{
   return m_changeFlg;
}

uint32_t CMetrics::onGetMtr(uint32_t *pBuffer)
{
   m_changeFlg = 0;
   for(uint8_t i=0; i<METR_BFF_LEN; i++)
   {
      pBuffer[i] = m_ValMtr[i];
      m_oldValMtr[i] = m_ValMtr[i];
   };
   m_oldFlags = m_Flags;
   return m_Flags;
}
