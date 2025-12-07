#ifndef QS100_H
#define QS100_H

#include "n32l40x.h"
#include <stddef.h>

#define QS100_EN_PORT GPIOB
#define QS100_EN_PIN GPIO_PIN_14
typedef enum
{
  Common_OK,
  Common_TIMEOUT,
  Common_BUSY,
  Common_ERROR
} Common_Status_t;
#define SERVER_ADDR "112.125.89.8"
#define SERVER_PORT 34735
void Int_QS100_Init(void);

/**
 * @brief 发送数据
 * 
 * @param pData 数据
 * @param data_len 数据的长度
 */
Common_Status_t Int_QS100_Send_Data(uint8_t *pData, uint16_t data_len);


void Int_QS100_Enter_LowPower(void);

void Qs100_Deinit(void);



void Qs100_Off(void);
#endif

