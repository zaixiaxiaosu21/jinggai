#ifndef SYSTICK_H
#define SYSTICK_H

#include "n32l40x.h"

void SysTick_Init(void);
uint32_t SysTick_GetTick(void);
void SysTick_Delay(uint32_t ms);

#endif
