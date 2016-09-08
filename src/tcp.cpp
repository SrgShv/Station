
#include "tcp.h"

#include <stdio.h>

#define TRUE         1
#define FALSE        0

#define SOURCE_PORT           ((unsigned short int)13339)
#define TERMINAL_PORT         ((unsigned short int)40000)
#define HTTP_PORT             ((unsigned short int)80)

#define TCP_FIN_FLG           0x01
#define TCP_SYN_FLG           0x02
#define TCP_RST_FLG           0x04
#define TCP_PSH_FLG           0x08
#define TCP_ACK_FLG           0x10

#define LIFT_MASK       0x10
#define ENTR_MASK       0x20
#define MASH_MASK       0x30

#define CLIENT_MASK     0x0C
#define TERMINAL_MASK   0x0A

#define HARD_HEAD_LEN         14
#define IP_HEAD_LEN           20
#define DATA_OFFSET           32

extern uint8_t *getNetBuffPtr(void);
extern uint8_t *getClientMacPtr(void);
extern uint8_t *getRouterMacPtr(void);
extern uint32_t getMyClientIP(void);
extern uint32_t reverseDWORD(uint32_t d);
extern uint16_t reverseWORD(uint16_t d);
extern uint32_t getSiteIP(void);
extern uint32_t getTimeStamp(void);
extern uint16_t GetIpCRC(uint8_t *b, uint16_t len);
extern void showNetPack(uint8_t *data, uint16_t len, uint8_t dir);
extern void showNetPackASCII(uint8_t *data, uint16_t len, uint8_t dir);
extern void showIP(uint32_t ip);
extern void sendPackForTerminal(uint8_t *date, uint16_t len);
extern void SetSeqAckRX(uint8_t lastRxFlag, uint32_t seqRX, uint32_t ackRX, uint32_t lenRX);
extern void SetTcpFlagTX(uint8_t flag);
extern void sendTcpACK(uint32_t dateLen);
extern void SetSentEvent(uint8_t flag);

volatile uint32_t OutTStampNmb = 0;
volatile uint32_t OutTEchoNmb = 0;
volatile uint16_t TerminalPort = 0;
volatile uint32_t TerminalIP = 0;

char hostName[] = "aposervices.adr.com.ua";

uint32_t GetCRC32(uint8_t* date, uint16_t len)
{
   register uint32_t crc = 0xFFFFFFFF;
	if(len > 0)
	{
		if(date)
		{
			char flag;
			uint8_t tch;
			for(uint16_t i=0; i<len; i++)
			{
				tch = date[i];
				for(uint8_t n=0; n<8; n++)
				{
					if(crc & 0x80000000) flag = TRUE;
					else flag = FALSE;
					crc <<= 1;
					if(tch & 0x80) crc |= 1;
					else crc &= 0xFFFFFFFE;
					tch <<= 1;
					if(flag) crc ^= 0x04C11DB7;
				};
			};
		};
	};
	return crc;
}

uint16_t setTCP(STcpItem &item, uint8_t *pBFF)
{
   //SWO_PrintString("uint16_t setTCP(STcpItem &item, uint8_t *pBFF)\n");
   uint16_t res = 0;
   sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&pBFF[0]);

   //item.data = (uint8_t*)(&hostName[0]);

   /**********************************************/
   static uint16_t dID = 1000;
   uint16_t ndlen = item.len;
   if(item.len & 0x0001) ++ndlen;
   uint8_t *pMyMAC = getClientMacPtr();
   uint8_t *pDstMAC = getRouterMacPtr();
   for(uint8_t i=0; i<6; i++)
   {
      pTcpHead->destMAC[i] = pDstMAC[i];
      pTcpHead->srcMAC[i] = pMyMAC[i];
   };
   pTcpHead->type[0] = 0x08;
   pTcpHead->type[1] = 0x00;
   pTcpHead->hdrLen = 0x45;
   pTcpHead->diffServ = 0x00;
   pTcpHead->dataID = reverseWORD(dID);
   if(++dID >= 0xFFFF) dID = 1000;
   pTcpHead->flags[0] = 0x40;
   pTcpHead->flags[1] = 0x00;
   pTcpHead->ttl = 0x80;
   pTcpHead->protocol = 0x06;
   pTcpHead->hdrCRC = 0x0000;
   pTcpHead->clientIP = getMyClientIP();
//   pTcpHead->targIP = getSiteIP();                       // ???????????????????????????====================
   pTcpHead->targIP = item.trgIP;
   pTcpHead->srcPort = reverseWORD(SOURCE_PORT);         // ???????????????????????????====================
   pTcpHead->destPort = reverseWORD(item.trgPort);       // ???????????????????????????====================
   SetTcpFlagTX(item.flag);
   if(item.flag == (uint8_t)TCP_ACK_FLG)
   {
      pTcpHead->seqNumb = reverseDWORD(item.seq);
      pTcpHead->ackNumb = reverseDWORD(item.ack);
   }
   else if(item.flag == (uint8_t)(TCP_ACK_FLG | TCP_PSH_FLG))
   {
      pTcpHead->seqNumb = reverseDWORD(item.seq);
      pTcpHead->ackNumb = reverseDWORD(item.ack);
   }
   else if(item.flag == (uint8_t)(TCP_ACK_FLG | TCP_FIN_FLG))
   {
      pTcpHead->seqNumb = reverseDWORD(item.seq);
      pTcpHead->ackNumb = reverseDWORD(item.ack);
   }
   else if(item.flag == (uint8_t)TCP_SYN_FLG)
   {
      pTcpHead->seqNumb = reverseDWORD(item.seq);
      pTcpHead->ackNumb = 0;
   }
   else if(item.flag == (uint8_t)TCP_RST_FLG)
   {
      pTcpHead->seqNumb = reverseDWORD(item.seq);
      pTcpHead->ackNumb = 0;
   };
   pTcpHead->dataOffs = 0xA0;
   pTcpHead->tcpFlgs = item.flag;
   pTcpHead->windSz = reverseWORD(1500);
   pTcpHead->crcData = 0x0000;
   pTcpHead->urgentPtr = 0x0000;

   pTcpHead->NOP1 = 0x01;
   pTcpHead->NOP2 = 0x01;

