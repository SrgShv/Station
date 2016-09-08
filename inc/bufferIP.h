#ifndef BUFFERIP_H
#define BUFFERIP_H

#include <stdint.h>

#define BUFF_PACK_COUNT    20


class CBufferIP
{
public:
   CBufferIP();
   ~CBufferIP();
   void onAddPack(uint8_t *date, uint16_t len);
   uint16_t onCheckBuff(void);
   void onGetPack(uint8_t *date, uint16_t &len);

protected:
private:
   uint8_t *m_Buffer[BUFF_PACK_COUNT];
   uint16_t m_packLen[BUFF_PACK_COUNT];
   uint16_t m_counter;
   uint16_t m_writePtr;
   uint16_t m_readPtr;
};

#endif // BUFFERIP_H
