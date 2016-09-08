#ifndef STRUCT_H
#define STRUCT_H

#define DHCPDSC_SIZE          314
#define DHCPRQ_SIZE           338
#define PSEUDO_HEADER_SIZE    13
#define TRM_REG_SIZE          58

//#define SPEEX_SZ              200
//#define SPEEX_HD              12
//#define UDP_HEAD_LEN          42
//#define MSG_TYPE_LEN          4
//#define SPX_MSG_LEN           (SPEEX_SZ+SPEEX_HD)
//#define MTR_MSG_LEN           56
//#define SRV_MSG_LEN           21
//#define RQC_MSG_LEN           13
//#define BCT_MSG_LEN           12
//#define STC_MSG_LEN           12
//#define SPC_MSG_LEN           12
//#define RST_MSG_LEN           12
//#define REG_MSG_LEN           12
//#define HPSEUDO_LEN           12

#include <stdint.h>

//#pragma pack(push,1)
//struct MyNetAddress
//{
//   uint8_t MyStaticFLG;    // 1
//   uint32_t MyStaticIP;    // 5
//   uint32_t MyStaticDNS;   // 9
//   uint32_t MyStaticADNS;  // 13
//   uint32_t MyStaticMSK;   // 17
//   uint32_t MyStaticSHL;   // 21
//   uint8_t MyMacNumb[6];   // 27
//   uint8_t MySubNetNumb;   // 28
//   uint16_t MyDeviceNumb;   // 30
//   uint8_t textLen;        // 31
//   char *text;          // 35 [70]
//};
//#pragma pack(pop)

#define URL_TEXT_LEN    48

#pragma pack(push,1)
struct sAddrFLASH         // sAddressFLASH
{

