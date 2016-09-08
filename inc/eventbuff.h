#ifndef EVENTBUFF_H
#define EVENTBUFF_H

#include <stdint.h>

#define EVENT_BUFF_SZ   10

class CEventBuffer
{
public:
   CEventBuffer();
   ~CEventBuffer();
   void onAddEvent(uint8_t evnt);
   uint8_t onGetEvent(void);
protected:
private:
   uint8_t m_Buff[EVENT_BUFF_SZ];
   uint8_t m_count;
   uint8_t m_read;
   uint8_t m_write;

};

#endif /* EVENTBUFF_H */