//   pTcpHead->maxKindSS = 0x02;
//   pTcpHead->maxLenSS = 0x04;
//   pTcpHead->maxValSS = reverseWORD(1460);
//   pTcpHead->tcpSackPrm = 0x04;
//   pTcpHead->tcpSackLen = 0x02;
   pTcpHead->tcpKind = 0x08;
   pTcpHead->tcpLen = 0xA0;

   OutTStampNmb = getTimeStamp();
   pTcpHead->timeStamp = reverseDWORD(OutTStampNmb);
   pTcpHead->timeEcho = reverseDWORD(OutTEchoNmb);
//   pTcpHead->tnop = 0x01;
//   pTcpHead->winScaleKind = 0x03;
//   pTcpHead->winScaleLen = 0x03;
//   pTcpHead->winScaleMult = 0x07;

   uint16_t dateOffset = (pTcpHead->dataOffs>>4)*4;
   pTcpHead->totalLen = reverseWORD(ndlen + IP_HEAD_LEN + dateOffset); // (D+20+9*4)+14 = D+56

   uint8_t *pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN]);
   pTcpHead->hdrCRC = GetIpCRC(pB, IP_HEAD_LEN);

   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset]);
   for(uint16_t i=0; i<item.len; i++)
   {
      pB[i] = item.data[i];
   };
   if(item.len & 0x0001) pB[item.len] = 0;

   /** ------------ TCP PSEUDO HEADER ------------- **/
   sTcpPseudo *pPseudoHdrTcp = 0;
   uint16_t pseudoCntr = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;
   pPseudoHdrTcp = (sTcpPseudo*)(&pBFF[pseudoCntr]); // pos D+70
   pPseudoHdrTcp->clientIP = pTcpHead->clientIP;
   pPseudoHdrTcp->targIP = pTcpHead->targIP;
   pPseudoHdrTcp->reserved = 0x00;
   pPseudoHdrTcp->protocol = pTcpHead->protocol;
   pPseudoHdrTcp->length = reverseWORD(ndlen + dateOffset);
   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN]);
   pTcpHead->crcData = GetIpCRC(pB, ndlen + dateOffset + 12);
   res = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;

   /**********************************************/

   return res;
}

uint16_t setTermSYN(STcpItem &item, uint8_t *pBFF)
{
   uint16_t res = 0;
   sTcpHeadSYN *pTcpHead = (sTcpHeadSYN *)(&pBFF[0]);
   /**********************************************/
   static uint16_t dID = 1000;
   uint16_t ndlen = item.len;
   if(item.len & 0x0001) ++ndlen;
   uint8_t *pMyMAC = getClientMacPtr();
   uint8_t *pDstMAC = getRouterMacPtr();
   for(uint8_t i=0; i<6; i++)
   {
      pTcpHead->destMAC[i] = pDstMAC[i];
      pTcpHead->srcMAC[i] = pMyMAC[i];
   };
   pTcpHead->type[0] = 0x08;
   pTcpHead->type[1] = 0x00;
   pTcpHead->hdrLen = 0x45;
   pTcpHead->diffServ = 0x00;
   pTcpHead->dataID = reverseWORD(dID);
   if(++dID >= 0xFFFF) dID = 1000;
   pTcpHead->flags[0] = 0x40;
   pTcpHead->flags[1] = 0x00;
   pTcpHead->ttl = 0x80;
   pTcpHead->protocol = 0x06;
   pTcpHead->hdrCRC = 0x0000;
   pTcpHead->clientIP = getMyClientIP();
   pTcpHead->targIP = item.trgIP;
   pTcpHead->srcPort = reverseWORD(SOURCE_PORT);         // ???????????????????????????====================
   pTcpHead->destPort = reverseWORD(item.trgPort);       // ???????????????????????????====================

   pTcpHead->seqNumb = reverseDWORD(item.seq);
   pTcpHead->ackNumb = reverseDWORD(item.ack);
   SetTcpFlagTX(item.flag);

   pTcpHead->dataOffs = 0x80;
   pTcpHead->tcpFlgs = item.flag;
   pTcpHead->windSz = reverseWORD(0xFFFF);
   pTcpHead->crcData = 0x0000;
   pTcpHead->urgentPtr = 0x0000;

   pTcpHead->maxKindSS = 0x02;
   pTcpHead->maxLenSS = 0x04;
   pTcpHead->maxValSS = reverseWORD(1460);
   pTcpHead->tcpSackPrm = 0x04;
   pTcpHead->tcpSackLen = 0x02;
   pTcpHead->tcpKind = 0x08;
   pTcpHead->tcpLen = 0xA0;

   OutTStampNmb = getTimeStamp();
   pTcpHead->timeStamp = reverseDWORD(OutTStampNmb);
   pTcpHead->timeEcho = reverseDWORD(OutTEchoNmb);
   pTcpHead->tnop = 0x01;
   pTcpHead->winScaleKind = 0x03;
   pTcpHead->winScaleLen = 0x03;
   pTcpHead->winScaleMult = 0x08;

   uint16_t dateOffset = (pTcpHead->dataOffs>>4)*4;
   pTcpHead->totalLen = reverseWORD(ndlen + IP_HEAD_LEN + dateOffset); // (D+20+9*4)+14 = D+56

   uint8_t *pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN]);
   pTcpHead->hdrCRC = GetIpCRC(pB, IP_HEAD_LEN);

   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset]);
   for(uint16_t i=0; i<item.len; i++)
   {
      pB[i] = item.data[i];
   };
   if(item.len & 0x0001) pB[item.len] = 0;

   /** ------------ TCP PSEUDO HEADER ------------- **/
   sTcpPseudo *pPseudoHdrTcp = 0;
   uint16_t pseudoCntr = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;
   pPseudoHdrTcp = (sTcpPseudo*)(&pBFF[pseudoCntr]); // pos D+70
   pPseudoHdrTcp->clientIP = pTcpHead->clientIP;
   pPseudoHdrTcp->targIP = pTcpHead->targIP;
   pPseudoHdrTcp->reserved = 0x00;
   pPseudoHdrTcp->protocol = pTcpHead->protocol;
   pPseudoHdrTcp->length = reverseWORD(ndlen + dateOffset);
   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN]);
   pTcpHead->crcData = GetIpCRC(pB, ndlen + dateOffset + 12);
   res = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;

   /**********************************************/

   return res;
}

