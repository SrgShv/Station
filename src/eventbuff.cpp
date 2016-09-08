
#include "eventbuff.h"

CEventBuffer::CEventBuffer() :
   m_count(0),
   m_read(0),
   m_write(0)
{

}

CEventBuffer::~CEventBuffer()
{

}

void CEventBuffer::onAddEvent(uint8_t evnt)
{
   if(m_count < EVENT_BUFF_SZ)
   {
      m_Buff[m_write] = evnt;
      if(++m_write >= EVENT_BUFF_SZ) m_write = 0;
      ++m_count;
   };
}

uint8_t CEventBuffer::onGetEvent(void)
{
   uint8_t res = 0;
   if(m_count > 0)
   {
      res = m_Buff[m_read];
      if(++m_read >= EVENT_BUFF_SZ) m_read = 0;
      --m_count;
   };
   return res;
}
