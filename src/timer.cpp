
#include "timer.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

#define TRUE         1
#define FALSE        0

extern void Tim2IrqEvent(void);

void TIM2_Config(void)
{
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);

	/* Делитель учитывается как TIM_Prescaler + 1, поэтому отнимаем 1 */
	/* Ft2 = 168MHz/((TIM_Prescaler+1)*TIM_Period) = 16000 Hz */
   base_timer.TIM_Prescaler = 31;
	base_timer.TIM_Period = 2625;
	base_timer.TIM_CounterMode = TIM_CounterMode_Up;
	base_timer.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &base_timer);

  /* Разрешаем прерывание по обновлению (в данном случае -
   * по переполнению) счётчика таймера TIM2. */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	/* Включаем таймер */
	TIM_Cmd(TIM2, ENABLE);
}

void NVIC2_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      Tim2IrqEvent();
   };
}

void TIM2_Start(void)
{
   TIM2_Config();
   NVIC2_Config();
}

/*************************************************/
CTimer::CTimer() :
   m_cntr(0),
   m_maxT(0),
   m_flag(FALSE),
   m_timeout(FALSE)
{
}

CTimer::~CTimer()
{
}

void CTimer::onStartTimer(DWORD ms)
{
   uint32_t c = 0;
   while(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      if(++c >= 100000) break;
   };
   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
   if(m_flag == FALSE)
   {
      m_cntr = 0;
      m_maxT = ms;
      m_flag = TRUE;
      m_timeout = FALSE;
   };
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void CTimer::onStopTimer(void)
{
   uint32_t c = 0;
   while(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      if(++c >= 100000) break;
   };

   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
   if(m_flag)
   {
      m_cntr = 0;
      m_maxT = 0;
      m_flag = FALSE;
      m_timeout = FALSE;
   };
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void CTimer::onResetTimer(void)
{
   if(m_flag) m_cntr = 0;
}

void CTimer::onTicTimer(void)
{
   uint32_t c = 0;
   while(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      if(++c >= 100000) break;
   };

   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
   if(m_flag)
   {
      if(++m_cntr >= m_maxT)
      {
         this->onTimeOut();
         m_cntr = 0;
      };
   };
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void CTimer::onTimeOut(void)
{
   m_timeout = TRUE;
   m_cntr = 0;
   m_maxT = 0;
   m_flag = FALSE;
}

char CTimer::onCheckTimout(void)
{
   uint32_t c = 0;
   while(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      if(++c >= 100000) break;
   };

   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
   char res = FALSE;
   if(m_timeout)
   {
      res = TRUE;
      m_timeout = FALSE;
   };
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
   return res;
}


