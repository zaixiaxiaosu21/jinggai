#include "systick.h"

static uint32_t ticks = 0;

void SysTick_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_SetPriority(SysTick_IRQn, 0x0);
}

uint32_t SysTick_GetTick(void)
{
    return ticks;
}

void SysTick_Delay(uint32_t ms)
{
    uint32_t start = ticks;

    while ((ticks - start) < ms)
        ;
}

void SysTick_Handler(void)
{
    ticks++;
}
