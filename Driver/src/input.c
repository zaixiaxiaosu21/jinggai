#include "input.h"

volatile bool input_flag = false;

void Input_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    /* Check the parameters */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

    /*Configure the GPIO pin as input floating*/
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    /*Configure key EXTI Line to key input Pin*/
    GPIO_ConfigEXTILine(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE0);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line = EXTI_LINE0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

bool Input_GetFlag(void)
{
    bool ret = input_flag;
    input_flag = false;
    return ret;
}

void EXTI0_IRQHandler(void)
{
    if (RESET != EXTI_GetStatusFlag(EXTI_LINE0))
    {
        input_flag = true;
        EXTI_ClrITPendBit(EXTI_LINE0);
    }
}
