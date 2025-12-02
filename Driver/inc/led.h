#ifndef LED_H
#define LED_H

#include "n32l40x.h"

#define LED_PORT GPIOA
#define LED_PIN GPIO_PIN_9

void Led_Init(void);

void Led_On(void);

void Led_Off(void);

#endif
