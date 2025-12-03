#include "gps.h"
#include <string.h>
#include <stdlib.h>
#define GPS_BUF_LEN 1024

static volatile bool gps_data_flag = false;
static uint8_t uart5_buffer[GPS_BUF_LEN];
static uint32_t uart5_buffer_len = 0;
static uint8_t gps_buffer[GPS_BUF_LEN];
void GPS_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    // 开启时钟
    RCC_EnableAPB2PeriphClk( RCC_APB2_PERIPH_GPIOB|RCC_APB2_PERIPH_UART5|RCC_APB2_PERIPH_AFIO, ENABLE);

    // 初始化EN引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPS_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPS_EN_PORT, &GPIO_InitStructure);

    // 初始化串口引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_8| GPIO_PIN_9;
    // GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF6_UART5;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    // 初始化串口
    USART_InitStructure.BaudRate = 9600;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits = USART_STPB_1;
    USART_InitStructure.Parity = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_TX | USART_MODE_RX;
    USART_Init(UART5, &USART_InitStructure);

    // 配置串口中断

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ConfigInt(UART5, USART_INT_RXDNE, ENABLE);
    USART_ConfigInt(UART5, USART_INT_IDLEF, ENABLE);

    // 启动串口
    USART_Enable(UART5, ENABLE);
   
}


    void GPS_Deinit(void)
{
    USART_DeInit(UART5);
}

void GPS_On(void)
{
    GPIO_ResetBits(GPS_EN_PORT,GPS_EN_PIN);
}

void GPS_Off(void)
{    
    GPIO_SetBits(GPS_EN_PORT,GPS_EN_PIN);
}
float GPS_NMEA_To_Float(char *coordinate, char direction) {
    // 提取度和分钟
    int degree = atoi(coordinate) / 100;  // 提取度部分（前两位）
    float minute = atof(coordinate) - degree * 100;  // 提取分钟部分

    // 转换为浮动数值
    float decimal_value = (float)degree + (minute / 60.0f);

    // 根据方向调整符号
    if (direction == 'S' || direction == 'W') {
        decimal_value = -decimal_value;  // 南纬和西经为负值
    }

    return decimal_value;
}
int GPS_GetData(GPS_Data_t *data)
{
     while (!gps_data_flag)
        ;
    gps_data_flag = false;
    char *latitude=NULL;
    char *longitude=NULL;

     char *sentence_start = strstr((char *)gps_buffer, "$GNGGA");
    if (!sentence_start)
    {
        return -1;
    }
    char * temp_str= strtok(sentence_start,",");
    uint8_t split_count=0;
    while(temp_str&& split_count<6){
        switch (split_count)
        {
        case 2:
            latitude =temp_str;
            break;
        case 3:
           data->latitude =GPS_NMEA_To_Float(latitude,temp_str[0]);//维度
            break;
        case 4:
            longitude=temp_str;
            break;
        case 5: 
            data->longitude=GPS_NMEA_To_Float(longitude,temp_str[0]);//经度
            break;
        default:
            break;
        }
        temp_str =strtok(NULL,",");
        split_count++;
    }
    return 0;
}
void UART5_IRQHandler(void){
    
    if (USART_GetIntStatus(UART5, USART_INT_IDLEF) == SET)
    {
        // 触发空闲中断
        memcpy(gps_buffer, uart5_buffer, uart5_buffer_len);
        gps_buffer[uart5_buffer_len]=0;
        uart5_buffer_len = 0;
        gps_data_flag = true;
        // 清理中断
        USART_ReceiveData(UART5);
        return;
    }
    if (USART_GetIntStatus(UART5, USART_INT_RXDNE) == SET)
    {
        // 触发数据就绪中断
        uart5_buffer[uart5_buffer_len++] = USART_ReceiveData(UART5);
    }
}
