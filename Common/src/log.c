/*****************************************************************************
 * Copyright (c) 2022, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file log.c
 * @author Nations Solution Team
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#include "log.h"

#if LOG_ENABLE

#include "n32l40x.h"
#include "n32l40x_gpio.h"
#include "n32l40x_usart.h"
#include "n32l40x_rcc.h"

#define LOG_GPIO GPIOA
#define LOG_PERIPH_GPIO RCC_APB2_PERIPH_GPIOA
#define LOG_TX_PIN GPIO_PIN_1
#define LOG_RX_PIN GPIO_PIN_0

void Log_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    LPUART_InitType LPUART_InitStructure;
    GPIO_InitStruct(&GPIO_InitStructure);
    LPUART_StructInit(&LPUART_InitStructure);

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO | LOG_PERIPH_GPIO, ENABLE);

    RCC_ConfigLPUARTClk(RCC_LPUARTCLK_SRC_APB1);
    RCC_EnableRETPeriphClk(RCC_RET_PERIPH_LPUART, ENABLE);

    GPIO_InitStructure.Pin = LOG_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF6_LPUART;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LOG_RX_PIN;
    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF6_LPUART;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    LPUART_DeInit();
    LPUART_InitStructure.BaudRate = 115200;
    /* Configure LPUART */
    LPUART_Init(&LPUART_InitStructure);
}

int fputc(int ch, FILE *f)
{
    LPUART_SendData((uint8_t)ch);
    while (LPUART_GetFlagStatus(LPUART_FLAG_TXC) == RESET)
        ;
    LPUART_ClrFlag(LPUART_FLAG_TXC);

    return (ch);
}

#ifdef USE_FULL_ASSERT

__WEAK void assert_failed(const uint8_t *expr, const uint8_t *file, uint32_t line)
{
    log_error("assertion failed: `%s` at %s:%d", expr, file, line);
    while (1)
    {
    }
}
#endif // USE_FULL_ASSERT

#endif // LOG_ENABLE
