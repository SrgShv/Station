#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

class CFlash
{
public:
   CFlash();
   ~CFlash();
   void onInit(void);
//   char onStartI2C(void);
//   void onStopI2C(void);
   //char onWriteByte(BYTE Address, WORD Register, BYTE Data);
   uint8_t onReceiveByte(uint8_t Address, uint16_t Register);
protected:
private:
};

#endif // FLASH_H
