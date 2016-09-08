
#include "spi.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"

extern void showNetPack(uint8_t *data, uint16_t len, uint8_t dir);

extern void EthernetIrqRXA(void);
extern void EthernetIrqRXB(void);

volatile uint8_t enc28j60_current_bankA = 0;
volatile uint16_t enc28j60_rxrdptA = 0;
volatile uint8_t enc28j60_current_bankB = 0;
volatile uint16_t enc28j60_rxrdptB = 0;

#define enc28j60_resetA	GPIOD, GPIO_Pin_1
#define enc28j60_resetB	GPIOB, GPIO_Pin_0

#define enc28j60_selectA()  GPIO_ResetBits(GPIOD, GPIO_Pin_0)
#define enc28j60_releaseA() GPIO_SetBits(GPIOD, GPIO_Pin_0)

#define enc28j60_selectB()  GPIO_ResetBits(GPIOC, GPIO_Pin_5)
#define enc28j60_releaseB() GPIO_SetBits(GPIOC, GPIO_Pin_5)

#define enc28j60_rxA() enc28j60_rxtxA(0xff)
#define enc28j60_txA(data) enc28j60_rxtxA(data)

#define enc28j60_rxB() enc28j60_rxtxB(0xff)
#define enc28j60_txB(data) enc28j60_rxtxB(data)

extern void delay_us(unsigned int us);

/************************* ETHERNET A ***************************/

uint8_t enc28j60_rxtxA(uint8_t data)
{
   ////SWO_PrintString((char*)("A_TXE\n"));
   uint32_t rcnt = 0;
   while(RESET == SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE))
   {
      if(++rcnt > 1000)
      {
         //SWO_PrintString((char*)("TIMEOUT_A FLAG_TXE\n"));
         break;
      };
   };

	SPI_I2S_SendData(SPI3,data);
	delay_us(2);
	rcnt = 0;

   //while(RESET != SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY))
	while(RESET == SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE))
   {
      if(++rcnt > 1000)
      {
         //SWO_PrintString((char*)("TIMEOUT_A FLAG_RXNE\n"));
         break;
      };
   };
	return SPI_I2S_ReceiveData(SPI3);
}

// Generic SPI read command
uint8_t enc28j60_read_opA(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	enc28j60_selectA();
	enc28j60_txA(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) enc28j60_rxA();// throw out dummy byte when reading MII/MAC register
	data = enc28j60_rxA();
	enc28j60_releaseA();

	return data;
}

// Generic SPI write command
void enc28j60_write_opA(uint8_t cmd, uint8_t adr, uint8_t data)
{
	enc28j60_selectA();
	enc28j60_txA(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_txA(data);
   while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY));
   delay_us(2);
	enc28j60_releaseA();
}

// Initiate software reset
void enc28j60_soft_resetA(void)
{
	enc28j60_selectA();
	enc28j60_txA(ENC28J60_SPI_SC);
	delay_us(10);
	enc28j60_releaseA();
	delay_us(1000);//Ожидание окончания инициализации ENC28J60
	enc28j60_current_bankA = 0;
}

// Set register bank
void enc28j60_set_bankA(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bankA)
		{
			enc28j60_write_opA(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_opA(ENC28J60_SPI_BFS, ECON1, bank);
			enc28j60_current_bankA = bank;
		}
	}
}