uint16_t setTermTCP(STcpItem &item, uint8_t *pBFF)
{
   uint16_t res = 0;
   sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&pBFF[0]);
   /**********************************************/
   static uint16_t dID = 1000;
   uint16_t ndlen = item.len;
   if(item.len & 0x0001) ++ndlen;
   uint8_t *pMyMAC = getClientMacPtr();
   uint8_t *pDstMAC = getRouterMacPtr();
   for(uint8_t i=0; i<6; i++)
   {
      pTcpHead->destMAC[i] = pDstMAC[i];
      pTcpHead->srcMAC[i] = pMyMAC[i];
   };
   pTcpHead->type[0] = 0x08;
   pTcpHead->type[1] = 0x00;
   pTcpHead->hdrLen = 0x45;
   pTcpHead->diffServ = 0x00;
   pTcpHead->dataID = reverseWORD(dID);
   if(++dID >= 0xFFFF) dID = 1000;
   pTcpHead->flags[0] = 0x40;
   pTcpHead->flags[1] = 0x00;
   pTcpHead->ttl = 0x80;
   pTcpHead->protocol = 0x06;
   pTcpHead->hdrCRC = 0x0000;
   pTcpHead->clientIP = getMyClientIP();
   pTcpHead->targIP = item.trgIP;
   pTcpHead->srcPort = reverseWORD(SOURCE_PORT);         // ???????????????????????????====================
   pTcpHead->destPort = reverseWORD(item.trgPort);       // ???????????????????????????====================

   pTcpHead->seqNumb = reverseDWORD(item.seq);
   pTcpHead->ackNumb = reverseDWORD(item.ack);
   SetTcpFlagTX(item.flag);

   pTcpHead->dataOffs = 0x80;
   pTcpHead->tcpFlgs = item.flag;
   pTcpHead->windSz = reverseWORD(0xFFFF);
   pTcpHead->crcData = 0x0000;
   pTcpHead->urgentPtr = 0x0000;

   pTcpHead->NOP1 = 0x01;
   pTcpHead->NOP2 = 0x01;

   pTcpHead->tcpKind = 0x08;
   pTcpHead->tcpLen = 0xA0;

   OutTStampNmb = getTimeStamp();
   pTcpHead->timeStamp = reverseDWORD(OutTStampNmb);
   pTcpHead->timeEcho = reverseDWORD(OutTEchoNmb);

   uint16_t dateOffset = (pTcpHead->dataOffs>>4)*4;
   pTcpHead->totalLen = reverseWORD(ndlen + IP_HEAD_LEN + dateOffset); // (D+20+9*4)+14 = D+56

   uint8_t *pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN]);
   pTcpHead->hdrCRC = GetIpCRC(pB, IP_HEAD_LEN);

   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset]);
   for(uint16_t i=0; i<item.len; i++)
   {
      pB[i] = item.data[i];
   };
   if(item.len & 0x0001) pB[item.len] = 0;

   /** ------------ TCP PSEUDO HEADER ------------- **/
   sTcpPseudo *pPseudoHdrTcp = 0;
   uint16_t pseudoCntr = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;
   pPseudoHdrTcp = (sTcpPseudo*)(&pBFF[pseudoCntr]); // pos D+70
   pPseudoHdrTcp->clientIP = pTcpHead->clientIP;
   pPseudoHdrTcp->targIP = pTcpHead->targIP;
   pPseudoHdrTcp->reserved = 0x00;
   pPseudoHdrTcp->protocol = pTcpHead->protocol;
   pPseudoHdrTcp->length = reverseWORD(ndlen + dateOffset);
   pB = (uint8_t *)(&pBFF[HARD_HEAD_LEN + IP_HEAD_LEN]);
   pTcpHead->crcData = GetIpCRC(pB, ndlen + dateOffset + 12);
   res = ndlen + HARD_HEAD_LEN + IP_HEAD_LEN + dateOffset;

   /**********************************************/

   return res;
}

char ParseSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item)
{
   //SWO_PrintString("char ParseSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item)\n");
   /**
   itemRx.trgPort = 80;
   itemRx.seq = 0;
   itemRx.ack = SequenceNumb + 1;
   itemRx.timeSt = 0;
   itemRx.echoSt = 0;
   itemRx.flag = TCP_SYN_FLG | TCP_ACK_FLG;
   itemRx.len = 0;
   itemRx.data = 0;
   **/
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         else SWO_PrintString("type OK\n");
         if(pTcpHead->tcpFlgs != (uint8_t)(TCP_SYN_FLG | TCP_ACK_FLG)) res = FALSE;    // if not SYN, ACK
         else SWO_PrintString("TCP flag OK\n");
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         else SWO_PrintString("my IP OK\n");
         if(pTcpHead->clientIP != getSiteIP()) res = FALSE;                            // if not site IP
         else SWO_PrintString("site IP OK\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         else SWO_PrintString("SR PORT OK\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         else SWO_PrintString("TR PORT OK\n");
         //if(pTcpHead->ackNumb != reverseDWORD(item.ack)) res = FALSE;
         //else SWO_PrintString("Ack OK\n");
         if(res)
         {
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;
            //else SWO_PrintString("hdr CRC OK!!!\n");

            sTcpPseudo *pPseudo = (sTcpPseudo *)(&date[len]);
            pPseudo->clientIP = pTcpHead->clientIP;
            pPseudo->targIP = pTcpHead->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pTcpHead->protocol;
            pPseudo->length = reverseWORD(len-34);//(pTcpHead->dataOffs>>4)*4;
            pPtr = (uint8_t *)(&date[34]);
            if(dtCrc != GetIpCRC(pPtr, len+12-34))
            {
//               sprintf(txt, "pPseudo->length: %d, CRC len: %d\n", pPseudo->length, (len+12-34));
//               SWO_PrintString(txt);
//               uint16_t tcrc = GetIpCRC(pPtr, len+12-34);
               res = FALSE;
//               sprintf(txt, "rxCRC: %04X, cmpCRC: %04X\n", dtCrc, tcrc);
//               SWO_PrintString(txt);
            }
            else
            {

               item.seq = reverseDWORD(pTcpHead->seqNumb);
               item.ack = reverseDWORD(pTcpHead->ackNumb);
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;
               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, 0);

               //sprintf(txt, "TCP SYN-ACK RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
               //SWO_PrintString(txt);
            };
            //else SWO_PrintString("date CRC OK!!!\n");

            if(res)
            {
               //setAddrIP(pTcpHead->yourIP, pTcpHead->subMask, pTcpHead->routerIP, pTcpHead->serverIP);
            };
         };
      };
   };
   return res;
}

