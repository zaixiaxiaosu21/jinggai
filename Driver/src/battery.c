#include "battery.h"

#define BATTERY_EN_PIN GPIO_PIN_8
#define BATTERY_EN_PORT GPIOA

static void Battery_GPIO_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = BATTERY_EN_PIN;
    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(BATTERY_EN_PORT, &GPIO_InitStructure);

    GPIO_SetBits(BATTERY_EN_PORT, BATTERY_EN_PIN);
}

static void Battery_ADC_Init(void)
{
    ADC_InitType ADC_InitStructure;
    /* Enable GPIOC clocks */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    /* Enable ADC clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    /* RCC_ADCHCLK_DIV16*/
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV16);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8); // selsect HSE as RCC ADC1M CLK Source

    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);
    /* Configure PC0 PC1 as analog input -------------------------*/
    GPIO_InitStructure.Pin = GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    ADC_DeInit(ADC);
    /* ADC configuration ------------------------------------------------------*/
    ADC_InitStruct(&ADC_InitStructure);
    ADC_InitStructure.MultiChEn = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber = 10;
    ADC_Init(ADC, &ADC_InitStructure);

    /* Enable ADC */
    ADC_Enable(ADC, ENABLE);
    /* Check ADC Ready */
    while (ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET)
        ;
    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC))
        ;
}

void Battery_Init(void)
{
    Battery_GPIO_Init();
    Battery_ADC_Init();
}

void Battery_Deinit(void)
{
    ADC_DeInit(ADC);
}

void Battery_On(void)
{
    GPIO_ResetBits(BATTERY_EN_PORT, BATTERY_EN_PIN);
}

void Battery_Off(void)
{
    GPIO_SetBits(BATTERY_EN_PORT, BATTERY_EN_PIN);
}

float Battery_GetVoltage(void)
{
    uint16_t dat;
    ADC_ConfigRegularChannel(ADC, ADC_CH_10_PB1, 1, ADC_SAMP_TIME_55CYCLES5);
    /* Start ADC Software Conversion */
    ADC_EnableSoftwareStartConv(ADC, ENABLE);
    while (ADC_GetFlagStatus(ADC, ADC_FLAG_ENDCA) == 0)
    {
    }
    ADC_ClearFlag(ADC, ADC_FLAG_ENDCA);
    ADC_ClearFlag(ADC, ADC_FLAG_STR);
    dat = ADC_GetDat(ADC);
    return (dat & 0x0FFF) * 3.3f / 4096 * 2;
}