// Read register
uint8_t enc28j60_rcrA(uint8_t adr)
{
	enc28j60_set_bankA(adr);
	return enc28j60_read_opA(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16A(uint8_t adr)
{
	enc28j60_set_bankA(adr);
	return (enc28j60_read_opA(ENC28J60_SPI_RCR, adr) | (enc28j60_read_opA(ENC28J60_SPI_RCR, adr+1) << 8));
}

// Write register
void enc28j60_wcrA(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bankA(adr);
	enc28j60_write_opA(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16A(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bankA(adr);
	enc28j60_write_opA(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_opA(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfcA(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bankA(adr);
	enc28j60_write_opA(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfsA(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bankA(adr);
	enc28j60_write_opA(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_bufferA(uint8_t *buf, uint16_t len)
{
	enc28j60_selectA();
	enc28j60_txA(ENC28J60_SPI_RBM);
	while(len--)
		*(buf++) = enc28j60_rxA();
	enc28j60_releaseA();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_bufferA(uint8_t *buf, uint16_t len)
{
	enc28j60_selectA();
	enc28j60_txA(ENC28J60_SPI_WBM);
	while(len--)
		enc28j60_txA(*(buf++));
	enc28j60_releaseA();
}

// Read PHY register
uint16_t enc28j60_read_phyA(uint8_t adr)
{
	enc28j60_wcrA(MIREGADR, adr);
	enc28j60_bfsA(MICMD, MICMD_MIIRD);
	while(enc28j60_rcrA(MISTAT) & MISTAT_BUSY) ;
	enc28j60_bfcA(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16A(MIRD);
}

// Write PHY register
void enc28j60_write_phyA(uint8_t adr, uint16_t data)
{
	enc28j60_wcrA(MIREGADR, adr);
	enc28j60_wcr16A(MIWR, data);
	while(enc28j60_rcrA(MISTAT) & MISTAT_BUSY) ;
}

void enc28j60_initA(uint8_t *macadr)
{
	enc28j60_releaseA();
	delay_us(50);
	enc28j60_selectA();
	delay_us(50);
	enc28j60_releaseA();
	delay_us(50);                                                  //CS == 1
	GPIO_SetBits(enc28j60_resetA);
	delay_us(50);
   GPIO_ResetBits(enc28j60_resetA);
   delay_us(50);
	GPIO_SetBits(enc28j60_resetA);
	delay_us(50);

	// Reset ENC28J60
	enc28j60_soft_resetA();
	delay_us(50);

//	uint8_t tmp = enc28j60_rcr(ESTAT);
	//while(~tmp & 0xFF);

	// Setup Rx/Tx buffer
	enc28j60_wcr16A(ERXST, ENC28J60_RXSTART);
	enc28j60_rcr16A(ERXST);
	enc28j60_wcr16A(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_wcr16A(ERXND, ENC28J60_RXEND);
	enc28j60_rxrdptA = ENC28J60_RXSTART;
	enc28j60_wcr16A(ETXST, ENC28J60_TXSTART);
	enc28j60_rcr16A(ETXST);

	// Set filter
	// packets with local MAC-address will be accepted
	//enc28j60_wcrA(ERXFCON, ERXFCON_CRCEN);
	enc28j60_wcrA(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN|ERXFCON_MPEN);
	enc28j60_wcr16A(EPMM0, 0x303f);
	enc28j60_wcr16A(EPMCSL, 0xf7f9);

	// Setup MAC
	//enc28j60_wcr(MACON1, 0x00);
	enc28j60_wcrA(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);           // Enable flow control, Enable MAC Rx
	enc28j60_wcrA(MACON2, 0x00);                                                   // Clear reset
	//enc28j60_wcrA(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);// Enable padding, Enable crc & frame len chk
	enc28j60_wcrA(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);// Enable padding, Enable crc & frame len chk
	//enc28j60_wcrA(MACON3, MACON3_PADCFG0|MACON3_FRMLNEN|MACON3_FULDPX);// Enable padding, Enable crc & frame len chk
   //enc28j60_wcrA(MACON3, MACON3_PADCFG0|MACON3_FRMLNEN|MACON3_FULDPX);
   enc28j60_wcr16A(MAIPGL, 0x0c12);
   //enc28j60_wcr(MAIPGH, 0x0c);
   enc28j60_wcrA(MABBIPG, 0x12);                                               // Set inter-frame gap
   // set maximum frame size for receive RX bytes (1520 BYTES)
	enc28j60_wcr16A(MAMXFL, ENC28J60_MAXFRAME);
	//enc28j60_wcr(MABBIPG, 0x15);                                               // Set inter-frame gap



	enc28j60_wcrA(MAADR5, macadr[0]);                                           // Set MAC address
	enc28j60_wcrA(MAADR4, macadr[1]);
	enc28j60_wcrA(MAADR3, macadr[2]);
	enc28j60_wcrA(MAADR2, macadr[3]);
	enc28j60_wcrA(MAADR1, macadr[4]);
	enc28j60_wcrA(MAADR0, macadr[5]);

//	enc28j60_wcrA(MAADR0, macadr[0]);                                           // Set MAC address
//	enc28j60_wcrA(MAADR1, macadr[1]);
//	enc28j60_wcrA(MAADR2, macadr[2]);
//	enc28j60_wcrA(MAADR3, macadr[3]);
//	enc28j60_wcrA(MAADR4, macadr[4]);
//	enc28j60_wcrA(MAADR5, macadr[5]);



	// Setup PHY
	enc28j60_write_phyA(PHCON1, PHCON1_PDPXMD);                                 // Force full-duplex mode
	enc28j60_write_phyA(PHCON2, PHCON2_HDLDIS);                                 // Disable loopback
	// LedA - yellow, DedB - green
	enc28j60_write_phyA(PHLCON, PHLCON_LACFG0|PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH); // Configure LED ctrl

	// Enable Rx packets
	enc28j60_bfsA(ECON1, ECON1_RXEN);
	// enable interrupts
	enc28j60_bfsA(EIE, EIE_INTIE|EIE_PKTIE);
	//enc28j60_bfs(ECON2, ECON2_AUTOINC);

	//enc28j60_bfs(EFLOCON, EFLOCON_FULDPXS|EFLOCON_FCEN0|EFLOCON_FCEN1);
}

void enc28j60_send_packetA(uint8_t *data, uint16_t len)
{
   int rcnt = 0;
   enc28j60_bfsA(ECON1, ECON1_TXRST);
   delay_us(1);
	enc28j60_bfcA(ECON1, ECON1_TXRST);
	delay_us(1);
	while(enc28j60_rcrA(ECON1) & ECON1_TXRTS)
	{
	   if(++rcnt >= 10000) break;
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error

		if(enc28j60_rcrA(EIR) & EIR_TXERIF)
		{
			enc28j60_bfsA(ECON1, ECON1_TXRST);
			enc28j60_bfcA(ECON1, ECON1_TXRST);
		};
	};

	enc28j60_wcr16A(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_bufferA((uint8_t*)"0x00", 1);
	//enc28j60_write_bufferA(data, len);
	enc28j60_write_bufferA(data, len);

	enc28j60_wcr16A(ETXST, ENC28J60_TXSTART);
	delay_us(1);
	enc28j60_wcr16A(ETXND, ENC28J60_TXSTART + len);
	delay_us(1);

	enc28j60_bfsA(ECON1, ECON1_TXRTS); // Request packet send
	delay_us(1);

	//SWO_PrintString("sent A pack\n");
	//showNetPack(data, len, 1);
}

uint16_t enc28j60_recv_packetA(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_rcrA(EPKTCNT))                                                  // Есть ли принятые пакеты?
	{
		enc28j60_wcr16A(ERDPT, enc28j60_rxrdptA);                         // Считываем заголовок

		enc28j60_read_bufferA((uint8_t*)&enc28j60_rxrdptA, sizeof(enc28j60_rxrdptA));
		enc28j60_read_bufferA((uint8_t*)&rxlen, sizeof(rxlen));
		enc28j60_read_bufferA((uint8_t*)&status, sizeof(status));

		if(status & 0x80)                                                                // Если пакет принят успешно
		{
			len = rxlen - 4;                                                               // Cut out crc
			if(len > buflen) len = buflen;
			enc28j60_read_bufferA(buf, len);
		}

		// Set Rx read pointer to next packet
		temp = (enc28j60_rxrdptA - 1) & ENC28J60_BUFEND;
		enc28j60_wcr16A(ERXRDPT, temp);

		// Decrement packet counter
		enc28j60_bfsA(ECON2, ECON2_PKTDEC);
	}

	return len;
}

/************************* ETHERNET B ***************************/

uint8_t enc28j60_rxtxB(uint8_t data)
{
   //SWO_PrintString((char*)("B_TXE\n"));
   uint32_t rcnt = 0;
   while(RESET == SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE))
   {
      if(++rcnt > 1000)
      {
         SWO_PrintString((char*)("TIMEOUT_B FLAG_TXE\n"));
         break;
      };
   };

	SPI_I2S_SendData(SPI1,data);
	delay_us(2);
	rcnt = 0;

	while(RESET == SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE))
   {
      if(++rcnt > 1000)
      {
         SWO_PrintString((char*)("TIMEOUT_B FLAG_RXNE\n"));
         break;
      };
   };
	return SPI_I2S_ReceiveData(SPI1);
}

// Generic SPI read command
uint8_t enc28j60_read_opB(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	enc28j60_selectB();
	enc28j60_txB(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) enc28j60_rxB();// throw out dummy byte when reading MII/MAC register
	data = enc28j60_rxB();
	enc28j60_releaseB();

	return data;
}

// Generic SPI write command
void enc28j60_write_opB(uint8_t cmd, uint8_t adr, uint8_t data)
{
	enc28j60_selectB();
	enc28j60_txB(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_txB(data);
   while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
   delay_us(2);
	enc28j60_releaseB();
}

// Initiate software reset
void enc28j60_soft_resetB(void)
{
	enc28j60_selectB();
	enc28j60_txB(ENC28J60_SPI_SC);
	delay_us(10);
	enc28j60_releaseB();
	delay_us(1000);//Ожидание окончания инициализации ENC28J60
	enc28j60_current_bankB = 0;
}

// Set register bank
void enc28j60_set_bankB(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bankB)
		{
			enc28j60_write_opB(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_opB(ENC28J60_SPI_BFS, ECON1, bank);
			enc28j60_current_bankB = bank;
		}
	}
}

// Read register
uint8_t enc28j60_rcrB(uint8_t adr)
{
	enc28j60_set_bankB(adr);
	return enc28j60_read_opB(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16B(uint8_t adr)
{
	enc28j60_set_bankB(adr);
	return (enc28j60_read_opB(ENC28J60_SPI_RCR, adr) | (enc28j60_read_opB(ENC28J60_SPI_RCR, adr+1) << 8));
}

// Write register
void enc28j60_wcrB(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bankB(adr);
	enc28j60_write_opB(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16B(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bankB(adr);
	enc28j60_write_opB(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_opB(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfcB(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bankB(adr);
	enc28j60_write_opB(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfsB(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bankB(adr);
	enc28j60_write_opB(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_bufferB(uint8_t *buf, uint16_t len)
{
	enc28j60_selectB();
	enc28j60_txB(ENC28J60_SPI_RBM);
	while(len--)
		*(buf++) = enc28j60_rxB();
	enc28j60_releaseB();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_bufferB(uint8_t *buf, uint16_t len)
{
	enc28j60_selectB();
	enc28j60_txB(ENC28J60_SPI_WBM);
	while(len--)
		enc28j60_txB(*(buf++));
	enc28j60_releaseB();
}

// Read PHY register
uint16_t enc28j60_read_phyB(uint8_t adr)
{
	enc28j60_wcrB(MIREGADR, adr);
	enc28j60_bfsB(MICMD, MICMD_MIIRD);
	while(enc28j60_rcrB(MISTAT) & MISTAT_BUSY) ;
	enc28j60_bfcB(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16B(MIRD);
}

// Write PHY register
void enc28j60_write_phyB(uint8_t adr, uint16_t data)
{
	enc28j60_wcrB(MIREGADR, adr);
	enc28j60_wcr16B(MIWR, data);
	while(enc28j60_rcrB(MISTAT) & MISTAT_BUSY) ;
}

void enc28j60_initB(uint8_t *macadr)
{
	enc28j60_releaseB();
	delay_us(50);
	enc28j60_selectB();
	delay_us(50);
	enc28j60_releaseB();
	delay_us(50);                                                  //CS == 1
	GPIO_SetBits(enc28j60_resetB);
	delay_us(50);
   GPIO_ResetBits(enc28j60_resetB);
   delay_us(50);
	GPIO_SetBits(enc28j60_resetB);
	delay_us(50);

	// Reset ENC28J60
	enc28j60_soft_resetB();
	delay_us(50);

//	uint8_t tmp = enc28j60_rcr(ESTAT);
	//while(~tmp & 0xFF);

	// Setup Rx/Tx buffer
	enc28j60_wcr16B(ERXST, ENC28J60_RXSTART);
	enc28j60_rcr16B(ERXST);
	enc28j60_wcr16B(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_wcr16B(ERXND, ENC28J60_RXEND);
	enc28j60_rxrdptB = ENC28J60_RXSTART;
	enc28j60_wcr16B(ETXST, ENC28J60_TXSTART);
	enc28j60_rcr16B(ETXST);

	// Set filter
	// packets with local MAC-address will be accepted
	enc28j60_wcrB(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN|ERXFCON_MPEN);
	enc28j60_wcr16B(EPMM0, 0x303f);
	enc28j60_wcr16B(EPMCSL, 0xf7f9);

	// Setup MAC
	//enc28j60_wcr(MACON1, 0x00);
	enc28j60_wcrB(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);           // Enable flow control, Enable MAC Rx
	enc28j60_wcrB(MACON2, 0x00);                                                   // Clear reset
	//enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);// Enable padding, Enable crc & frame len chk
	enc28j60_wcrB(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);// Enable padding, Enable crc & frame len chk
	//enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_FRMLNEN|MACON3_FULDPX);// Enable padding, Enable crc & frame len chk
   //enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_FRMLNEN|MACON3_FULDPX);
   enc28j60_wcr16B(MAIPGL, 0x0c12);
   //enc28j60_wcr(MAIPGH, 0x0c);
   enc28j60_wcrB(MABBIPG, 0x12);                                               // Set inter-frame gap
   // set maximum frame size for receive RX bytes (1520 BYTES)
	enc28j60_wcr16B(MAMXFL, ENC28J60_MAXFRAME);
	//enc28j60_wcr(MABBIPG, 0x15);                                               // Set inter-frame gap



	enc28j60_wcrB(MAADR5, macadr[0]);                                           // Set MAC address
	enc28j60_wcrB(MAADR4, macadr[1]);
	enc28j60_wcrB(MAADR3, macadr[2]);
	enc28j60_wcrB(MAADR2, macadr[3]);
	enc28j60_wcrB(MAADR1, macadr[4]);
	enc28j60_wcrB(MAADR0, macadr[5]);



	// Setup PHY
	enc28j60_write_phyB(PHCON1, PHCON1_PDPXMD);                                 // Force full-duplex mode
	enc28j60_write_phyB(PHCON2, PHCON2_HDLDIS);                                 // Disable loopback
	// LedA - yellow, DedB - green
	enc28j60_write_phyB(PHLCON, PHLCON_LACFG0|PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH); // Configure LED ctrl

	// Enable Rx packets
	enc28j60_bfsB(ECON1, ECON1_RXEN);
	// enable interrupts
	enc28j60_bfsB(EIE, EIE_INTIE|EIE_PKTIE);
	//enc28j60_bfs(ECON2, ECON2_AUTOINC);

	//enc28j60_bfs(EFLOCON, EFLOCON_FULDPXS|EFLOCON_FCEN0|EFLOCON_FCEN1);
}

void enc28j60_send_packetB(uint8_t *data, uint16_t len)
{
   int rcnt = 0;
   enc28j60_bfsB(ECON1, ECON1_TXRST);
   delay_us(1);
	enc28j60_bfcB(ECON1, ECON1_TXRST);
	delay_us(1);
	while(enc28j60_rcrB(ECON1) & ECON1_TXRTS)
	{
	   if(++rcnt >= 10000) break;
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error

		if(enc28j60_rcrB(EIR) & EIR_TXERIF)
		{
			enc28j60_bfsB(ECON1, ECON1_TXRST);
			enc28j60_bfcB(ECON1, ECON1_TXRST);
		};
	};

	enc28j60_wcr16B(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_bufferB((uint8_t*)"0x00", 1);
	enc28j60_write_bufferB(data, len);

	enc28j60_wcr16B(ETXST, ENC28J60_TXSTART);
	delay_us(1);
	enc28j60_wcr16B(ETXND, ENC28J60_TXSTART + len);
	delay_us(1);

	enc28j60_bfsB(ECON1, ECON1_TXRTS); // Request packet send
	delay_us(1);

	//SWO_PrintString("sent B pack\n");
	//showNetPack(data, len, 1);
}

uint16_t enc28j60_recv_packetB(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_rcrB(EPKTCNT))                                                  // Есть ли принятые пакеты?
	{
		enc28j60_wcr16B(ERDPT, enc28j60_rxrdptB);                         // Считываем заголовок

		enc28j60_read_bufferB((uint8_t*)&enc28j60_rxrdptB, sizeof(enc28j60_rxrdptB));
		enc28j60_read_bufferB((uint8_t*)&rxlen, sizeof(rxlen));
		enc28j60_read_bufferB((uint8_t*)&status, sizeof(status));

		if(status & 0x80)                                                                // Если пакет принят успешно
		{
			len = rxlen - 4;                                                               // Cut out crc
			if(len > buflen) len = buflen;
			enc28j60_read_bufferB(buf, len);
		}

		// Set Rx read pointer to next packet
		temp = (enc28j60_rxrdptB - 1) & ENC28J60_BUFEND;
		enc28j60_wcr16B(ERXRDPT, temp);

		// Decrement packet counter
		enc28j60_bfsB(ECON2, ECON2_PKTDEC);
	}

	return len;
}

/****************************************************************/

void initGPIO_SPI(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   SPI_InitTypeDef SPI_InitStructure;
   /**============================ SPI3 - ETHERNET A ================================*/
   /* Configure GPIO pin SPI3 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin alternate function */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

	/* Configure GPIO pin for External INTERRUPT */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Configure GPIO pin for CS */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Configure GPIO pin for RESET */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      // полный дуплекс
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                       // передаем по 8 бит
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                              // Полярность и
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                            // фаза тактового сигнала
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                               // Управлять состоянием сигнала NSS программно
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;     // Предделитель SCK
   //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;     // Предделитель SCK
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      // Первым отправляется старший бит
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                           // Режим - мастер
   SPI_Init(SPI3, &SPI_InitStructure);                                     // Настраиваем SPI1

   // Поскольку сигнал NSS контролируется программно, установим его в единицу
   // Если сбросить его в ноль, то наш SPI модуль подумает, что
   // у нас мультимастерная топология и его лишили полномочий мастера.
   SPI_NSSInternalSoftwareConfig(SPI3, SPI_NSSInternalSoft_Set);
   SPI_Cmd(SPI3, ENABLE);                                                  // Включаем модуль SPI1....
   //SPI_DataSizeConfig(SPI3, SPI_DataSize_8b);

   /**============================ SPI1 - ETHERNET B ================================*/
	/* Configure GPIO pin SPI1 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure GPIO pin alternate function */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	/* Configure GPIO pin for External INTERRUPT */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin for CS */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin for RESET */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      // полный дуплекс
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                       // передаем по 8 бит
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                              // Полярность и
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                            // фаза тактового сигнала
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                               // Управлять состоянием сигнала NSS программно
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;     // Предделитель SCK
   //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;     // Предделитель SCK
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      // Первым отправляется старший бит
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                           // Режим - мастер
   SPI_Init(SPI1, &SPI_InitStructure);                                     // Настраиваем SPI1

   // Поскольку сигнал NSS контролируется программно, установим его в единицу
   // Если сбросить его в ноль, то наш SPI модуль подумает, что
   // у нас мультимастерная топология и его лишили полномочий мастера.
   SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
   SPI_Cmd(SPI1, ENABLE);                                                  // Включаем модуль SPI1....
   //SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);

}

void initGPIO_EXTI(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   /* Enable clock for SYSCFG */
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /*Configure GPIO pin for RX0BF, RX1BF MCP2515 */
//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//	GPIO_Init(GPIOE, &GPIO_InitStruct);

   EXTI_InitTypeDef EXTI_InitStruct;
   NVIC_InitTypeDef NVIC_InitStruct;
   /********************************************************/
   /* Tell system that you will use PD2 for EXTI_Line2 */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource2);

   /* PE7 is connected to EXTI_Line2 */
   EXTI_InitStruct.EXTI_Line = EXTI_Line2;
   /* Enable interrupt */
   EXTI_InitStruct.EXTI_LineCmd = ENABLE;
   /* Interrupt mode */
   EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
   /* Triggers on rising and falling edge */
   EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
   /* Add to EXTI */
   EXTI_Init(&EXTI_InitStruct);


   /* Add IRQ vector to NVIC */
   /* PD2 is connected to EXTI_Line2, which has EXTI2_IRQn vector */
   NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
   /* Set priority */
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 10;
   /* Set sub priority */
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 5;
   /* Enable interrupt */
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
   /* Add to NVIC */
   NVIC_Init(&NVIC_InitStruct);

   /********************************************************/
   /* Tell system that you will use 4 for EXTI_Line4 */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource4);

   /* PE7 is connected to EXTI_Line4 */
   EXTI_InitStruct.EXTI_Line = EXTI_Line4;
   /* Enable interrupt */
   EXTI_InitStruct.EXTI_LineCmd = ENABLE;
   /* Interrupt mode */
   EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
   /* Triggers on rising and falling edge */
   EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
   /* Add to EXTI */
   EXTI_Init(&EXTI_InitStruct);


   /* Add IRQ vector to NVIC */
   /* PC4 is connected to EXTI_Line4, which has EXTI0_IRQn vector */
   NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
   /* Set priority */
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 10;
   /* Set sub priority */
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 5;
   /* Enable interrupt */
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
   /* Add to NVIC */
   NVIC_Init(&NVIC_InitStruct);
}

void EXTI2_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXTI_Line2) != RESET)
   {
      EthernetIrqRXA();
      EXTI_ClearITPendingBit(EXTI_Line2);
      //EthernetIrqRXA();
   };
}

void EXTI4_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXTI_Line4) != RESET)
   {
      EthernetIrqRXB();
      EXTI_ClearITPendingBit(EXTI_Line4);
      //EthernetIrqRXB();
   };
}

void initENC28J60_SPI(void)
{
   initGPIO_SPI();
   initGPIO_EXTI();
}



