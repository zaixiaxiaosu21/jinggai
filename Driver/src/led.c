#include "led.h"

void Led_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(LED_PORT, &GPIO_InitStructure);
}

void Led_On(void)
{
    GPIO_ResetBits(LED_PORT, LED_PIN);
}

void Led_Off(void)
{
    GPIO_SetBits(LED_PORT, LED_PIN);
}
