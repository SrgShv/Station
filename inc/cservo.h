#ifndef CSERVO_H
#define CSERVO_H

#include <stdint.h>

class CServo
{
public:
   CServo();
   ~CServo();

   void onSetFlags(uint32_t flags);
   void onSetDevMtr(uint32_t metric, uint8_t type);
   uint32_t onGetFlags(void);
   uint32_t onGetDevMtr(uint8_t &type);
   char onCheckSetFlg(void);
   void onUpdateSetFlg(void);
   void onClearSetFlg(void);
protected:
private:
   uint32_t m_Flags;
   uint32_t m_Metric;
   uint8_t m_DevType;
   char m_SetFlag;
};

#endif // CSERVO_H
