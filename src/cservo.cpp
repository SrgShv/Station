
#include "cservo.h"

CServo::CServo() :
   m_Flags(0),
   m_Metric(0),
   m_DevType(0),
   m_SetFlag(0)
{
}

CServo::~CServo()
{
}

void CServo::onSetFlags(uint32_t flags)
{
   m_Flags = flags;
}

void CServo::onSetDevMtr(uint32_t metric, uint8_t type)
{
   m_Metric = metric;
   m_DevType = type;
}

uint32_t CServo::onGetFlags(void)
{
   return m_Flags;
}

uint32_t CServo::onGetDevMtr(uint8_t &type)
{
   type = m_DevType;
   return m_Metric;
}

char CServo::onCheckSetFlg(void)
{
   return m_SetFlag;
}

void CServo::onUpdateSetFlg(void)
{
   m_SetFlag = 1;
}

void CServo::onClearSetFlg(void)
{
   m_SetFlag = 0;
}

