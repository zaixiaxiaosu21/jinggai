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
 * @file main.c
 * @author Nations
 * @version V1.2.2
 *
 * @copyright Copyright (c) 2022, Nations Technologies Inc. All rights reserved.
 */
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include "systick.h"
#include "log.h"
#include "led.h"
#include "lazer.h"
#include "gps.h"
#include "qs100.h"
#include "power.h"
#include "lis3dh.h"
int main(void)
{   

    SysTick_Init();
    Log_Init();
    Led_Init();
    Lazer_Init();
    GPS_Init();
    Power_Init();
    
    Power_On();
    //Lazer_On();
   // Int_QS100_Init();
    SysTick_Delay(100);
     lis3dh_init();
    
    
    SysTick_Delay(500);
    Log_Info("Start");
    while (1)
    {
        if (wake_up_reason & WAKE_UP_REASON_STOLEN)
        {
            Log_Info("Wake up by stolen\n");
            wake_up_reason &= ~(WAKE_UP_REASON_STOLEN);
        }
        if (wake_up_reason & WAKE_UP_REASON_FALL)
        {
            Log_Info("Wake up by fall\n");
            wake_up_reason &= ~(WAKE_UP_REASON_FALL);
        }
        
    }
    
}