char ParseHTTP(uint8_t *date, uint16_t len, STcpItem &item)
{
   //SWO_PrintString("char ParseHTTP(uint8_t *date, uint16_t len, STcpItem &item)\n");
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         //else SWO_PrintString("TCP type OK!!!\n");
         //if(pTcpHead->tcpFlgs != (uint8_t)(TCP_SYN_FLG | TCP_ACK_FLG)) res = FALSE;    // if not SYN, ACK
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         //else SWO_PrintString("targ IP OK!!!\n");
         if(pTcpHead->clientIP != getSiteIP()) res = FALSE;                            // if not site IP
         //else SWO_PrintString("client IP OK!!!\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         //else SWO_PrintString("targ port OK!!!\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         //else SWO_PrintString("source port OK!!!\n");
         //if(pTcpHead->ackNumb != reverseDWORD(item.ack)) res = FALSE;
         //else SWO_PrintString("ack numb OK!!!\n");


         if(res)
         {
            item.seq = reverseDWORD(pTcpHead->seqNumb);
            item.ack = reverseDWORD(pTcpHead->ackNumb);
//            sprintf(txt, "TCP HTTP RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
//            SWO_PrintString(txt);
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;
            //else SWO_PrintString("hdr CRC OK!!!\n");

            sTcpPseudo *pPseudo = 0;
            if(len & 1)
            {
               pPseudo = (sTcpPseudo *)(&date[len+1]);
               date[len] = 0;
            }
            else
            {
               pPseudo = (sTcpPseudo *)(&date[len]);
            };
            pPseudo->clientIP = pTcpHead->clientIP;
            pPseudo->targIP = pTcpHead->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pTcpHead->protocol;
            uint16_t clen = reverseWORD(pTcpHead->totalLen);   // (len - 14) = 535  len: 549
            pPseudo->length = reverseWORD(clen-20);            // 515
            pPtr = (uint8_t *)(&date[34]);

            uint16_t CRClen = 0;
            if(len > (clen + 34 - 20)) CRClen = len - 34 + 12; // 527
            else CRClen = clen+12-20;                          // 535+12-20 = 527
            uint16_t tmpCrc = 0;
            if(len & 1) tmpCrc = GetIpCRC(pPtr, CRClen+1);
            else tmpCrc = GetIpCRC(pPtr, CRClen);

            if(dtCrc != tmpCrc)
            {
               res = FALSE;
//               sprintf(txt, "len: %d, pPseudo->length: %d, CRC len: %d\n", len, reverseWORD(pPseudo->length), CRClen);
//               SWO_PrintString(txt);
//               sprintf(txt, "rxCRC: %04X, cmpCRC: %04X\n", dtCrc, tmpCrc);
//               SWO_PrintString(txt);
//               sprintf(txt, "   CRC ERROR TCP HTTP RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
//               SWO_PrintString(txt);
               //showNetPack(date, len, 0);
            }
            else
            {
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;
               item.flag = pTcpHead->tcpFlgs;
               uint16_t deltha = 34 + (pTcpHead->dataOffs>>4)*4;
               if(len > deltha) item.len = (reverseWORD(pTcpHead->totalLen)+14) - deltha;
               else item.len = 0;

               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, item.len);
               ExtractTerminalAddress(date, len);
               uint16_t dateOffs = (uint16_t)(((pTcpHead->dataOffs>>4) & 0x0F)*4 + 34);
               if(len > dateOffs)
               {
                  sendTcpACK(len - dateOffs);
//                  showNetPackASCII(date, len, 0);
               };
//               sprintf(txt, "dataOffs: %02X, offs: %u, deltha: %u, item.len: %u, len: %u\n", (unsigned int)pTcpHead->dataOffs, (unsigned int)((pTcpHead->dataOffs>>4)*4), (unsigned int)deltha, (unsigned int)item.len, (unsigned int)len);
//               SWO_PrintString(txt);
//               sprintf(txt, "CRC OK TCP HTTP RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
//               SWO_PrintString(txt);
               //showNetPack(date, len, 0);
            };
         };
      };
   };
   return res;
}

