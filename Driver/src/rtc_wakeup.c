#include "rtc_wakeup.h"
#include "log.h"
#include "systick.h"

#define RTC_LSE_TRY_COUNT 250
#define RTC_LSI_TRY_COUNT 250

typedef enum
{
    RTC_CLK_SRC_TYPE_HSE_DIV32 = 0x01,
    RTC_CLK_SRC_TYPE_LSE = 0x02,
    RTC_CLK_SRC_TYPE_LSI = 0x03,
} RTC_CLK_SRC_TYPE;

uint32_t SynchPrediv, AsynchPrediv;
volatile bool RTC_WAKEUP_FLAG = false;

static ErrorStatus RTC_CLKSourceConfig(RTC_CLK_SRC_TYPE Clk_Src_Type, bool Is_First_Cfg_RCC)
{
    uint8_t lse_ready_count = 0, lsi_ready_count = 0;
    ErrorStatus Status = SUCCESS;
    /* Enable the PWR clock */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    /* Allow access to RTC */
    PWR_BackupAccessEnable(ENABLE);
    /* Disable RTC clock */
    RCC_EnableRtcClk(DISABLE);
    if (RTC_CLK_SRC_TYPE_HSE_DIV32 == Clk_Src_Type)
    {
        Log_Info("\r\n RTC_ClkSrc Is Set HSE/32! \r\n");
        if (true == Is_First_Cfg_RCC)
        {
            /* Enable HSE */
            RCC_EnableLsi(DISABLE);
            RCC_ConfigHse(RCC_HSE_ENABLE);
            if (RCC_WaitHseStable() == ERROR)
            {
                Status = ERROR;
                Log_Info("\r\n RTC_ClkSrc Set HSE/32 Faile! \r\n");
            }
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_HSE_DIV32);
        }
        else
        {
            RCC_EnableLsi(DISABLE);
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_HSE_DIV32);
            /* Enable HSE */
            RCC_ConfigHse(RCC_HSE_ENABLE);
            if (RCC_WaitHseStable() == ERROR)
            {
                Status = ERROR;
                Log_Info("\r\n RTC_ClkSrc Set HSE/32 Faile! \r\n");
            }
        }
        SynchPrediv = 0x7A0; // 8M/32 = 250KHz
        AsynchPrediv = 0x7F; // value range: 0-7F
    }
    else if (RTC_CLK_SRC_TYPE_LSE == Clk_Src_Type)
    {
        Log_Info("\r\n RTC_ClkSrc Is Set LSE! \r\n");
        if (true == Is_First_Cfg_RCC)
        {
            /* Enable the LSE OSC32_IN PC14 */
            RCC_EnableLsi(DISABLE); // LSI is turned off here to ensure that only one clock is turned on
#if (_TEST_LSE_BYPASS_)
            RCC_ConfigLse(RCC_LSE_BYPASS, 0x1FF);
#else
            RCC_ConfigLse(RCC_LSE_ENABLE, 0x1FF);
#endif
            lse_ready_count = 0;
            /****Waite LSE Ready *****/
            while ((RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET) && (lse_ready_count < RTC_LSE_TRY_COUNT))
            {
                lse_ready_count++;
                SysTick_Delay(10);
                /****LSE Ready failed or timeout*****/
                if (lse_ready_count >= RTC_LSE_TRY_COUNT)
                {
                    Status = ERROR;
                    Log_Info("\r\n RTC_ClkSrc Set LSE Faile! \r\n");
                    break;
                }
            }
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSE);
        }
        else
        {
            /* Enable the LSE OSC32_IN PC14 */
            RCC_EnableLsi(DISABLE);
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSE);
#if (_TEST_LSE_BYPASS_)
            RCC_ConfigLse(RCC_LSE_BYPASS, 0x1FF);
#else
            RCC_ConfigLse(RCC_LSE_ENABLE, 0x1FF);
