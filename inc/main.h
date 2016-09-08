#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "timer.h"
#include "spi.h"
#include "flash.h"
#include "struct.h"
#include "jumper.h"
#include "bufferIP.h"
#include "handlerIP.h"
#include "tcp.h"
#include "periphery.h"
#include "usart.h"
#include "cmetric.h"
#include "cservo.h"
#include "eventbuff.h"

void delay_us(unsigned int us);
void EthernetIrqRXA(void);
void EthernetIrqRXB(void);
void Tim2IrqEvent(void);

extern "C"
{
#include "swo.h"
void SystemInit();
}



class CStation
{
public:
  CStation();
  ~CStation();

  int run();
  int setNet();
  int setConnect();
  int siteRequest();
  int startInit();
  int clearInit();

protected:
   CFlash *m_pFlash;
   CTimer *pEventTim;
   CTimer *pTim2;
   CTimer *pTimRST;
   CTimer *pTerminalTimoutRST;
   CTimer *pStartTim;
   CBufferIP *pBuffIP;
   CBufferIP *pTransitBuffA;
   CBufferIP *pTransitBuffB;
   CApoClient *pClient;
   CUsart *pUSART;
   CSpeexPack *pSPX;
   CDoubleSpxBuffer *pSpxPackUSART;
   CEventBuffer *pEventBuff;
   CMetrics *pMtrDate;
   CServo *pServo;
   CUsartMetric *pMtrUSART;
private:
	uint32_t m_StationIP;
	uint32_t m_SubNetMask;
	uint32_t m_RouterIP;
	uint32_t m_ServerDhcpIP;
	uint32_t m_ServerDnsnIP;
	uint32_t m_ServerAltDnsnIP;
	uint32_t m_SiteIP;
	char *m_AddressSiteURL1;
	char *m_AddressSiteURL2;
	uint8_t m_LenURL1;
	uint8_t m_LenURL2;
	uint8_t *m_StationMACA;
	uint8_t *m_StationMACB;
	uint8_t m_StaticFlag;
//	uint8_t *m_TxBuffA;
//	uint8_t *m_RxBuffA;
//	uint8_t *m_TxBuffB;
//	uint8_t *m_RxBuffB;

};

/**************************************************************/
uint32_t reverseDWORD(uint32_t d);
uint16_t reverseWORD(uint16_t d);
void showIP(uint32_t ip);
void showDevNMB(uint16_t nmb);
void showMAC(char *mac);
void showNetPack(uint8_t *data, uint16_t len, uint8_t dir);
void showNetPackASCII(uint8_t *data, uint16_t len, uint8_t dir);
void setAddrIP(uint32_t client, uint32_t submask, uint32_t router, uint32_t dhcp);
void InitClientNetAddr(void);
void SendNetDatePack(uint8_t *date, uint16_t len);
uint16_t GetIpCRC(uint8_t *b, uint16_t len);
char threadInit(void);
char sendDhcpDiscover(void);
char sendDhcpRequest(void);
char sendARP(void);
void sendRespARP(uint32_t trgIP, uint8_t *trgMAC);
char sendDnsRequest(void);
uint32_t getMyClientIP(void);
uint32_t getMyRouterIP(void);
uint32_t getSiteIP(void);
void setRouterMAC(uint8_t *mac);
void setAddrIP(uint32_t trmIP);
char sendSiteRequest(void);
uint8_t *getNetBuffPtr(void);
uint8_t *getClientMacPtr(void);
uint8_t *getRouterMacPtr(void);
uint32_t getTimeStamp(void);
char sendTcpSYN(void);
void sendTcpACK(uint32_t dateLen);
char sendTcpHTTP_SiteRequest(uint32_t dateLen);
char sendTcpACK_HTTP(uint32_t dateLen);
char sendTcpFIN_ACK(void);
void setReset(void);

char sendTermSYN(uint16_t port, uint32_t IPaddr);
void sendTermACK(uint32_t dateLen, uint16_t port, uint32_t IPaddr);
//char sendTcpTerm(uint32_t dateSeqLen, uint16_t len, uint8_t *date, uint16_t port, uint32_t IPaddr);
char registerClientTerminal(uint16_t port, uint32_t IPaddr);
char closeClientTerminal(uint16_t port, uint32_t IPaddr);
char sendTermFIN(uint16_t port, uint32_t IPaddr);

void sendPackForTerminal(uint8_t *date, uint16_t len);
void SendNetEvent(void);
void SetEventFlag(uint8_t flag);
uint8_t GetEventFlag(void);
void sendACK(uint32_t seq, uint32_t ack);
void sendACK_RST(uint32_t seq, uint32_t ack);
void ComputeSeqAck(uint8_t TxFlag, uint32_t &seqTX, uint32_t &ackTX);
void SetSeqAckRX(uint8_t lastRxFlag, uint32_t seqRX, uint32_t ackRX, uint32_t lenRX);
void GetSeqAckRX(uint8_t &lastRxFlag, uint32_t &seqRX, uint32_t &ackRX, uint32_t &lenRX);
char checkACK(void);
void SetTcpFlagTX(uint8_t flag);
void GetLastTcpFlags(uint8_t &lastRxFlag, uint8_t &lastTxFlag);
uint8_t HandleApoMsg(uint8_t *date, uint16_t len);
uint16_t GetMyDevNumb(void);
void OpenSession(void);
void CloseSession(void);
void SetReceivedEvent(uint8_t flag);
uint8_t GetReceivedEvent(void);
void ClearReceivedEvent(void);
void SetSentEvent(uint8_t flag);
uint8_t GetSentEvent(void);
void ClearSentEvent(void);
void SetActiveMask(uint8_t mask);
uint8_t GetActiveMask(void);
void ParseMetricsRX(uint8_t *packRX);
uint8_t getCRC8(uint8_t *date, uint16_t len);
void ReSendToSubNetDatePack(uint8_t *date, uint16_t len);
/**************************************************************/

#endif // MAIN_H
