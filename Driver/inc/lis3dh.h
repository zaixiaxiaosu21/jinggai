#ifndef LIS3DH_H
#define LIS3DH_H

#include "lis3dh_reg.h"
#include "n32l40x.h"

typedef enum
{
    WAKE_UP_REASON_NONE = 0x0,
    WAKE_UP_REASON_STOLEN = 0x1,
    WAKE_UP_REASON_FALL = 0x2,
} wake_up_reason_t;

extern volatile int8_t wake_up_reason;

void lis3dh_init(void);

#endif