char ExtractTerminalAddress(uint8_t *pB, uint16_t len)
{
   SWO_PrintString("ExtractTerminalAddr\n");
   char txt[100];
   char res = FALSE;
   char tmp1 = FALSE;
   char tmp2 = FALSE;
   uint16_t pos = 0;
   for(uint16_t i=52; i<len-18; i++)
   {
      if((pB[i] == '<') && (pB[i+17] == '>'))
      {
         if((pB[i+1] == 'T') && (pB[i+2] == 'E') && (pB[i+3] == 'R') && (pB[i+4] == 'M')
            && (pB[i+5] == 'I') && (pB[i+6] == 'N') && (pB[i+7] == 'A') && (pB[i+8] == 'L')
            && (pB[i+9] == ' ') && (pB[i+10] == 'A') && (pB[i+11] == 'D') && (pB[i+12] == 'D')
            && (pB[i+13] == 'R') && (pB[i+14] == 'E') && (pB[i+15] == 'S') && (pB[i+16] == 'S'))
         {
            pos = i + 18;

            //sprintf(txt, "pos 1: %u\n", (unsigned int)pos);
            //SWO_PrintString(txt);

            break;
         };
      };
   };
   if(pos > 0)
   {
      uint16_t pos1 = 0;
      uint16_t pos2 = 0;
      for(uint16_t i=pos; i<pos+3; i++)
      {
         if((pB[i] == '<') && (pB[i+8] == '>'))
         {
            if((pB[i+1] == 'T') && (pB[i+2] == 'a') && (pB[i+3] == 'd') && (pB[i+4] == 'd')
               && (pB[i+5] == 'r') && (pB[i+6] == 'I') && (pB[i+7] == 'P'))
               {
                  pos1 = i + 9;

                  //sprintf(txt, "pos 2: %u\n", (unsigned int)pos1);
                  //SWO_PrintString(txt);
                  //tmp1 = TRUE;
                  break;
               };
         };
      };
      for(uint16_t i=pos1; i<pos1+16; i++)
      {
         if((pB[i] == '<') && (pB[i+9] == '>'))
         {
            if((pB[i+1] == '/') && (pB[i+2] == 'T') && (pB[i+3] == 'a') && (pB[i+4] == 'd') && (pB[i+5] == 'd')
               && (pB[i+6] == 'r') && (pB[i+7] == 'I') && (pB[i+8] == 'P'))
               {
                  pos2 = i;

                  //sprintf(txt, "pos 3: %u\n", (unsigned int)pos2);
                  //SWO_PrintString(txt);
                  //tmp2 = TRUE;
                  break;
               };
         };
      };
      if((pos1 > 0) && (pos2 > 0) && (pos2 > pos1))
      {
         uint8_t *pDT = new uint8_t[pos2-pos1+1];
         uint16_t cntd = 0;
         for(uint16_t i=pos1; i<pos2; i++)
         {
            pDT[cntd++] = pB[i];
         };
         pDT[cntd] = 0x00;

//d1 = (uint8_t)(ip>>24);
//d2 = (uint8_t)(ip>>16);
//d3 = (uint8_t)(ip>>8);
//d4 = (uint8_t)(ip>>0);
#ifdef LOCAL_IP
//         TerminalIP = ((uint16_t)tempDebugIP0)<<24;
//         TerminalIP |= ((uint16_t)tempDebugIP1)<<16;
//         TerminalIP |= ((uint16_t)tempDebugIP2)<<8;
//         TerminalIP |= ((uint16_t)tempDebugIP3)<<0;

//         m_TerminalIP = ((uint16_t)192)<<24;
//         m_TerminalIP |= ((uint16_t)168)<<16;
//         m_TerminalIP |= ((uint16_t)3)<<8;
//         m_TerminalIP |= ((uint16_t)91)<<0;
#else
         TerminalIP = onExtractIP(pDT, cntd);
#endif // LOCAL_IP
         //m_TerminalIP = this->onExtractIP(pDT, cntd);

         pos = pos2 + 8;
         tmp1 = TRUE;
         delete [] pDT;
      };

//      pos1 = 0;
//      pos2 = 0;
//      for(uint16_t i=pos; i<pos+3; i++)
//      {
//         if((pB[i] == '<') && (pB[i+6] == '>'))
//         {
//            if((pB[i+1] == 'T') && (pB[i+2] == 'p') && (pB[i+3] == 'o') && (pB[i+4] == 'r')
//               && (pB[i+5] == 't'))
//               {
//                  pos1 = i + 7;
//                  break;
//               };
//         };
//      };
//      for(uint16_t i=pos1; i<pos1+7; i++)
//      {
//         if((pB[i] == '<') && (pB[i+7] == '>'))
//         {
//            if((pB[i+1] == '/') && (pB[i+2] == 'T') && (pB[i+3] == 'p') && (pB[i+4] == 'o') && (pB[i+5] == 'r')
//               && (pB[i+6] == 't'))
//               {
//                  pos2 = i;
//                  break;
//               };
//         };
//      };
//      if((pos1 > 0) && (pos2 > 0) && (pos2 > pos1))
//      {
//         uint8_t *pDT = new uint8_t[pos2-pos1+1];
//         uint16_t cntd = 0;
//         for(uint16_t i=pos1; i<pos2; i++)
//         {
//            pDT[cntd++] = pB[i];
//         };
//         pDT[cntd] = 0x00;
//         //m_TerminalPort = this->onExtractPort(pDT, cntd);
//         TerminalPort = 40000;
//         tmp2 = TRUE;
//         delete [] pDT;
//      };
      TerminalPort = 40000;
   };
   //if(tmp1 && tmp2)
   if(tmp1)
   {
//      SWO_PrintString((char*)("Terminal IP: "));
//      showIP(reverseDWORD(TerminalIP));
      res = TRUE;
   };
   return res;
}

uint32_t onExtractIP(uint8_t *b, uint16_t len)
{
   SWO_PrintString("ExtractIP\n");
   uint32_t res = 0;
	signed char p = (signed char)(len - 1);
	uint8_t tmp = 0;
	char flg = 0;
	for(uint8_t n=0; n<4; n++)
	{
		tmp = 0;
		flg = 0;
		for(uint8_t i=0; i<3 && p >= 0; i++)
		{
			if(b[p] == 0)
			{
				n = 0;
				--i;
				tmp = 0;
				--p;
			}
			else
			{
				if((b[p] >= 0x30) && (b[p] <= 0x39))
				{
					switch(i)
					{
					case 0:
						tmp = b[p] - 0x30;
					break;
					case 1:
						tmp += (b[p] - 0x30)*10;
					break;
					case 2:
						tmp += (b[p] - 0x30)*100;
					break;
					};
					--p;
				};
				if(b[p] == '.')
				{
					--p;
					flg = 1;
					break;
				};
				if(p < 0)
				{
					flg = 1;
					break;
				};
			};
		};
		if(flg == 1)
		{
			flg = 0;
			switch(n)
			{
			case 0:
				res = (uint32_t)tmp<<24 & 0xFF000000;
			break;
			case 1:
				res |= (uint32_t)tmp<<16 & 0x00FF0000;
			break;
			case 2:
				res |= (uint32_t)tmp<<8 & 0x0000FF00;
			break;
			case 3:
				res |= (uint32_t)tmp & 0x000000FF;
			break;
			};
		};
	};
	return res;
}

char getTerminalAddress(uint32_t &IP, uint16_t &port)
{
   //SWO_PrintString("char getTerminalAddress(uint32_t &IP, uint16_t &port)\n");
   if((TerminalIP != 0) && (TerminalPort == 40000))
   {
      IP = TerminalIP;
      port = TerminalPort;
      return TRUE;
   }
   else return FALSE;
}

char ParseTermSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item)
{
   //SWO_PrintString("char ParseSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item)\n");
   /**
   itemRx.trgPort = 80;
   itemRx.seq = 0;
   itemRx.ack = SequenceNumb + 1;
   itemRx.timeSt = 0;
   itemRx.echoSt = 0;
   itemRx.flag = TCP_SYN_FLG | TCP_ACK_FLG;
   itemRx.len = 0;
   itemRx.data = 0;
   **/
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         if(res == FALSE) SWO_PrintString("type ERROR\n");
         if(pTcpHead->tcpFlgs != (uint8_t)(TCP_SYN_FLG | TCP_ACK_FLG)) res = FALSE;    // if not SYN, ACK
         if(res == FALSE) SWO_PrintString("flag ERROR\n");
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         if(res == FALSE) SWO_PrintString("myIP ERROR\n");
         if(pTcpHead->clientIP != item.trgIP) res = FALSE;                            // if not site IP
         if(res == FALSE) SWO_PrintString("targ IP ERROR\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         if(res == FALSE) SWO_PrintString("src port ERROR\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         if(res == FALSE) SWO_PrintString("trg port ERROR\n");
//         if(pTcpHead->ackNumb != reverseDWORD(item.ack)) res = FALSE;
//         if(res == FALSE) SWO_PrintString("ack ERROR\n");
         if(res)
         {
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;
            //else SWO_PrintString("hdr CRC OK!!!\n");

            sTcpPseudo *pPseudo = (sTcpPseudo *)(&date[len]);
            pPseudo->clientIP = pTcpHead->clientIP;
            pPseudo->targIP = pTcpHead->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pTcpHead->protocol;
            pPseudo->length = reverseWORD(len-34);//(pTcpHead->dataOffs>>4)*4;
            pPtr = (uint8_t *)(&date[34]);
            //if(dtCrc != GetIpCRC(pPtr, len+12-34))
            //{
//               sprintf(txt, "pPseudo->length: %d, CRC len: %d\n", pPseudo->length, (len+12-34));
//               SWO_PrintString(txt);
//               uint16_t tcrc = GetIpCRC(pPtr, len+12-34);

               ////res = FALSE;

//               sprintf(txt, "rxCRC: %04X, cmpCRC: %04X\n", dtCrc, tcrc);
//               SWO_PrintString(txt);
            //}
            //else
           //{
               item.len = 0;
               item.seq = reverseDWORD(pTcpHead->seqNumb);
               item.ack = reverseDWORD(pTcpHead->ackNumb);
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;
               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, item.len);

               //sprintf(txt, "TCP SYN-ACK RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
               //SWO_PrintString(txt);
            //};
            if(res == FALSE) SWO_PrintString("date CRC ERROR\n");
            //else SWO_PrintString("date CRC OK!!!\n");

            if(res)
            {
               //setAddrIP(pTcpHead->yourIP, pTcpHead->subMask, pTcpHead->routerIP, pTcpHead->serverIP);
            };
         };
      };
   };
   return res;
}

char ParseTermFIN_ACK(uint8_t *date, uint16_t len, STcpItem &item)
{
//   SWO_PrintString("ParseFIN_ACK\n");
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         else SWO_PrintString("type OK\n");
         if(pTcpHead->tcpFlgs != (uint8_t)(TCP_FIN_FLG | TCP_ACK_FLG)) res = FALSE;    // if not SYN, ACK
         else SWO_PrintString("flag OK\n");
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         else SWO_PrintString("myIP OK\n");
         if(pTcpHead->clientIP != item.trgIP) res = FALSE;                            // if not site IP
         else SWO_PrintString("targ IP OK\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         else SWO_PrintString("src port OK\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         else SWO_PrintString("trg port OK\n");
//         if(pTcpHead->ackNumb != reverseDWORD(item.ack)) res = FALSE;
//         else SWO_PrintString("ack OK\n");
         if(res)
         {
            SWO_PrintString("RX FIN_ACK OK!!!\n");
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;
            //else SWO_PrintString("hdr CRC OK!!!\n");

//            sTcpPseudo *pPseudo = (sTcpPseudo *)(&date[len]);
//            pPseudo->clientIP = pTcpHead->clientIP;
//            pPseudo->targIP = pTcpHead->targIP;
//            pPseudo->reserved = 0x00;
//            pPseudo->protocol = pTcpHead->protocol;
//            pPseudo->length = reverseWORD(len-34);//(pTcpHead->dataOffs>>4)*4;
//            pPtr = (uint8_t *)(&date[34]);
//            if(dtCrc != GetIpCRC(pPtr, len+12-34))
//            {
//               sprintf(txt, "pPseudo->length: %d, CRC len: %d\n", pPseudo->length, (len+12-34));
//               SWO_PrintString(txt);
//               uint16_t tcrc = GetIpCRC(pPtr, len+12-34);
//               res = FALSE;
//               sprintf(txt, "rxCRC: %04X, cmpCRC: %04X\n", dtCrc, tcrc);
//               SWO_PrintString(txt);
//            }
//            else
//            {
               item.len = 0;
               item.seq = reverseDWORD(pTcpHead->seqNumb);
               item.ack = reverseDWORD(pTcpHead->ackNumb);
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;

               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, item.len);

               //sprintf(txt, "TCP SYN-ACK RX - seq: %u, ack: %u\n", (unsigned int)item.seq, (unsigned int)item.ack);
               //SWO_PrintString(txt);
//            };
//            if(res == FALSE) SWO_PrintString("date CRC ERROR\n");
            //else SWO_PrintString("date CRC OK!!!\n");

//            if(res)
//            {
//               //setAddrIP(pTcpHead->yourIP, pTcpHead->subMask, pTcpHead->routerIP, pTcpHead->serverIP);
//            };
         };
      };
   };
   return res;
}

char ParseTermTcpACK(uint8_t *date, uint16_t len, STcpItem &item)
{
//   SWO_PrintString("Parse <- ACK\n");
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         //else SWO_PrintString("type OK\n");
         if(pTcpHead->tcpFlgs != (uint8_t)(TCP_ACK_FLG))
         {
            res = FALSE;    // if not SYN, ACK
            return res;
         }
         else SWO_PrintString("ACK flag OK\n");
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         else SWO_PrintString("myIP OK\n");
         if(pTcpHead->clientIP != item.trgIP) res = FALSE;                            // if not site IP
         else SWO_PrintString("targ IP OK\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         else SWO_PrintString("src port OK\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         else SWO_PrintString("trg port OK\n");

         if(res)
         {
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20))
            {
               SWO_PrintString("ACK header crc ERROR\n");
               res = FALSE;
               return res;
            }
            else SWO_PrintString("ACK header crc OK\n");

            sTcpPseudo *pPseudo = 0;
            if(len & 1)
            {
               pPseudo = (sTcpPseudo *)(&date[len+1]);
               date[len] = 0;
            }
            else
            {
               pPseudo = (sTcpPseudo *)(&date[len]);
            };
            pPseudo->clientIP = pTcpHead->clientIP;
            pPseudo->targIP = pTcpHead->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pTcpHead->protocol;
            uint16_t clen = reverseWORD(pTcpHead->totalLen);   // (len - 14) = 535  len: 549
            pPseudo->length = reverseWORD(clen-20);            // 515
            pPtr = (uint8_t *)(&date[34]);

            uint16_t CRClen = 0;
            if(len > (clen + 34 - 20)) CRClen = len - 34 + 12; // 527
            else CRClen = clen+12-20;                          // 535+12-20 = 527
            uint16_t tmpCrc = 0;
            if(len & 1) tmpCrc = GetIpCRC(pPtr, CRClen+1);
            else tmpCrc = GetIpCRC(pPtr, CRClen);

            if(dtCrc == tmpCrc)
            {
               SWO_PrintString("ACK date crc OK\n");
               item.seq = reverseDWORD(pTcpHead->seqNumb);
               item.ack = reverseDWORD(pTcpHead->ackNumb);
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;
               item.len = (clen + 0) - (((pTcpHead->dataOffs >> 4) & 0x0F)*4 + 20);
               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, item.len);
            }
            else
            {
               SWO_PrintString("ACK date crc ERROR\n");
               res = FALSE;
            };
         };
      };
   };
   return res;
}

