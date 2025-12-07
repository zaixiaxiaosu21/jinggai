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
#include "battery.h"
#include  "rtc_wakeup.h"
#include "input.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
typedef struct
{
    GPS_Data_t gps_data;
    Lazer_Data_t lazer_data;
    float battery_voltage;
    bool rtc_wakeup_flag;
    bool input_wakeup_flag;
    bool stolen_wakeup_flag;
    bool fall_wakeup_flag;
} Data_t;

Data_t data;
void main_init(void)
{
    SysTick_Init();
    Log_Init();
    Led_Init();
   
    Power_Init();
    lis3dh_init();
    
    RTC_Wakeup_Init();
    Input_Init();
     SysTick_Delay(500);
    Log_Info("Flashing window");
    SysTick_Delay(3000);
    Log_Info("Flashing window end");
}
static char *data_to_json_str(Data_t *data)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "battery", data->battery_voltage);
    cJSON_AddNumberToObject(root, "latitude", data->gps_data.latitude);
    cJSON_AddNumberToObject(root, "longitude", data->gps_data.longitude);
    cJSON_AddNumberToObject(root, "co2", data->lazer_data.co2);
    cJSON_AddNumberToObject(root, "ch2o", data->lazer_data.ch2o);
    cJSON_AddNumberToObject(root, "voc", data->lazer_data.voc);
    // cJSON_AddBoolToObject(root, "fall", data->fall_wakeup_flag);
    // cJSON_AddBoolToObject(root, "stolen", data->stolen_wakeup_flag);
    // cJSON_AddBoolToObject(root, "input", data->input_wakeup_flag);
    // cJSON_AddBoolToObject(root, "rtc", data->rtc_wakeup_flag);

    char *ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    Log_Debug("JSON: \r\n%s\r\n", ret);
    return ret;
}
int main(void)
{   

    main_init();
     Log_Info("Start\n");
   while (1)
   {
     memset(&data,0,sizeof(data));
    //获取唤醒源
    data.rtc_wakeup_flag=RTC_Wakeup_Get_Flag();
    data.input_wakeup_flag=Input_GetFlag();
    data.stolen_wakeup_flag=lis3dh_get_stolen_flag();
    data.fall_wakeup_flag=lis3dh_get_fall_flag();
     Power_On();
     SysTick_Delay(100);
     //测量电池
     Log_Debug("Measuring battery...\n");
     Battery_Init();
     Battery_On();
     data.battery_voltage=Battery_GetVoltage();
     Battery_Off();
     Battery_Deinit();

    //  // 获取激光传感器数据
    //     Log_Debug("Start to get lazer data");
    //     Lazer_Init();
    //     Lazer_On();
    //     while (Lazer_Read(&data.lazer_data) != 0 ||
    //            data.lazer_data.ch2o == 0xFF)// 读取失败或数据异常（预热默认65535），重试
    //     {
    //         Log_Debug("Lazer Data co2: %u, ch2o: %u, voc: %u\r\n",
    //                   data.lazer_data.co2, data.lazer_data.ch2o, data.lazer_data.voc);
    //     }
    //     Lazer_Off();
    //     Lazer_Deinit();

    //     SysTick_Delay(100);
    //没有激光传感器，模拟数据
        data.lazer_data.co2=400;
        data.lazer_data.ch2o=20;
        data.lazer_data.voc=100;
    
        // 获取GPS数据
        // Log_Debug("Start to get GPS data");
        // GPS_Init();
        // GPS_On();
        // while (GPS_GetData(&data.gps_data) != 0 || data.gps_data.latitude == 0)
        // {
        //     Log_Debug("Get GPS data failed, retry");
        // }
        // GPS_Off();
        // GPS_Deinit();

        // SysTick_Delay(100);
        //没有GPS，模拟数据
        data.gps_data.latitude=39.908823;
        data.gps_data.longitude=116.397470;
        char *json_str = data_to_json_str(&data);
        // 发送数据
        Int_QS100_Init();
       // Int_QS100_Send_Data("hello", strlen("hello"));
        uint8_t *send_data = (uint8_t *)json_str;
        uint16_t send_data_len = strlen(json_str);
        Int_QS100_Send_Data(send_data, send_data_len);
        Qs100_Off();
        Qs100_Deinit();
        free(json_str);
        SysTick_Delay(100);
        Power_Off();
        Log_Debug("Start to sleep");

        Power_Sleep();

   }
   
    
}