   uint32_t staticIP;         // 04
   uint32_t staticDNS;        // 08
   uint32_t staticADNS;       // 12
   uint32_t staticMSK;        // 16
   uint32_t staticSHL;        // 20
   uint16_t devNumb;			   // 22
   uint8_t staticFLG;         // 23
   uint8_t MacNumb[6];        // 29 MAC number
   uint8_t subNetNumb;        // 30
   char text[URL_TEXT_LEN];   // 78 URL name
   char atext[URL_TEXT_LEN];  // 126 URL alter name
};
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sUdpPseudo
{
   uint32_t clientIP;      // Client IP
   uint32_t targIP;        // Target IP
   uint8_t reserved;
   uint8_t protocol;
   uint16_t length;         // UDP length (TCP Header+TCP Data)
} sUdpPseudo;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          // sArpTX
{
   uint8_t destMAC[6];     // Destination MAC
   uint8_t srcMAC[6];      // Source MAC
   uint8_t type[2];        // Packet type (ARP, IP...)
   uint8_t htype[2];       // Hardware type (Ethernet=0x01)
   uint8_t protocol[2];    // Protocol type (IP=0x8000)
   uint8_t hsize;          // Hardware (MAC) size (6)
   uint8_t psize;          // Protocol size (4)
   uint8_t opcode[2];      // Opcode request (0x0001)
   uint8_t sendMAC[6];     // Sender MAC
   uint32_t sendIP;        // Sender IP
   uint8_t targMAC[6];     // Target MAC
   uint32_t targIP;        // Target IP
} sArpTX;               // 42 Byte
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          	// sDhcpTX_DISC (length 314 Byte)
{
   uint8_t destMAC[6];     // Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // Source MAC (0x00...0x00)
   uint16_t protocolType; 	// Packet type (ARP, IP...) 0x0800
   uint8_t hdrLen;         // Header length 0x45
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;      // Total Length: 303 from pos 14 to end
   uint16_t dataID;        // Identification: number of datagram fragment
   uint8_t flags[2];       // Flags (0000 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17) 0x11
   uint16_t hdrCRC;        // Header checksum (from pos 14 to pos 33)
   uint32_t clientIP;      // Sender IP (0x00...0x00)
   uint32_t targIP;        // Target IP (0xFF...0xFF)
   uint16_t srcPort;       // Source Port (0x0044)
   uint16_t destPort;      // Destination Port (0x0043)
   uint16_t length;        // Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
   uint8_t msgType;        // Message type: Boot request (1)
   uint8_t hrdType;        // Hardware type: Ethernet (1)
   uint8_t addrLen;        // Hardware address length: (6)
   uint8_t hops;           // Hops: 0
   uint32_t transID;       // Transaction ID:
   uint16_t secElps;       // Seconds elapsed: 0
   uint16_t broadCast;     // Broadcast flags: broadcast(0x8000)
   uint32_t senderIP;      // Sender IP address (0)
   uint32_t yourIP;        // Your (Client) IP address (0)
   uint32_t nserverIP;     // Next server IP address (0)
   uint32_t relayIP;       // Gateway IP Address (0)
   uint8_t clntMAC[6];     // Client MAC address (0...0)
   uint32_t file[50];      // file (0...0)
   uint8_t endfile[2];     // file (0...0)
   uint8_t magCookie[4];   // Magic cookie: (from pos: 278 - {0x63,0x82,0x53,0x63})
   uint8_t msgType1;      	// Options: 53 - DHCP Message type (0x35)
   uint8_t length0;      	// Options: Length - 0x01
   uint8_t DHCP;      	   // Options: DHCP - Discover 0x01, Request 0x03
   uint8_t autoConf;      	// Options: 116 - DHCP Auto-Configuration (0x74)
   uint8_t length1;      	// Options: Length - 0x01
   uint8_t autoConf1;      // Options: AutoConfigure 0x01
   uint8_t clientID;       // Client identifier (0x3D)
   uint8_t length2;        // Length: 0x07
   uint8_t hrdType1;       // Hardware type: Ethernet (1)
   uint8_t clntMAC1[6];    // Client MAC address (0...0)
   uint8_t hostName;       // Host Name
   uint8_t length3;        // Length: 9
   char name[9];           // "ApoClient"
   uint8_t listRQ;         // Options: 55 - Parameter request list (0x37)
   uint8_t length4;        // Length: 0x03
   uint8_t subMsk;         // Parameter request list Item: (0x01) Subnet Mask
   uint8_t router;         // Parameter request list Item: (0x03) Router
   uint8_t dns;            // Parameter request list Item: (0x06) Domain Name Server
   uint8_t optEnd;         // Option End: 255 (0xFF)
} sDhcpTX_DISC;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          	// sDhcpTX_OFFER (length 320+270 Byte)
{
   uint8_t destMAC[6];     // Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // Source MAC (0x00...0x00)
   uint16_t protocolType; 	// Packet type (ARP, IP...) 0x0800
   uint8_t hdrLen;         // Header length 0x45
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;      // Total Length: 303 from pos 14 to end
   uint16_t dataID;        // Identification: number of datagram fragment
   uint8_t flags[2];       // Flags (0000 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17) 0x11
   uint16_t hdrCRC;        // Header checksum (from pos 14 to pos 33)
   uint32_t clientIP;      // Sender IP (0x00...0x00)
   uint32_t targIP;        // Target IP (0xFF...0xFF)
   uint16_t srcPort;       // Source Port (0x0044)
   uint16_t destPort;      // Destination Port (0x0043)
   uint16_t length;        // Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
   uint8_t msgType;        // Message type: Boot request (1)
   uint8_t hrdType;        // Hardware type: Ethernet (1)
   uint8_t addrLen;        // Hardware address length: (6)
   uint8_t hops;           // Hops: 0
   uint32_t transID;       // Transaction ID:
   uint16_t secElps;       // Seconds elapsed: 0
   uint16_t broadCast;     // Broadcast flags: broadcast(0x8000)
   uint32_t senderIP;      // Sender IP address (0)
   uint32_t yourIP;        // Your (Client) IP address (0)
   uint32_t nserverIP;     // Next server IP address (0)
   uint32_t relayIP;       // Gateway IP Address (0)
   uint8_t clntMAC[6];     // Client MAC address (0...0)
   uint32_t file[50];      // file (0...0)
   uint8_t endfile[2];     // file (0...0)
   uint8_t magCookie[4];   // Magic cookie: (from pos: 278 - {0x63,0x82,0x53,0x63})
   uint8_t msgType1;     	// Options: 53 - DHCP Message type (0x35)
   uint8_t length0;      	// Options: Length - 0x01
   uint8_t DHCP;      	   // Options: DHCP - Offer 0x02, Discover 0x01, Request 0x03
   uint8_t DHCPsrvIP;      // Options: DHCP server Identifier (0x36)
   uint8_t length1;      	// Options: Length - 0x04
   uint32_t serverIP;      // DHCP server IP address (c0:a8:00:01)
   uint8_t leaseTimeIP;    // Options: IP addres lease time (0x33)
   uint8_t length2;        // Length: 0x04
   uint32_t leaseTime;     // DHCP server lease Time (0x0000012C) (300sec, 5min)
   uint8_t subMaskOp;      // Subnet mask (1)
   uint8_t length3;        // Length: 0x04
   uint32_t subMask;       // Subnet mask (255.255.255.0)
   uint8_t routerOp;       // Router (0x03)
   uint8_t length4;        // Length: 0x04
   uint32_t routerIP;      // Router IP address (c0:a8:00:01)
   uint8_t dnsOp;          // Parameter request list Item: (0x06) Domain Name Server
   uint8_t length5;        // Length: 0x08
   uint32_t dnsIP;         // DNS server IP address (c0:a8:00:01)
   uint32_t adnsIP;        // DHCP server IP address (08:08:08:08)
   uint8_t optEnd;         // Option End: 255 (0xFF)
} sDhcpTX_OFFER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          	// sDhcpTX_RQ (length 338 Byte)
{
   uint8_t destMAC[6];     // Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // Source MAC (0x00...0x00)
   uint16_t protocolType; 	// Packet type (ARP, IP...) 0x0800
   uint8_t hdrLen;         // Header length 0x45
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;      // Total Length: 303 from pos 14 to end
   uint16_t dataID;        // Identification: number of datagram fragment
   uint8_t flags[2];       // Flags (0000 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17) 0x11
   uint16_t hdrCRC;        // Header checksum (from pos 14 to pos 33)
   uint32_t clientIP;      // Sender IP (0x00...0x00)
   uint32_t targIP;        // Target IP (0xFF...0xFF)
   uint16_t srcPort;       // Source Port (0x0044)
   uint16_t destPort;      // Destination Port (0x0043)
   uint16_t length;        // Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
   uint8_t msgType;        // Message type: Boot request (1)
   uint8_t hrdType;        // Hardware type: Ethernet (1)
   uint8_t addrLen;        // Hardware address length: (6)
   uint8_t hops;           // Hops: 0
   uint32_t transID;       // Transaction ID:
   uint16_t secElps;       // Seconds elapsed: 0
   uint16_t broadCast;     // Broadcast flags: broadcast(0x8000)
   uint32_t senderIP;      // Sender IP address (0)
   uint32_t yourIP;        // Your (Client) IP address (0)
   uint32_t nserverIP;     // Next server IP address (0)
   uint32_t relayIP;       // Gateway IP Address (0)
   uint8_t clntMAC[6];     // Client MAC address (0...0)
   uint32_t file[50];      // file (0...0)
   uint8_t endfile[2];     // file (0...0)
   uint8_t magCookie[4];   // Magic cookie: (from pos: 278 - {0x63,0x82,0x53,0x63})
   uint8_t msgType1;      	// Options: 53 - DHCP Message type (0x35)
   uint8_t length1;      	// Options: Length - 0x01
   uint8_t DHCP;      	   // Options: DHCP - Discover 0x01, Request 0x03

//   uint8_t autoConf;      	// Options: 116 - DHCP Auto-Configuration (0x74)
//   uint8_t length1;      	// Options: Length - 0x01
//   uint8_t autoConf1;       // Options: AutoConfigure 0x01

   uint8_t clientID;       // Client identifier (0x3D)
   uint8_t length2;        // Length: 0x07
   uint8_t hrdType1;       // Hardware type: Ethernet (0x01)
   uint8_t clntMAC1[6];    // Client MAC address (0...0)

   uint8_t rqAddrIP;       // Requested IP address: 0x32
   uint8_t length3;        // Length: 0x04
   uint32_t clientIP1;     // Client IP: 0x00000000

   uint8_t dhcpServerID;   // DHCP server Identifier: 0x36
   uint8_t length4;        // Length: 0x04
   uint32_t dhcpIP;        // DHCP IP: 0x00000000

   uint8_t hostName;       // Host Name: 0x0C
   uint8_t length5;        // Length: 0x09
   char name[9];           // "ApoClient"

   uint8_t domainName;     // Client Fully Qualified Domain Name: 0x51
   uint8_t length6;        // Length: 0x0D
   char dname[13];         // 0x00,0x00,0x00,"ApoClient."

   uint8_t listRQ;         // Options: 55 - Parameter request list (0x37)
   uint8_t length7;        // Length: 0x03
   uint8_t subMsk;         // Parameter request list Item: (0x01) Subnet Mask
   uint8_t router;         // Parameter request list Item: (0x03) Router
   uint8_t dns;            // Parameter request list Item: (0x06) Domain Name Server
   uint8_t optEnd;         // Option End: 255 (0xFF)
} sDhcpTX_RQ;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          	// sDhcpTX_ACK (length 320+270 Byte)
{
   uint8_t destMAC[6];     // 0-5 Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // 6-11 Source MAC (0x00...0x00)
   uint16_t protocolType; 	// Packet type (ARP, IP...) 0x0800
   uint8_t hdrLen;         // Header length 0x45
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;      // Total Length: 303 from pos 14 to end
   uint16_t dataID;        // Identification: number of datagram fragment
   uint8_t flags[2];       // Flags (0000 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17) 0x11
   uint16_t hdrCRC;        // Header checksum (from pos 14 to pos 33)
   uint32_t clientIP;      // Sender IP (0x00...0x00)
   uint32_t targIP;        // Target IP (0xFF...0xFF)
   uint16_t srcPort;       // Source Port (0x0044)
   uint16_t destPort;      // Destination Port (0x0043)
   uint16_t length;        // Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
   uint8_t msgType;        // Message type: Boot request (1)
   uint8_t hrdType;        // Hardware type: Ethernet (1)
   uint8_t addrLen;        // Hardware address length: (6)
   uint8_t hops;           // Hops: 0
   uint32_t transID;       // Transaction ID:
   uint16_t secElps;       // Seconds elapsed: 0
   uint16_t broadCast;     // Broadcast flags: broadcast(0x8000)
   uint32_t senderIP;      // Sender IP address (0)
   uint32_t yourIP;        // Your (Client) IP address (0)
   uint32_t nserverIP;     // Next server IP address (0)
   uint32_t relayIP;       // Gateway IP Address (0)
   uint8_t clntMAC[6];     // Client MAC address (0...0)
   uint32_t file[50];      // file (0...0)
   uint8_t endfile[2];     // file (0...0)
   uint8_t magCookie[4];   // 278-281 Magic cookie: (from pos: 278 - {0x63,0x82,0x53,0x63})
   uint8_t msgType1;     	// Options: 53 - DHCP Message type (0x35)
   uint8_t length0;      	// Options: Length - 0x01
   uint8_t DHCP;      	   // Options: DHCP - Offer 0x02, Discover 0x01, Request 0x03, Ack 0x05
   uint8_t DHCPsrvIP;      // Options: DHCP server Identifier (0x36)
   uint8_t length1;      	// Options: Length - 0x04
   uint32_t serverIP;      // DHCP server IP address (c0:a8:00:01)
   uint8_t leaseTimeIP;    // Options: IP addres lease time (0x33)
   uint8_t length2;        // Length: 0x04
   uint32_t leaseTime;     // DHCP server lease Time (0x0000012C) (300sec, 5min)
   uint8_t subMaskOp;      // Subnet mask (1)
   uint8_t length3;        // Length: 0x04
   uint32_t subMask;       // Subnet mask (255.255.255.0)
   uint8_t routerOp;       // Router (0x03)
   uint8_t length4;        // Length: 0x04
   uint32_t routerIP;      // Router IP address (c0:a8:00:01)
   uint8_t dnsOp;          // Parameter request list Item: (0x06) Domain Name Server
   uint8_t length5;        // Length: 0x08
   uint32_t dnsIP;         // DNS server IP address (c0:a8:00:01)
   uint32_t adnsIP;        // DHCP server IP address (08:08:08:08)
   uint8_t optEnd;         // Option End: 255 (0xFF)
} sDhcpTX_ACK;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct          	// sDnsRQ (length 320+270 Byte)
{
   uint8_t destMAC[6];     // 0-5 Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // 6-11 Source MAC (0x00...0x00)
   uint16_t protocolType; 	// Packet type (ARP, IP...) 0x0800
   uint8_t hdrLen;         // Header length 0x45
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;      // Total Length: 303 from pos 14 to end
   uint16_t dataID;        // Identification: number of datagram fragment
   uint8_t flags[2];       // Flags (0000 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17) 0x11
   uint16_t hdrCRC;        // Header checksum (from pos 14 to pos 33)
   uint32_t clientIP;      // Sender IP (0x00...0x00)
   uint32_t targIP;        // Target IP (0xFF...0xFF)
   uint16_t srcPort;       // Source Port (0x0044)
   uint16_t destPort;      // Destination Port (0x0043)
   uint16_t length;        // Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)

   uint16_t transCntrID;   // Transaction ID
   uint8_t flags1[2];      // Flags: 0x01, 0x00
   uint16_t question;      // questions: 0x0001 (reverse)
   uint16_t answer;        // answer RRs: 0x0000
   uint16_t authority;     // authority RRs: 0x0000
   uint16_t additional;    // additional RRs: 0x0000
   uint8_t file[66];
} sDnsRQ;
#pragma pack(pop)


