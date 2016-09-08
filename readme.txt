You have to set the correct memory layout for your device in the linker script.
Please check the FLASH and SRAM length.

e.g.


MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 0x08000   /* 32k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x01000   /*  4k */
}

Stack_Size: 0x800 (2048 Byte)
Heap_Size: 0xC00 (3072 Byte)

---------------------------------------------------------------/
router:
IP: 192.168.0.1
MAC: E8-94-F6-7C-D4-92

Samsung:
MAC: 00-16-EA-A4-4D-82
MAC: 00-13-77-6E-A9-48

----------------------------------------------------------------/
Protocol for MECHANICS PACK
---------------------------
BYTE 0
BYTE 'M'
BYTE 'T'
BYTE 'R'
BYTE FLG0
BYTE FLG1
BYTE FLG2
BYTE FLG3
DWORD MTR0...MTR9 (10*4 BYTE)
BYTE CRC8
BYTE 0
----------------------------

send ARP - get MAC for target

send DHCP - get DnsIP, myIP, routerIP, gateway, netMASK

send DNS - get terminalIP from URL address

work TCP-CONNECTION

event List:

1) Lift Button Press
2) Entr Button Press
3) Mach Button Press
4) Machine Error Event
5) Machine Alarm Event
6) Terminal call Lift
7) Terminal call Entr
8) Terminal call Mach
9) Terminal call Metrics State
10) Terminal call Set Servo State

================================================================/
USART2_RX + DMA1(Channel-4; Stream-5)
USART2_TX + DMA1(Channel-4; Stream-6)

================================================================/
green (blue) LED - TERMINAL IP recieved OK
red (orange) LED - TCP connection to the TERMINAL (SYN, SYN-ACK, ACK) OK => register Client in TERMINAL

================================================================/
registerClientTerminal(port, IP);
       |
sendTermSYN(port, IP);                 =>
ParseTermSYN_ACK(BuffA, lenRx, itemRx) <=
sendTermACK(0, port, IP);              =>

================================================================/
int main(void)
      |
       ----
           |
         while(1)
           |
void SendNetEvent(void);
           |
            ---- pClient->onSendRQC(mask, 0);
           |
            ---- pClient->onSendSTC(mask);
           |
            ---- pClient->onSendSPC(mask); --- CloseSession();
           |
            ---- pClient->onSendRST(mask, 0);
           |
            ---- pClient->onSendMTR(FLG, MTRD);

================================================================/
            RECIEVE DATE FROM TERMINAL
            --------------------------
void EthernetIrqRXA(void); -----
                                |
               enc28j60_recv_packetA(rxBuffA, BUFFRX_LEN); ---
                                                              |
void EthernetIrqRXB(void); -----                              |
                                |                             |
               enc28j60_recv_packetB(rxBuffB, BUFFRX_LEN); ---|
int main(void)                                                |
      |                                                       |
       ----                                     pBuffIP->onAddPack(rxBuffA-B, len);
           |                                                  |
         while(1)         ------------------------------------
           |             |
if(0 < pBuffIP->onCheckBuff()) ---
                                  |
                     char ParseTermTcpHTTP(uint8_t *date, uint16_t len, STcpItem &item);
                                  |
                     uint8_t HandleApoMsg(uint8_t *date, uint16_t len)
                                  |
                                   --- if(pData[] == 'Spx')
                                  |
                                   --- if(pData[] == 'Srv')
                                  |
                                   --- if(pData[] == 'Mtr')
                                  |
                                   --- if(pData[] == 'RqC') --- OpenSession();
                                  |
                                   --- if(pData[] == 'StC') --- OpenSession();
                                  |
                                   --- if(pData[] == 'SpC') --- CloseSession();
                                  |
                                   --- if(pData[] == 'Rst')

================================================================/
void OpenSession(void);
           |
      SessionFlag = TRUE;

void CloseSession(void);
           |
      SessionFlag = FALSE;

================================================================/
if(FALSE == CheckSession()) *Retranslate recieved packs from Internet or Subnet line*

================================================================/
               RECIEVING METRIC DATE
               ---------------------
MECHANIC'S device send T=0.2ms METRICS PACK

void DMA1_Stream5_IRQHandler(void)
                     |
              copyMetricBuff(pBff, pDstBff);

int main(void)
      |
       ----
           |
         while(1)
           |
       if(TRUE == CheckMetricFlag())
                          |
              void ParseMetricsRX(uint8_t *packRX) ---> MetricsRX[i];
              pMtrUSART->onSendTXD(pServoFlags, 50); ---> Send to the MECHANICS


=================================================================/
char threadInit(void);
           |
      if(dynamic IP) ---> sendDhcpDiscover(); sendDhcpRequest();
           |
       sendARP();        (getting MAC for router)
       sendDnsRequest(); (getting SITE-SERVER IP from SITE NANE)

================================================================/
      INIT ADDRESS IP
      ---------------

struct sAddrFLASH         // sAddressFLASH
{

   uint32_t staticIP;      // 04
   uint32_t staticDNS;     // 08
   uint32_t staticADNS;    // 12
   uint32_t staticMSK;     // 16
   uint32_t staticSHL;     // 20
   uint16_t devNumb;			// 22
   uint8_t staticFLG;      // 23
   uint8_t MacNumb[6];     // 29 MAC number
   uint8_t subNetNumb;     // 30
   uint8_t textLen;        // 31
   char text[69];          // 35
};

void InitClientNetAddr(void) -> myAddrBuff[i] - (*pAddrBuff)
                                                      |


void setAddrIP(uint32_t client, uint32_t submask, uint32_t router, uint32_t dhcp)