#endif
            lse_ready_count = 0;
            /****Waite LSE Ready *****/
            while ((RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET) && (lse_ready_count < RTC_LSE_TRY_COUNT))
            {
                lse_ready_count++;
                SysTick_Delay(10);
                /****LSE Ready failed or timeout*****/
                if (lse_ready_count >= RTC_LSE_TRY_COUNT)
                {
                    Status = ERROR;
                    Log_Info("\r\n RTC_ClkSrc Set LSE Faile! \r\n");
                    break;
                }
            }
        }
        SynchPrediv = 0xFF;  // 32.768KHz
        AsynchPrediv = 0x7F; // value range: 0-7F
    }
    else if (RTC_CLK_SRC_TYPE_LSI == Clk_Src_Type)
    {
        Log_Info("\r\n RTC_ClkSrc Is Set LSI! \r\n");
        if (true == Is_First_Cfg_RCC)
        {
            /* Enable the LSI OSC */
            RCC_EnableLsi(ENABLE);
            /****Wait LSI Ready *****/
            while (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_LSIRD) == RESET && (lsi_ready_count < RTC_LSI_TRY_COUNT))
            {
                lsi_ready_count++;
                SysTick_Delay(10);
                /****LSI Ready failed or timeout*****/
                if (lsi_ready_count >= RTC_LSI_TRY_COUNT)
                {
                    Status = ERROR;
                    Log_Info("\r\n RTC_ClkSrc Set LSI Faile! \r\n");
                    break;
                }
            }
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSI);
        }
        else
        {
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSI);
            /* Enable the LSI OSC */
            RCC_EnableLsi(ENABLE);
            while (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_LSIRD) == RESET && (lsi_ready_count < RTC_LSI_TRY_COUNT))
            {
                lsi_ready_count++;
                SysTick_Delay(10);
                /****LSI Ready failed or timeout*****/
                if (lsi_ready_count >= RTC_LSI_TRY_COUNT)
                {
                    Status = ERROR;
                    Log_Info("\r\n RTC_ClkSrc Set LSI Faile! \r\n");
                    break;
                }
            }
        }
        SynchPrediv = 0x14A; // 41828Hz
        AsynchPrediv = 0x7F; // value range: 0-7F
    }
    else
    {
        Log_Info("\r\n RTC_ClkSrc Value is error! \r\n");
    }
    /* Enable the RTC Clock */
    RCC_EnableRtcClk(ENABLE);
    RTC_WaitForSynchro();
    (void)RTC->DATE;
    return Status;
}

static void BKP_WriteBkpData(uint32_t BKP_DAT, uint32_t Data)
{
    __IO uint32_t tmp = 0;
    /* Check the parameters */
    assert_param(IS_BKP_DAT(BKP_DAT));
    tmp = (uint32_t)&RTC->BKP1R;
    tmp += BKP_DAT;
    *(__IO uint32_t *)tmp = Data;
}

static uint32_t BKP_ReadBkpData(uint32_t BKP_DAT)
{
    __IO uint32_t tmp = 0;
    uint32_t value = 0;
    /* Check the parameters */
    assert_param(IS_BKP_DAT(BKP_DAT));
    tmp = (uint32_t)&RTC->BKP1R;
    tmp += BKP_DAT;
    value = (*(__IO uint32_t *)tmp);
    return value;
}

void RTC_PrescalerConfig(void)
{
    RTC_InitType RTC_InitStructure;
    /* Configure the RTC data register and RTC prescaler */
    RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
    RTC_InitStructure.RTC_HourFormat = RTC_24HOUR_FORMAT;
    /* Check on RTC init */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
        Log_Info("\r\n //******* RTC Prescaler Config failed **********// \r\n");
    }
}

static void EXTI20_RTCWKUP_Configuration(FunctionalState Cmd)
{
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    EXTI_ClrITPendBit(EXTI_LINE20);
    EXTI_InitStruct(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_LINE20;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);
    /* Enable the RTC WakeUp Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = Cmd;
    NVIC_Init(&NVIC_InitStructure);
}

void RTC_Wakeup_Init(void)
{
    /* Enable the PWR clock */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
    /* Allow access to RTC */
    PWR_BackupAccessEnable(ENABLE);

    /* RTC clock source select */
    if (SUCCESS == RTC_CLKSourceConfig(RTC_CLK_SRC_TYPE_LSE, true))
    {
        if (USER_WRITE_BKP_DAT1_DATA != BKP_ReadBkpData(BKP_DAT1))
        {
            RTC_PrescalerConfig();
            Log_Info("\r\n RTC configured....\r\n");
            /* wake up clock select */
            RTC_ConfigWakeUpClock(RTC_WKUPCLK_CK_SPRE_16BITS);
            /* wake up timer value */
            RTC_SetWakeUpCounter(3600);
            BKP_WriteBkpData(BKP_DAT1, USER_WRITE_BKP_DAT1_DATA);
            Log_Info("\r\n RTC Init Success\r\n");
        }
    }
    else
    {
        Log_Info("\r\n RTC Init Faile\r\n");
    }

    EXTI_ClrITPendBit(EXTI_LINE20);
    EXTI20_RTCWKUP_Configuration(ENABLE);
    /* Enable the RTC Wakeup Interrupt */
    RTC_ClrIntPendingBit(RTC_INT_WUT);
    RTC_ConfigInt(RTC_INT_WUT, ENABLE);
    RTC_EnableWakeUp(DISABLE);
}

bool RTC_Wakeup_Get_Flag(void)
{
    bool ret = RTC_WAKEUP_FLAG;
    RTC_WAKEUP_FLAG = false;
    return ret;
}

void RTC_WKUP_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_LINE20))
    {
        RTC_WAKEUP_FLAG = true;
        EXTI_ClrITPendBit(EXTI_LINE20);
        RTC_ClrIntPendingBit(RTC_INT_WUT);
    }
}
