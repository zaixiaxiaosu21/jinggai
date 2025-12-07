#ifndef RTC_WAKEUP_H
#define RTC_WAKEUP_H

#include "n32l40x.h"

#define BKP_DAT1 ((uint16_t)0x0000)
#define BKP_DAT2 ((uint16_t)0x0004)
#define BKP_DAT3 ((uint16_t)0x0008)
#define BKP_DAT4 ((uint16_t)0x000C)
#define BKP_DAT5 ((uint16_t)0x0010)
#define BKP_DAT6 ((uint16_t)0x0014)
#define BKP_DAT7 ((uint16_t)0x0018)
#define BKP_DAT8 ((uint16_t)0x001C)
#define BKP_DAT9 ((uint16_t)0x0020)
#define BKP_DAT10 ((uint16_t)0x0024)
#define BKP_DAT11 ((uint16_t)0x0028)
#define BKP_DAT12 ((uint16_t)0x002C)
#define BKP_DAT13 ((uint16_t)0x0030)
#define BKP_DAT14 ((uint16_t)0x0034)
#define BKP_DAT15 ((uint16_t)0x0038)
#define BKP_DAT16 ((uint16_t)0x003C)
#define BKP_DAT17 ((uint16_t)0x0040)
#define BKP_DAT18 ((uint16_t)0x0044)
#define BKP_DAT19 ((uint16_t)0x0048)
#define BKP_DAT20 ((uint16_t)0x004C)

#define IS_BKP_DAT(DAT) \
    (((DAT) == BKP_DAT1) || ((DAT) == BKP_DAT2) || ((DAT) == BKP_DAT3) || ((DAT) == BKP_DAT4) || ((DAT) == BKP_DAT5) || ((DAT) == BKP_DAT6) || ((DAT) == BKP_DAT7) || ((DAT) == BKP_DAT8) || ((DAT) == BKP_DAT9) || ((DAT) == BKP_DAT10) || ((DAT) == BKP_DAT11) || ((DAT) == BKP_DAT12) || ((DAT) == BKP_DAT13) || ((DAT) == BKP_DAT14) || ((DAT) == BKP_DAT15) || ((DAT) == BKP_DAT16) || ((DAT) == BKP_DAT17) || ((DAT) == BKP_DAT18) || ((DAT) == BKP_DAT19) || ((DAT) == BKP_DAT20))

#define USER_WRITE_BKP_DAT1_DATA 0xA5A5

void RTC_Wakeup_Init(void);

bool RTC_Wakeup_Get_Flag(void);

#endif
