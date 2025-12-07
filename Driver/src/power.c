#include "power.h"
#include "systick.h"
#include "rtc_wakeup.h"
void Power_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = POWER_PIN;
    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(POWER_PORT, &GPIO_InitStructure);
}

void Power_On(void)
{
    GPIO_SetBits(POWER_PORT, POWER_PIN);
}

void Power_Off(void)
{
    GPIO_ResetBits(POWER_PORT, POWER_PIN);
}

void Power_Sleep(void)
{
    RTC_EnableWakeUp(ENABLE);
    SysTick_Delay(20);
    PWR_EnterSTOP2Mode(PWR_STOPENTRY_WFI, PWR_CTRL3_RAM1RET | PWR_CTRL3_RAM2RET);
    SysTick_Delay(20);
    RTC_EnableWakeUp(DISABLE);
}
