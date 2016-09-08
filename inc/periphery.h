#ifndef PERIPHERY_H
#define PERIPHERY_H

#include <stdint.h>

void PeripheryEnable(void);
void setGreenLedON(void);
void setGreenLedOFF(void);
void setRedLedON(void);
void setRedLedOFF(void);
void initLeds(void);
void InitSoundCtrlPin(void);
void setAudioCommand(uint8_t cmmd);
uint8_t CheckAudioBtn(void);

#endif /* PERIPHERY_H */
