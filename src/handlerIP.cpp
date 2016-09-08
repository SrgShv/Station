
#include "handlerIP.h"
#include "struct.h"
#include <stdio.h>

#define TRUE         1
#define FALSE        0

extern uint32_t reverseDWORD(uint32_t d);
extern uint16_t reverseWORD(uint16_t d);
extern uint16_t GetIpCRC(uint8_t *b, uint16_t len);
extern void showNetPack(uint8_t *data, uint16_t len, uint8_t dir);
extern void setAddrIP(uint32_t client, uint32_t submask, uint32_t router, uint32_t dhcp);
extern void showIP(uint32_t ip);
extern uint32_t getMyClientIP(void);
extern uint32_t getMyRouterIP(void);
extern void setRouterMAC(uint8_t *mac);
extern void setSiteIP(uint32_t trmIP);
extern void sendRespARP(uint32_t trgIP, uint8_t *trgMAC);

char ParseOfferDHCP(uint8_t *date, uint16_t len)
{
   char res = FALSE;
   if(len >= 320)
   {
      sDhcpTX_OFFER *pDHCPO = (sDhcpTX_OFFER *)(&date[0]);
      if(pDHCPO->protocol == 0x11) // if - UDP
      {
         res = TRUE;
         if(pDHCPO->protocolType != 0x0008) res = FALSE;                   // if not IP
         if(pDHCPO->transID != reverseDWORD(TRANSACT_ID)) res = FALSE;     // if not correct ID
         if(pDHCPO->magCookie[0] != 0x63) res = FALSE;                     // {0x63,0x82,0x53,0x63}
         if(pDHCPO->magCookie[1] != 0x82) res = FALSE;
         if(pDHCPO->magCookie[2] != 0x53) res = FALSE;
         if(pDHCPO->magCookie[3] != 0x63) res = FALSE;
         if(pDHCPO->DHCP != 0x02) res = FALSE;

         if(res)
         {
            uint16_t hdCrc = pDHCPO->hdrCRC;
            uint16_t dtCrc = pDHCPO->dataCRC;
            pDHCPO->hdrCRC = 0x0000;
            pDHCPO->dataCRC = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;

            sUdpPseudo *pPseudo = (sUdpPseudo *)(&date[len]);
            pPseudo->clientIP = pDHCPO->clientIP;
            pPseudo->targIP = pDHCPO->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pDHCPO->protocol;
            pPseudo->length = pDHCPO->length;
            pPtr = (uint8_t *)(&date[34]);
            if(dtCrc != GetIpCRC(pPtr, len+12-34)) res = FALSE;

            if(res)
            {
               setAddrIP(pDHCPO->yourIP, pDHCPO->subMask, pDHCPO->routerIP, pDHCPO->serverIP);
            };
         };
      };
   };
   return res;
}

char ParseAckDHCP(uint8_t *date, uint16_t len)
{
   char res = FALSE;
   char txt[100];
   if(len >= 320)
   {
      sDhcpTX_ACK *pDHCPO = (sDhcpTX_ACK *)(&date[0]);
      if(pDHCPO->protocol == 0x11) // if - UDP
      {
         res = TRUE;
         if(pDHCPO->protocolType != 0x0008) res = FALSE;                   // if not IP
         if(pDHCPO->transID != reverseDWORD(TRANSACT_ID)) res = FALSE;     // if not correct ID
         if(pDHCPO->magCookie[0] != 0x63) res = FALSE;                     // {0x63,0x82,0x53,0x63}
         if(pDHCPO->magCookie[1] != 0x82) res = FALSE;
         if(pDHCPO->magCookie[2] != 0x53) res = FALSE;
         if(pDHCPO->magCookie[3] != 0x63) res = FALSE;
         if(pDHCPO->DHCP != 0x05) res = FALSE;
         //sprintf(txt, "DHCP: 0x%02X \n", (int)pDHCPO->DHCP);
         //SWO_PrintString(txt);

         if(res)
         {
            uint16_t hdCrc = pDHCPO->hdrCRC;
            uint16_t dtCrc = pDHCPO->dataCRC;
            pDHCPO->hdrCRC = 0x0000;
            pDHCPO->dataCRC = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;

            sUdpPseudo *pPseudo = (sUdpPseudo *)(&date[len]);
            pPseudo->clientIP = pDHCPO->clientIP;
            pPseudo->targIP = pDHCPO->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pDHCPO->protocol;
            pPseudo->length = pDHCPO->length;
            pPtr = (uint8_t *)(&date[34]);
            if(dtCrc != GetIpCRC(pPtr, len+12-34)) res = FALSE;

            if(res)
            {
               setAddrIP(pDHCPO->yourIP, pDHCPO->subMask, pDHCPO->routerIP, pDHCPO->serverIP);
            };
         };
      };
   };
   return res;
}

