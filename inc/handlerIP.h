#ifndef HANDLERIP_H
#define HANDLERIP_H

#include <stdint.h>

extern "C"
{
#include "swo.h"
}

#define TRANSACT_ID     0x5E933D4C

char ParseOfferDHCP(uint8_t *date, uint16_t len);
char ParseAckDHCP(uint8_t *date, uint16_t len);
char ParseARP(uint8_t *date, uint16_t len);
char ParseDNS(uint8_t *date, uint16_t len);

#endif // HANDLERIP_H
