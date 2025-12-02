#ifndef LAZER_H
#define LAZER_H

#include "n32l40x.h"

#define LAZER_EN_PORT GPIOB
#define LAZER_EN_PIN GPIO_PIN_13

typedef struct
{
    uint16_t voc;
    uint16_t ch2o;
    uint16_t co2;
} Lazer_Data_t;

void Lazer_Init(void);

int Lazer_Read(Lazer_Data_t *data);

void Lazer_On(void);

void Lazer_Off(void);

void Lazer_Deinit(void);

#endif
