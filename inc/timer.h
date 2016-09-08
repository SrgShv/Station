#ifndef TIMER_H
#define TIMER_H

typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned long int DWORD;

extern "C"
{
#include "swo.h"
void TIM2_IRQHandler(void);
}

void TIM2_Config(void);
void NVIC2_Config(void);
void TIM2_IRQHandler(void);
void TIM2_Start(void);

class CTimer
{
public:
   CTimer();
   ~CTimer();
   void onStartTimer(DWORD ms);
   void onStopTimer(void);
   void onResetTimer(void);
   void onTicTimer(void);
   void onTimeOut(void);
   char onCheckTimout(void);
protected:
private:
   DWORD m_cntr;
   DWORD m_maxT;
   char m_flag;
   char m_timeout;
};

#endif // TIMER_H