char ParseTermTcpHTTP(uint8_t *date, uint16_t len, STcpItem &item)
{
//   SWO_PrintString("Parse <- HTTP\n");
   char res = FALSE;
   char txt[100];
   if(len >= 60)
   {
      sTcpHeadTX *pTcpHead = (sTcpHeadTX *)(&date[0]);
      if(pTcpHead->protocol == 0x06) // if - TCP
      {
         res = TRUE;
         if((pTcpHead->type[0] != 0x08) || (pTcpHead->type[1] != 0x00)) res = FALSE;   // if not IP type protocol
         //else SWO_PrintString("type OK\n");
         if(pTcpHead->tcpFlgs != (uint8_t)(TCP_PSH_FLG | TCP_ACK_FLG))
         {
            //SWO_PrintString("flag ERROR\n");
            //sprintf(txt, "TCP flg: %02X\n", pTcpHead->tcpFlgs);
            //SWO_PrintString(txt);
            res = FALSE;    // if not SYN, ACK
         }
         //else SWO_PrintString("flag OK\n");
         if(pTcpHead->targIP != getMyClientIP()) res = FALSE;                          // if not my IP
         //else SWO_PrintString("myIP OK\n");
         if(pTcpHead->clientIP != item.trgIP) res = FALSE;                            // if not site IP
         //else SWO_PrintString("targ IP OK\n");
         if(pTcpHead->destPort != reverseWORD(SOURCE_PORT)) res = FALSE;               // if not my port
         //else SWO_PrintString("src port OK\n");
         if(pTcpHead->srcPort != reverseWORD(item.trgPort)) res = FALSE;                  // if not HTTP port
         //else SWO_PrintString("trg port OK\n");

         if(res)
         {
            uint16_t hdCrc = pTcpHead->hdrCRC;
            uint16_t dtCrc = pTcpHead->crcData;
            pTcpHead->hdrCRC = 0x0000;
            pTcpHead->crcData = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20))
            {
               SWO_PrintString("header crc ERROR\n");
               res = FALSE;
               return res;
            }
            //else SWO_PrintString("header crc OK\n");

            sTcpPseudo *pPseudo = 0;
            if(len & 1)
            {
               pPseudo = (sTcpPseudo *)(&date[len+1]);
               date[len] = 0;
            }
            else
            {
               pPseudo = (sTcpPseudo *)(&date[len]);
            };
            pPseudo->clientIP = pTcpHead->clientIP;
            pPseudo->targIP = pTcpHead->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pTcpHead->protocol;
            uint16_t clen = reverseWORD(pTcpHead->totalLen);   // (len - 14) = 535  len: 549
            pPseudo->length = reverseWORD(clen-20);            // 515
            pPtr = (uint8_t *)(&date[34]);

            uint16_t CRClen = 0;
            if(len > (clen + 34 - 20)) CRClen = len - 34 + 12; // 527
            else CRClen = clen+12-20;                          // 535+12-20 = 527
            uint16_t tmpCrc = 0;
            if(len & 1) tmpCrc = GetIpCRC(pPtr, CRClen+1);
            else tmpCrc = GetIpCRC(pPtr, CRClen);

            if(dtCrc == tmpCrc)
            {
               //SWO_PrintString("date crc OK\n");
               item.seq = reverseDWORD(pTcpHead->seqNumb);
               item.ack = reverseDWORD(pTcpHead->ackNumb);
               item.timeSt = pTcpHead->timeStamp;
               item.echoSt = pTcpHead->timeEcho;
               item.len = (clen + 0) - (((pTcpHead->dataOffs >> 4) & 0x0F)*4 + 20);
               SetSeqAckRX(pTcpHead->tcpFlgs, item.seq, item.ack, item.len);
               //showNetPack(date, len, 0);
            }
            else
            {
               SWO_PrintString("date crc ERROR\n");
               res = FALSE;
            };
         };
      };
   };
   return res;
}

CApoClient::CApoClient() :
   m_MyClientNmb(0),
   m_port(0),
   m_IP(0),
   m_Seq(0),
   m_Ack(0),
   m_ResetFlg(FALSE)
{
}

CApoClient::~CApoClient()
{
}

void CApoClient::onInitAddress(uint16_t port, uint32_t IP)
{
   m_port = port;
   m_IP = IP;
}

void CApoClient::onInitSeqAck(uint32_t seq, uint32_t ack)
{
   m_Seq = seq;
   m_Ack = ack;
}

void CApoClient::onInitClientNmb(uint16_t nmb)
{
   m_MyClientNmb = nmb;
}

void CApoClient::onHandleRX(uint8_t *date, uint16_t len)
{

}

char CApoClient::onCheckResetFlg(void)
{
   return m_ResetFlg;
}

void CApoClient::onSendTX(uint8_t *date, uint16_t len)
{
   sendPackForTerminal(date, len);
}

