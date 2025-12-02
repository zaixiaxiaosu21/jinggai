#include "lazer.h"
#include <string.h>

static volatile bool data_ready = false;
static uint8_t usart2_buffer[8];
static uint8_t usart2_len = 0;
static uint8_t lazer_buffer[8];
static uint8_t lazer_len = 0;

void Lazer_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    // 开启时钟
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART2, ENABLE);

    // 初始化EN引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = LAZER_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(LAZER_EN_PORT, &GPIO_InitStructure);

    // 初始化串口引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    // GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART2;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // 初始化串口
    USART_InitStructure.BaudRate = 9600;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits = USART_STPB_1;
    USART_InitStructure.Parity = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_TX | USART_MODE_RX;
    USART_Init(USART2, &USART_InitStructure);

    // 配置串口中断

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ConfigInt(USART2, USART_INT_RXDNE, ENABLE);
    USART_ConfigInt(USART2, USART_INT_IDLEF, ENABLE);

    // 启动串口
    USART_Enable(USART2, ENABLE);
}

int Lazer_Read(Lazer_Data_t *data)
{
    // 等待数据就绪
    while (!data_ready)
        ;
    // 数据就绪后，重新将data_ready置为false
    data_ready = false;
    // 校验收到的数据
    if (lazer_len != 8)
        return -1;

    lazer_len = 0;
    // 校验数据
    if (lazer_buffer[0] != 0x5F)
        return -1;

    uint8_t temp = 0;
    for (size_t i = 0; i < 7; i++)
        temp += lazer_buffer[i];

    if (temp != lazer_buffer[7])
        return -1;

    data->voc = lazer_buffer[1] << 8 | lazer_buffer[2];
    data->ch2o = lazer_buffer[3] << 8 | lazer_buffer[4];
    data->co2 = lazer_buffer[5] << 8 | lazer_buffer[6];
    return 0;
}

void Lazer_On(void)
{
    GPIO_ResetBits(LAZER_EN_PORT, LAZER_EN_PIN);
}

void Lazer_Off(void)
{
    GPIO_SetBits(LAZER_EN_PORT, LAZER_EN_PIN);
}

void Lazer_Deinit(void)
{
    USART_DeInit(USART2);
    GPIO_DeInit(GPIOA);
}

void USART2_IRQHandler(void)
{
    if (USART_GetIntStatus(USART2, USART_INT_IDLEF) == SET)
    {
        // 触发空闲中断
        memcpy(lazer_buffer, usart2_buffer, usart2_len);
        lazer_len = usart2_len;
        usart2_len = 0;
        data_ready = true;
        // 清理中断
        USART_ReceiveData(USART2);
        return;
    }
    if (USART_GetIntStatus(USART2, USART_INT_RXDNE) == SET)
    {
        // 触发数据就绪中断
        usart2_buffer[usart2_len++] = USART_ReceiveData(USART2);
    }
}
