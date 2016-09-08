#ifndef CMETRIC_H
#define CMETRIC_H

#include <stdint.h>

#define METR_BFF_LEN    10

class CMetrics
{
public:
   CMetrics();
   ~CMetrics();
   void onSetFlag(char flag, uint8_t pos);
   void onSetFlags(uint32_t flags);
   void onSetMtrVal(uint32_t val, uint8_t pos);
   void onClearBff(void);
   char onCheckChangeInBff(void);
   uint32_t onGetMtr(uint32_t *pBuffer);
protected:
private:
   uint32_t m_Flags;
   uint32_t m_ValMtr[METR_BFF_LEN];
   uint32_t m_oldFlags;
   uint32_t m_oldValMtr[METR_BFF_LEN];
   char m_changeFlg;
};

#endif // CMETRIC_H
