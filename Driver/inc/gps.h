#ifndef GPS_H
#define GPS_H

#include "n32l40x.h"
#include  "stdbool.h"
#include "stdint.h"
#define GPS_EN_PORT GPIOB
#define GPS_EN_PIN  GPIO_PIN_15
typedef struct
{
    float latitude;
    float longitude;
} GPS_Data_t;

void GPS_Init(void);

void GPS_Deinit(void);

void GPS_On(void);

void GPS_Off(void);

int GPS_GetData(GPS_Data_t *data);

#endif