char ParseARP(uint8_t *date, uint16_t len)
{
   char res = FALSE;
   char txt[100];
   if(len >= 42)
   {
      sArpTX *pARP = (sArpTX *)(&date[0]);
      if((pARP->type[0] == 0x08) && (pARP->type[1] == 0x06)) // if - ARP
      {
         res = TRUE;
         if((pARP->protocol[0] != 0x08) || (pARP->protocol[1] != 0x00)) res = FALSE;                   // if not IP
         if((pARP->htype[0] != 0x00) || (pARP->htype[1] != 0x01)) res = FALSE;
         if(pARP->targIP != getMyClientIP()) res = FALSE;
         if(res == TRUE)
         {
            if((pARP->opcode[0] == 0x00) || (pARP->opcode[1] == 0x01)) sendRespARP(pARP->sendIP, pARP->sendMAC);
            if((pARP->opcode[0] == 0x00) || (pARP->opcode[1] == 0x02)) setRouterMAC((uint8_t *)(&pARP->sendMAC[0]));
         };
         if(pARP->sendIP != getMyRouterIP()) res = FALSE;
         if(res)
         {

         };
      };
   };
   return res;
}

char ParseDNS(uint8_t *date, uint16_t len)
{
   char res = FALSE;
   char txt[100];
   if(len > 54)
   {
      sDnsRQ *pDNS = (sDnsRQ *)(&date[0]);
      if(pDNS->protocol == 0x11) // if - UDP
      {
         res = TRUE;
         if(pDNS->protocolType != 0x0008) res = FALSE;                   // if not IP
         if(pDNS->clientIP != getMyRouterIP()) res = FALSE;
         if(pDNS->targIP != getMyClientIP()) res = FALSE;
         if(pDNS->question != 0x0100) res = FALSE;
         if(pDNS->answer != 0x0100) res = FALSE;
         if(pDNS->srcPort != 0x3500) res = FALSE;

         if(res)
         {
            uint16_t hdCrc = pDNS->hdrCRC;
            uint16_t dtCrc = pDNS->dataCRC;
            pDNS->hdrCRC = 0x0000;
            pDNS->dataCRC = 0x0000;

            uint8_t *pPtr = (uint8_t *)(&date[14]);
            if(hdCrc != GetIpCRC(pPtr, 20)) res = FALSE;

            sUdpPseudo *pPseudo = (sUdpPseudo *)(&date[len]);
            pPseudo->clientIP = pDNS->clientIP;
            pPseudo->targIP = pDNS->targIP;
            pPseudo->reserved = 0x00;
            pPseudo->protocol = pDNS->protocol;
            pPseudo->length = pDNS->length;
            pPtr = (uint8_t *)(&date[34]);
            if(dtCrc != GetIpCRC(pPtr, len+12-34)) res = FALSE;

            if(res)
            {
               uint8_t pos = 0;
               for(uint8_t i=0; i<66; i++)
               {
                  if(pDNS->file[i] == 0x00) break;
                  else ++pos;
               };
               pos += 17;
               uint32_t *pIP = (uint32_t *)(&pDNS->file[pos]);
               setSiteIP(*pIP);
            };
         };
      };
   };
   return res;
}

