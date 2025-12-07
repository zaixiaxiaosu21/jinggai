#ifndef POWER_H
#define POWER_H

#include "n32l40x.h"

#define POWER_PORT GPIOB
#define POWER_PIN GPIO_PIN_3

void Power_Init(void);

void Power_On(void);

void Power_Off(void);
void Power_Sleep(void);

#endif
