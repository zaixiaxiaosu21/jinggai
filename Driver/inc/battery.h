#ifndef BATTERY_H
#define BATTERY_H 

#include "n32l40x.h"

void Battery_Init(void);

void Battery_Deinit(void);

void Battery_On(void);

void Battery_Off(void);

float Battery_GetVoltage(void);

#endif
