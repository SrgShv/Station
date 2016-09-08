#include <iostream>
#include "main.h"
#include "struct.h"

#define START_SW					1
#define RESET_CONNECT_SW		2
#define SITE_REQUEST_SW			3
#define SELECT_NET_SW			4
#define CLEAR_INIT_SW			5
#define RUN_SW						10

#define TRUE	1
#define FALSE	0

#define FLASH_SZ     127

using namespace std;

/** TEST **/
/** !!! GLOBAL FUNCTIONS **/

void delay_us(unsigned int us)
{
   unsigned int tcnt = 0;
   while(++tcnt < us*5);
}

void EthernetIrqRXA(void)
{
   //SWO_PrintString("IrqRXA\n");

//   uint16_t len = 0;
//   for(uint16_t i=0; i<100; i++)
//   {
//      len = enc28j60_recv_packetA(rxBuffA, BUFFRX_LEN);
//      if(len == 0) break;
//      else
//      {
//         if(1 == checkJMP2()) pBuffIP->onAddPack(rxBuffA, len);
//         pTransitBuffA->onAddPack(rxBuffA, len);
//      };
//   };

   // if(not session) -> copy to the buffer -> send to the channel B
   //showNetPack(rxBuffA, len, 0);
}

void EthernetIrqRXB(void)
{
   //SWO_PrintString("IrqRXB\n");

//   uint16_t len = 0;
//   for(uint16_t i=0; i<100; i++)
//   {
//      len = enc28j60_recv_packetB(rxBuffB, BUFFRX_LEN);
//      if(len == 0) break;
//      else
//      {
//         if(0 == checkJMP2()) pBuffIP->onAddPack(rxBuffB, len);
//         pTransitBuffB->onAddPack(rxBuffB, len);
//      };
//   };

   // if(not session) -> copy to the buffer -> send to the channel A
   //showNetPack(rxBuffB, len, 0);
}

void Tim2IrqEvent(void)
{
//   pEventTim->onTicTimer();
//   pTim2->onTicTimer();
//   pTimRST->onTicTimer();
//   if(++TimeStamp == 0xFFFFFFFF) TimeStamp = 132;
}

/** GLOBAL FUNCTIONS !!! **/

CStation::CStation() :
   pEventTim(0),
   pTim2(0),
   pTimRST(0),
   pTerminalTimoutRST(0),
   pStartTim(0),
   pBuffIP(0),
   pTransitBuffA(0),
   pTransitBuffB(0),
   pClient(0),
   pUSART(0),
   pSPX(0),
   pSpxPackUSART(0),
   pEventBuff(0),
   pMtrDate(0),
   pServo(0),
   pMtrUSART(0),
   m_pFlash(0),

	m_StationIP(0),
	m_SubNetMask(0),
	m_RouterIP(0),
	m_ServerDhcpIP(0),
	m_ServerDnsnIP(0),
	m_ServerAltDnsnIP(0),
	m_SiteIP(0),
	m_AddressSiteURL1(0),
	m_AddressSiteURL2(0),
	m_LenURL1(0),
	m_LenURL2(0),
	m_StationMACA(0),
	m_StationMACB(0),
	m_StaticFlag(FALSE)
{
   m_StationMACA = new uint8_t[6];
   m_StationMACB = new uint8_t[6];

   pEventTim = new CTimer();
   pTim2 = new CTimer();
   pTimRST = new CTimer();
   pTerminalTimoutRST = new CTimer();
   pStartTim = new CTimer();
   pBuffIP = new CBufferIP();
   pTransitBuffA = new CBufferIP();
   pTransitBuffB = new CBufferIP();
   pClient = new CApoClient();
   pUSART = new CUsart();
   pSPX = new CSpeexPack();
   pSpxPackUSART = new CDoubleSpxBuffer();
   pEventBuff = new CEventBuffer();
   pMtrDate = new CMetrics();
   pServo = new CServo();
   pMtrUSART = new CUsartMetric();
}

CStation::~CStation()
{
   if(m_StationMACA != 0) delete [] m_StationMACA;
   if(m_StationMACB != 0) delete [] m_StationMACB;
   if(m_AddressSiteURL1 != 0) delete [] m_AddressSiteURL1;
   if(m_AddressSiteURL2 != 0) delete [] m_AddressSiteURL2;
}

