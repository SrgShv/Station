#ifndef TCP_H
#define TCP_H

#include <stdint.h>

#define SPEEX_SZ              1000
#define SPEEX_HD              12
#define UDP_HEAD_LEN          42
#define MSG_TYPE_LEN          4
#define SPX_MSG_LEN           (SPEEX_SZ+SPEEX_HD)
#define MTR_MSG_LEN           56
#define SRV_MSG_LEN           21
#define RQC_MSG_LEN           13
#define BCT_MSG_LEN           12
#define STC_MSG_LEN           12
#define SPC_MSG_LEN           12
#define RST_MSG_LEN           12
#define REG_MSG_LEN           12
#define HPSEUDO_LEN           12

#define SENT_REG              12
#define SENT_RQC              13
#define SENT_STC              14
#define SENT_SPC              15
#define SENT_RST              16
#define SENT_MTR              17



extern "C"
{
#include "swo.h"
}

#pragma pack(push,1)
struct STcpItem               //    (26 Byte)
{
   uint32_t seq;              // 00
   uint32_t ack;              // 04
   uint32_t timeSt;           // 08
   uint32_t echoSt;           // 12
   uint32_t trgIP;            // 16
   uint16_t len;              // 20
   uint16_t trgPort;          // 22
   char flag;                 // 24
   uint8_t *data;             // 25
};
#pragma pack(pop)

#pragma pack(push,1)
typedef struct                // sTcpHeadSYN (74 Byte)
{
   uint8_t destMAC[6];        // 00 Destination MAC
   uint8_t srcMAC[6];         // 06 Source MAC
   uint8_t type[2];           // 12 Packet type (ARP, IP...)
   uint8_t hdrLen;            // 14 Header length
   uint8_t diffServ;          // 15 Differentiated Services (00000000)
   uint16_t totalLen;         // 16 Total Length
   uint16_t dataID;           // 18 Identification:                              =20
   uint8_t flags[2];          // 20 Flags (0100 0000 0000 0000) = don't fragment
   uint8_t ttl;               // 22 TTL - 64
   uint8_t protocol;          // 23 Protocol UDP (17), TCP(6)
   uint16_t hdrCRC;           // 24 Header checksum
   uint32_t clientIP;         // 26 Sender IP
   uint32_t targIP;           // 30 Target IP
   uint16_t srcPort;          // 34 Source Port
   uint16_t destPort;         // 36 Destination Port                             =38
   uint32_t seqNumb;          // 38 Sequence number
   uint32_t ackNumb;          // 42 Acknowledgment number
   uint8_t dataOffs;          // 46 Header length - Data offset (0x90 =: 34+9*4 = 70)
   uint8_t tcpFlgs;           // 47 tcpFlags (0x18 - PSH, ACK)
   uint16_t windSz;           // 48 Window size value
   uint16_t crcData;          // 50 CheckSum data (from 34 pos to end + pseudo header)

   uint16_t urgentPtr;        // 52 Urgent pointer: 0                            =54
   uint8_t maxKindSS;         // 54 Maximum segment size kind: (2) 		- Options 0
   uint8_t maxLenSS;          // 55 Maximum segment size length: (4) 	- Options 1
   uint16_t maxValSS;         // 56 Maximum segment size value: (1460) 	- Options 2
   uint8_t tcpSackPrm;        // 58 TCP SACK Permission: (4)
   uint8_t tcpSackLen;        // 59 TCP Length: (2)                              =60
   uint8_t tcpKind;           // 60 TCP Kind: Timestamp (8)
   uint8_t tcpLen;            // 61 TCP Length: (10)
   uint32_t timeStamp;        // 62 Timestamp value:
   uint32_t timeEcho;         // 66 Timestamp echo reply: 0
   uint8_t tnop;              // 70 No-operation (NOP)
   uint8_t winScaleKind;      // 71 Window scale kind: (3)
   uint8_t winScaleLen;       // 72 Window scale length: (3)
   uint8_t winScaleMult;      // 73 Window scale multiplier: 128 (Shift count: 7)
/**
   uint16_t reserved;         // 52 0x0000
   uint16_t NOP;              // 54 0x0101
   uint8_t opCode;            // 56 TimeStamps code (0x08)
   uint8_t Tlen;              // 57 TimeStamps length (0x0a = 10)
   uint32_t StampTime;        // 58 timestamp
   uint32_t StampEcho;        // 62 echoTime + 4 = Data start from pos 66
   uint8_t tnop;              // 66 No-operation (NOP) 0x01
   uint8_t winScaleKind;      // 67 Window scale kind: (3)
   uint8_t winScaleLen;       // 68 Window scale length: (3)
   uint8_t winScaleMult;      // 69 Window scale multiplier: 128 (Shift count: 7)
**/
} sTcpHeadSYN;                 //  Data start from pos 74
#pragma pack(pop)