/*
#pragma pack(push,1)
typedef struct          // sDnsTX
{
   uint8_t destMAC[6];     // Destination MAC
   uint8_t srcMAC[6];      // Source MAC
   uint8_t type[2];        // Packet type (ARP, IP...)
   uint8_t hdrLen;         // Header length
   uint8_t diffServ;       // Differentiated Services (00000000)
   uint16_t totalLen;       // Total Length
   uint16_t dataID;         // Identification:
   uint8_t flags[2];       // Flags (0100 0000 0000 0000) = don't fragment
   uint8_t ttl;            // TTL - 64
   uint8_t protocol;       // Protocol UDP (17)
   uint16_t hdrCRC;         // Header checksum
   uint32_t clientIP;      // Sender IP
   uint32_t targIP;        // Target IP
   uint16_t srcPort;        // Source Port
   uint16_t destPort;       // Destination Port
   uint16_t length;         // Length: m_TxLength - 34
   uint16_t dataCRC;        // Data checksum
   uint16_t transID;        // Transaction ID:
   uint8_t dflags[2];      // Data Flags
   uint16_t question;       // questions: 1
   uint16_t answer;         // answer RRs: 0
   uint16_t authority;      // authority RRs: 0
   uint16_t additional;     // additional RRs: 0
} sDnsTX;
#pragma pack(pop)
*/
#endif // STRUCT_H