int CStation::startInit()
{
	int res = SELECT_NET_SW;

	SystemInit();
   delay_us(1000);
   PeripheryEnable();
   delay_us(1000);
	initGPIO_JMP();
	delay_us(1000);

   if(m_pFlash != 0) delete m_pFlash;
   m_pFlash = new CFlash();
	if(m_pFlash != 0)
   {
      m_pFlash->onInit();
      uint8_t *buff = new uint8_t[FLASH_SZ];
      for(uint16_t i=0; i<FLASH_SZ; i++)
      {
         buff[i] = m_pFlash->onReceiveByte(0, i);
      };
      sAddrFLASH *addr = (sAddrFLASH *)(&buff[0]);
      delete m_pFlash;

      /** LAN A->B(1) or LAN B->A(0) **/
      if(checkJMP2())
      {
         m_StationMACA[0] = (uint8_t)(addr->MacNumb[5]);
         m_StationMACA[1] = (uint8_t)(addr->MacNumb[4]);
         m_StationMACA[2] = (uint8_t)(addr->MacNumb[3]);
         m_StationMACA[3] = 0x01;
         m_StationMACA[4] = (uint8_t)(addr->devNumb>>8);
         m_StationMACA[5] = (uint8_t)(addr->devNumb);

         m_StationMACB[0] = 0xE6;
         m_StationMACB[1] = 0xD4;
         m_StationMACB[2] = 0xD3;
         m_StationMACB[3] = 0x01;
         m_StationMACB[4] = (uint8_t)(addr->devNumb>>8);
         m_StationMACB[5] = (uint8_t)(addr->devNumb);
      }
      else
      {
         m_StationMACB[0] = (uint8_t)(addr->MacNumb[5]);
         m_StationMACB[1] = (uint8_t)(addr->MacNumb[4]);
         m_StationMACB[2] = (uint8_t)(addr->MacNumb[3]);
         m_StationMACB[3] = 0x01;
         m_StationMACB[4] = (uint8_t)(addr->devNumb>>8);
         m_StationMACB[5] = (uint8_t)(addr->devNumb);

         m_StationMACA[0] = 0xE6;
         m_StationMACA[1] = 0xD4;
         m_StationMACA[2] = 0xD3;
         m_StationMACA[3] = 0x01;
         m_StationMACA[4] = (uint8_t)(addr->devNumb>>8);
         m_StationMACA[5] = (uint8_t)(addr->devNumb);
      };

      /** 1 - dynamic IP, 0 - static IP **/
      if(checkJMP1()) m_StaticFlag = FALSE;
      else m_StaticFlag = TRUE;

      if(m_StaticFlag)
      {
         m_StationIP = addr->staticIP;
         m_SubNetMask = addr->staticMSK;
         m_RouterIP = addr->staticSHL;
         m_ServerDnsnIP = addr->staticDNS;
         m_ServerAltDnsnIP = addr->staticADNS;
      };

      m_LenURL1 = 0;
      for(uint8_t i = 0; i < URL_TEXT_LEN; i++)
      {
         if(addr->text[i] == 0)
         {
            m_LenURL1 = i;
            break;
         };
      };
      if(m_AddressSiteURL1 != 0) delete [] m_AddressSiteURL1;
      if(m_LenURL1 > 0)
      {
         m_AddressSiteURL1 = new char[m_LenURL1+1];
         if(m_AddressSiteURL1 != 0)
         {
            for(uint8_t i = 0; i < m_LenURL1; i++)
            {
               m_AddressSiteURL1[i] = addr->text[i];
            };
            m_AddressSiteURL1[m_LenURL1] = 0;
         };
      };

      m_LenURL2 = 0;
      for(uint8_t i = 0; i < URL_TEXT_LEN; i++)
      {
         if(addr->atext[i] == 0)
         {
            m_LenURL2 = i;
            break;
         };
      };
      if(m_AddressSiteURL2 != 0) delete [] m_AddressSiteURL2;
      if(m_LenURL2 > 0)
      {
         m_AddressSiteURL2 = new char[m_LenURL2+1];
         if(m_AddressSiteURL2 != 0)
         {
            for(uint8_t i = 0; i < m_LenURL2; i++)
            {
               m_AddressSiteURL2[i] = addr->text[i];
            };
            m_AddressSiteURL2[m_LenURL2] = 0;
         };
      };

      addr = 0;
      delete [] buff;
   }
   else res = 0;

   delay_us(1000);
   initENC28J60_SPI();
   delay_us(1000);
   enc28j60_initA(m_StationMACA);
   delay_us(1000);
   enc28j60_initB(m_StationMACB);
   delay_us(1000);
   enc28j60_write_phyA(0x14, 0x476); // Leds ON
   delay_us(1000);
   enc28j60_write_phyB(0x14, 0x476); // Leds ON
   delay_us(100000);
   initLeds();
   setGreenLedOFF();
   setRedLedOFF();
   InitSoundCtrlPin();

   /**
   init USART 1,2
   ...
   */

	return res;
}

int CStation::setNet()
{
	int res = 0;
	res = RESET_CONNECT_SW;
	return res;
}

int CStation::setConnect()
{
	int res = 0;
	res = SITE_REQUEST_SW;
	return res;
}

int CStation::siteRequest()
{
	int res = 0;
	res = RUN_SW;
	return res;
}

int CStation::clearInit()
{
	int res = 0;
	return res;
}

int CStation::run()
{
	int res = 0;
	while(1)
	{
	};
	return res;
}

int main()
{
	int res = START_SW;
	CStation *pSt = new CStation();

	if(pSt != 0)
	{
		while(res != 0)
		{
			switch(res)
			{
			case START_SW:
				res = pSt->startInit();
				break;
			case SELECT_NET_SW:
				res = pSt->setNet();
				break;
			case RESET_CONNECT_SW:
				res = pSt->setConnect();
				break;
			case SITE_REQUEST_SW:
				res = pSt->siteRequest();
				break;
			case CLEAR_INIT_SW:
				res = pSt->clearInit();
				break;
			case RUN_SW:
				res = pSt->run();
				break;
			};
		};
		delete pSt;
		pSt = 0;
	};

	return 0;
}