void CApoClient::onSendREG(void)
{
   SWO_PrintString("send->REG\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = REG_MSG_LEN;
   uint8_t *pPtr = new uint8_t[REG_MSG_LEN];
   ItemTX.data = pPtr;
   sREG *pREG = (sREG *)(&pPtr[0]);
   pREG->code[0] = 'R';
   pREG->code[1] = 'e';
   pREG->code[2] = 'g';
   pREG->devNumb = m_MyClientNmb;
   pREG->length = REG_MSG_LEN;
   pREG->mask = LIFT_MASK | CLIENT_MASK;
   pREG->crc32 = 0;
   pREG->crc32 = GetCRC32(pPtr, REG_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_REG);

/**
TX:
0x52   // 0		'R'
0x65   // 1		'e'
0x67   // 2		'g'
0x1C   // 3		mask
0x0C   // 4		len L
0x00   // 5		len H
0x01   // 6		dev numb L
0x00   // 7		dev numb H
0x41   // 8		CRC32-0
0x17   // 9		CRC32-1
0x56   // 10	CRC32-2
0xCB   // 11	CRC32-3
TX.
**/
}

void CApoClient::onSendRQC(uint8_t mask, uint8_t callType)
{
   SWO_PrintString("send->RQC\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = RQC_MSG_LEN;
   uint8_t *pPtr = new uint8_t[RQC_MSG_LEN];
   ItemTX.data = pPtr;
   sRQC *pRQC = (sRQC *)(&pPtr[0]);
   pRQC->code[0] = 'R';
   pRQC->code[1] = 'q';
   pRQC->code[2] = 'C';
   pRQC->devNumb = m_MyClientNmb;
   pRQC->length = RQC_MSG_LEN;
   pRQC->rqType = callType;
   pRQC->mask = mask | CLIENT_MASK;
   pRQC->crc32 = 0;
   pRQC->crc32 = GetCRC32(pPtr, RQC_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_RQC);
}

void CApoClient::onSendSTC(uint8_t mask)
{
   SWO_PrintString("send->STC\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = STC_MSG_LEN;
   uint8_t *pPtr = new uint8_t[STC_MSG_LEN];
   ItemTX.data = pPtr;
   sSTC *pSTC = (sSTC *)(&pPtr[0]);
   pSTC->code[0] = 'S';
   pSTC->code[1] = 't';
   pSTC->code[2] = 'C';
   pSTC->devNumb = m_MyClientNmb;
   pSTC->length = STC_MSG_LEN;
   pSTC->mask = mask | CLIENT_MASK;
   pSTC->crc32 = 0;
   pSTC->crc32 = GetCRC32(pPtr, STC_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_STC);
}

void CApoClient::onSendSPC(uint8_t mask)
{
   SWO_PrintString("send->SPC\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = SPC_MSG_LEN;
   uint8_t *pPtr = new uint8_t[SPC_MSG_LEN];
   ItemTX.data = pPtr;
   sSPC *pSPC = (sSPC *)(&pPtr[0]);
   pSPC->code[0] = 'S';
   pSPC->code[1] = 'p';
   pSPC->code[2] = 'C';
   pSPC->devNumb = m_MyClientNmb;
   pSPC->length = SPC_MSG_LEN;
   pSPC->mask = mask | CLIENT_MASK;
   pSPC->crc32 = 0;
   pSPC->crc32 = GetCRC32(pPtr, SPC_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_SPC);
}

void CApoClient::onSendRST(uint8_t mask)
{
   SWO_PrintString("send->RST\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = RST_MSG_LEN;
   uint8_t *pPtr = new uint8_t[RST_MSG_LEN];
   ItemTX.data = pPtr;
   sRST *pRST = (sRST *)(&pPtr[0]);
   pRST->code[0] = 'R';
   pRST->code[1] = 's';
   pRST->code[2] = 't';
   pRST->devNumb = m_MyClientNmb;
   pRST->length = RST_MSG_LEN;
   pRST->mask = mask | CLIENT_MASK;
   pRST->crc32 = 0;
   pRST->crc32 = GetCRC32(pPtr, RST_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_RST);
}

void CApoClient::onSendMTR(uint32_t flags, uint32_t *metrics)
{
   SWO_PrintString("send->MTR\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = MTR_MSG_LEN;
   uint8_t *pPtr = new uint8_t[MTR_MSG_LEN];
   ItemTX.data = pPtr;
   sMTR *pMTR = (sMTR *)(&pPtr[0]);
   pMTR->code[0] = 'M';
   pMTR->code[1] = 't';
   pMTR->code[2] = 'r';
   pMTR->devNumb = m_MyClientNmb;
   pMTR->length = MTR_MSG_LEN;
   pMTR->mask = LIFT_MASK | CLIENT_MASK;
   pMTR->flags = flags;
   pMTR->metrics[0] = metrics[0];
   pMTR->metrics[1] = metrics[1];
   pMTR->metrics[2] = metrics[2];
   pMTR->metrics[3] = metrics[3];
   pMTR->metrics[4] = metrics[4];
   pMTR->metrics[5] = metrics[5];
   pMTR->metrics[6] = metrics[6];
   pMTR->metrics[7] = metrics[7];
   pMTR->metrics[8] = metrics[8];
   pMTR->metrics[9] = metrics[9];
   pMTR->crc32 = 0;
   pMTR->crc32 = GetCRC32(pPtr, MTR_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
   SetSentEvent(SENT_MTR);
}

void CApoClient::onSendSPX(uint8_t mask, uint8_t *date)
{
   SWO_PrintString("send->SPX\n");
   ItemTX.ack = m_Ack;
   ItemTX.seq = m_Seq;
   ItemTX.flag = TCP_ACK_FLG | TCP_PSH_FLG;
   ItemTX.trgIP = m_IP;
   ItemTX.trgPort = m_port;
   ItemTX.timeSt = getTimeStamp();
   ItemTX.echoSt = 0;
   ItemTX.len = SPX_MSG_LEN;
   uint8_t *pPtr = new uint8_t[SPX_MSG_LEN];
   ItemTX.data = pPtr;
   sSPX *pSPX = (sSPX *)(&pPtr[0]);
   pSPX->code[0] = 'S';
   pSPX->code[1] = 'p';
   pSPX->code[2] = 'x';
   pSPX->devNumb = m_MyClientNmb;
   pSPX->length = SPX_MSG_LEN;
   pSPX->mask = mask | CLIENT_MASK;
   for(uint16_t i=0; i<SPEEX_SZ; i++)
   {
      pSPX->speex[i] = date[i];
   };
   pSPX->crc32 = 0;
   pSPX->crc32 = GetCRC32(pPtr, SPX_MSG_LEN);
   uint8_t *txBff = getNetBuffPtr();
   uint16_t TxLen = setTermTCP(ItemTX, txBff);
   sendPackForTerminal(txBff, TxLen);

   delete [] pPtr;
}