#pragma pack(push,1)
typedef struct                // sTcpHead (74 Byte)
{
   uint8_t destMAC[6];        // 00 Destination MAC
   uint8_t srcMAC[6];         // 06 Source MAC
   uint8_t type[2];           // 12 Packet type (ARP, IP...)
   uint8_t hdrLen;            // 14 Header length
   uint8_t diffServ;          // 15 Differentiated Services (00000000)
   uint16_t totalLen;         // 16 Total Length
   uint16_t dataID;           // 18 Identification:                              =20
   uint8_t flags[2];          // 20 Flags (0100 0000 0000 0000) = don't fragment
   uint8_t ttl;               // 22 TTL - 64
   uint8_t protocol;          // 23 Protocol UDP (17), TCP(6)
   uint16_t hdrCRC;           // 24 Header checksum
   uint32_t clientIP;         // 26 Sender IP
   uint32_t targIP;           // 30 Target IP
   uint16_t srcPort;          // 34 Source Port
   uint16_t destPort;         // 36 Destination Port                             =38
   uint32_t seqNumb;          // 38 Sequence number
   uint32_t ackNumb;          // 42 Acknowledgment number
   uint8_t dataOffs;          // 46 Header length - Data offset (0x90 =: 34+9*4 = 70)
   uint8_t tcpFlgs;           // 47 tcpFlags (0x18 - PSH, ACK)
   uint16_t windSz;           // 48 Window size value
   uint16_t crcData;          // 50 CheckSum data (from 34 pos to end + pseudo header)
   uint16_t urgentPtr;        // 52 Urgent pointer: 0                            =54
   uint8_t NOP1;              // 54 Maximum segment size kind: (2) 		- Options 0
   uint8_t NOP2;              // 55 Maximum segment size length: (4) 	- Options 1
   uint8_t tcpKind;           // 56 TCP Kind: Timestamp (0x08)
   uint8_t tcpLen;            // 57 TCP Length: (0x0A) 10
   uint32_t timeStamp;        // 58 Timestamp value:
   uint32_t timeEcho;         // 62 Timestamp echo reply: 0
} sTcpHeadTX;                 //  Data start from pos 66
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sTcpPseudo
{
   uint32_t clientIP;      // Client IP
   uint32_t targIP;        // Target IP
   uint8_t reserved;
   uint8_t protocol;
   uint16_t length;         // UDP length (TCP Header+TCP Data)
} sTcpPseudo;
#pragma pack(pop)

/**========================================================**/
#pragma pack(push,1)
typedef struct          // sSPX (212 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
   uint8_t speex[SPEEX_SZ];// Speex pack: 10x20Byte
} sSPX;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sMTR (56 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
   uint32_t flags;         // Sensors flags
   uint32_t metrics[10];   // Metrics data
} sMTR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sSRV (21 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t devFlags;       // Device flags
   uint32_t crc32;         // CRC32
   uint32_t metrics;       // Metrics data
   uint8_t devType;        // Type of device
} sSRV;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sRQC (13 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask "LIFT_MASK (0x10), ENTR_MASK (0x20), MASH_MASK (0x30)"
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number (1...65535)
   uint32_t crc32;         // CRC32
   uint8_t rqType;         // Request type "CALL_TYPE_PASSENGER (1),  CALL_TYPE_REPORT (2), CALL_TYPE_ALARM (4)"
} sRQC;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sBCT (12 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
} sBCT;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sSTC (12 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
} sSTC;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sSPC (12 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
} sSPC;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sRST (12 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
} sRST;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct         // sREG (12 Byte)
{
   char code[3];        // Code of message "Spx, Mtr, Srv, RqC, Bct, StC, SpC, Rst, Reg"
   uint8_t mask;           // Mask
   uint16_t length;         // Length of MSG
   uint16_t devNumb;        // Device(station) number
   uint32_t crc32;         // CRC32
} sREG; // 12 Byte
#pragma pack(pop)


class CApoClient
{
public:
   CApoClient();
   ~CApoClient();

   void onInitAddress(uint16_t port, uint32_t IP);
   void onInitSeqAck(uint32_t seq, uint32_t ack);
   void onHandleRX(uint8_t *date, uint16_t len);
   char onCheckResetFlg(void);
   void onSendTX(uint8_t *date, uint16_t len);
   void onInitClientNmb(uint16_t nmb);
   void onSendREG(void);
   void onSendRQC(uint8_t mask, uint8_t callType);
   void onSendSTC(uint8_t mask);
   void onSendSPC(uint8_t mask);
   void onSendRST(uint8_t mask);
   void onSendMTR(uint32_t flags, uint32_t *metrics);
   void onSendSPX(uint8_t mask, uint8_t *date);
protected:

private:
   uint16_t m_MyClientNmb;
   uint16_t m_port;
   uint32_t m_IP;
   uint32_t m_Seq;
   uint32_t m_Ack;
   char m_ResetFlg;
   STcpItem ItemTX;

};

uint32_t GetCRC32(uint8_t* date, uint16_t len);
uint16_t setTCP(STcpItem &item, uint8_t *pBFF);
char ParseSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item);
char ParseHTTP(uint8_t *date, uint16_t len, STcpItem &item);
char ExtractTerminalAddress(uint8_t *pB, uint16_t len);
uint32_t onExtractIP(uint8_t *b, uint16_t len);
char getTerminalAddress(uint32_t &IP, uint16_t &port);
uint16_t setTermTCP(STcpItem &item, uint8_t *pBFF);
char ParseTermSYN_ACK(uint8_t *date, uint16_t len, STcpItem &item);
char ParseTermFIN_ACK(uint8_t *date, uint16_t len, STcpItem &item);
char ParseTermTcpHTTP(uint8_t *date, uint16_t len, STcpItem &item);
char ParseTermTcpACK(uint8_t *date, uint16_t len, STcpItem &item);
uint16_t setTermSYN(STcpItem &item, uint8_t *pBFF);

#endif // TCP_H
