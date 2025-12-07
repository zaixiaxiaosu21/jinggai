#include "qs100.h"
#include  "systick.h"
#include "stdio.h"
#include "string.h"

void Int_QS100_Reset(void);

/**
 * @brief 发送指定的AT指令
 *
 * @param cmd AT指令
 * @param exceptStr 期望接收的字符串
 *  - "期望接收的字符串"
 *  - NULL
 */
void Int_QS100_SendCmd(uint8_t *cmd, uint8_t *exceptStr);

/* 检查是否能够联网(附着) */
Common_Status_t Int_QS100_CheckNet(void);
/* 创建socket客户端 */
Common_Status_t Int_QS100_Create_Client(uint8_t *socket);
/* 链接服务器端 */
Common_Status_t Int_QS100_Connect_Server(uint8_t socket, uint8_t *server_addr, uint16_t server_port);
/* 发送数据给指定的服务器 */
Common_Status_t Int_QS100_SendDataToServer(uint8_t socket, uint8_t *pData, uint16_t data_len);
/* 关闭socket服务 */
Common_Status_t Int_QS100_CloseClient(uint8_t socket);

#define IOT_FULL_BUFF_MAX_LEN 1024
uint8_t iot_full_buff[IOT_FULL_BUFF_MAX_LEN] = {0};
uint16_t iot_full_buff_len = 0;

#define IOT_BUFF_MAX_LEN 256
uint8_t iot_buff[IOT_BUFF_MAX_LEN] = {0};
uint16_t iot_buff_len = 0;

/* 重新尝试的次数上限 */
#define TRY_AGAIN_COUNT 5

/* 用于查询发送数据包的状态 */
#define SEQUENCE 1

/* socket 通道号 */
uint8_t socket = 0xff;
static void Qs100_En_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

    // 初始化EN引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = QS100_EN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(QS100_EN_PORT, &GPIO_InitStructure);
    // 低电平使能QS100
    GPIO_ResetBits(QS100_EN_PORT, QS100_EN_PIN);
}
static void uart_gpio_init(void)
{   
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOA, ENABLE);
    GPIO_InitType GPIO_InitStructure;
    // 初始化串口引脚
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_4;
    // GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF1_USART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_5;
    // GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART1;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
}
static void uart_init(void)
{
    USART_InitType USART_InitStructure;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART1, ENABLE);
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.BaudRate = 9600;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits = USART_STPB_1;
    USART_InitStructure.Parity = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_TX | USART_MODE_RX;
    USART_Init(USART1, &USART_InitStructure);

    // 启动串口
    USART_Enable(USART1, ENABLE);
}
static void uart_send(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        USART_SendData(USART1, data[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXC) == RESET)
            ;
             USART_ClrFlag(USART1, USART_FLAG_TXC);
    }
}

static size_t uart_recv(uint8_t *data, size_t len, int frame_timeout_ms, int timeout_ms)
{
    size_t index = 0;
    uint32_t start = SysTick_GetTick();

    while (index < len - 1)
    {
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXDNE) == SET)
        {
            data[index++] = USART_ReceiveData(USART1);
            start = SysTick_GetTick();
        }
        else
        {
            if (SysTick_GetTick() - start > frame_timeout_ms)
                break;
        }
    }
    data[index++] = '\0';
    return index;
}
void Int_QS100_Init(void)
{
    Qs100_En_Init();
    uart_gpio_init();
    uart_init();

  Int_QS100_Reset();
  // 4. 设置AT指令回显, 用于调试
  Int_QS100_SendCmd("ATE1\r\n", NULL);
  // 5. 查询软件版本信息， 用于测试
  Int_QS100_SendCmd("AT+CGMR\r\n", NULL);
}
/**
 * @brief 发送数据
 *
 * @param pData 数据
 * @param data_len 数据的长度
 */
Common_Status_t Int_QS100_Send_Data(uint8_t *pData, uint16_t data_len)
{
  uint8_t try_again_count = TRY_AGAIN_COUNT;
  // ! 1. 检查是否能够联网(附着)
  while (Int_QS100_CheckNet() != Common_OK && try_again_count > 0)
  {
    SysTick_Delay(200);
    try_again_count--;
    printf("目前不能联网，正在积极尝试重连...");
  }
  // 判断是否是达到了最大尝试次数
  if (try_again_count == 0)
  {
    printf("联网失败了...");
    return Common_ERROR;
  }

  printf("1. 联网成功...");
  try_again_count = TRY_AGAIN_COUNT;
  // ! 2. 创建socket客户端
  while (Int_QS100_Create_Client(&socket) != Common_OK && try_again_count > 0)
  {
    SysTick_Delay(200);
    try_again_count--;
    printf("正在积极尝试创建客户端...");
  }
  // 判断是否是达到了最大尝试次数
  if (try_again_count == 0)
  {
    printf("创建客户端失败了...");
    return Common_ERROR;
  }

  printf("2. 创建客户端成功...");

  try_again_count = TRY_AGAIN_COUNT;
  // ! 3. 链接服务器端
  while (Int_QS100_Connect_Server(socket, SERVER_ADDR, SERVER_PORT) != Common_OK && try_again_count > 0)
  {
    SysTick_Delay(200);
    try_again_count--;
    printf("正在积极尝试链接服务器端...");
  }
  // 判断是否是达到了最大尝试次数
  if (try_again_count == 0)
  {
    printf("链接服务器端失败了...");
    return Common_ERROR;
  }
  printf("3. 链接服务器端成功...");

  try_again_count = TRY_AGAIN_COUNT;
  // ! 4. 发送数据给服务器端
  while (Int_QS100_SendDataToServer(socket, pData, data_len) != Common_OK && try_again_count > 0)
  {
    SysTick_Delay(200);
    try_again_count--;
    printf("正在积极尝试发送数据给服务器端...");
  }
  // 判断是否是达到了最大尝试次数
  if (try_again_count == 0)
  {
    printf("发送数据给服务器端失败了...");
    return Common_ERROR;
  }

  printf("4. 发送数据给服务器端成功...");

  try_again_count = TRY_AGAIN_COUNT;
  // ! 5. 关闭socket服务
  while (Int_QS100_CloseClient(socket) != Common_OK && try_again_count > 0)
  {
    SysTick_Delay(200);
    try_again_count--;
    printf("正在积极尝试关闭socket...");
  }
  // 判断是否是达到了最大尝试次数
  if (try_again_count == 0)
  {
    printf("关闭socket失败了...");
    return Common_ERROR;
  }

  printf("5. 关闭socket成功...");
  SysTick_Delay(1000);

  return Common_OK;
}



void Int_QS100_Reset(void)
{
  // 1. 发送AT指令实现软重启
  uint8_t *reset_cmd = "AT+RB\r\n";

  uart_send(reset_cmd, strlen((char *)reset_cmd));
  // 2. 接收回显信息
  uart_recv(iot_buff, IOT_BUFF_MAX_LEN, 3000, 3000);
  iot_buff_len = strlen((char *)iot_buff);

  if (iot_buff_len > 0)
  {
    memcpy(iot_full_buff, iot_buff, iot_buff_len);
    iot_full_buff_len = iot_buff_len;

    memset(iot_buff, 0, IOT_BUFF_MAX_LEN);
    iot_buff_len = 0;
  }

  printf("\n %s", iot_full_buff);
}

/**
 * @brief 发送AT指令并接收变长数据
 * @param cmd AT指令
 * @param exceptStr 期望接收的字符串（如 "OK" 或 "ERROR"）
 */
void Int_QS100_SendCmd(uint8_t *cmd, uint8_t *exceptStr)
{
    // 1. 发送AT指令
    uart_send(cmd, strlen((char *)cmd));

    // 2. 接收回显数据
    memset(iot_full_buff, 0, IOT_FULL_BUFF_MAX_LEN);
    iot_full_buff_len = 0;

    // 3. 接收数据直到遇到 "OK" 或 "ERROR"（根据需要调整结束条件）
    while (1)
    {
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXDNE) == SET)
        {
            // 接收数据
            uint8_t received_byte = USART_ReceiveData(USART1);
            iot_buff[iot_buff_len++] = received_byte;

            // 存储接收到的数据到 iot_full_buff
            if (iot_buff_len > 0)
            {
                memcpy(&iot_full_buff[iot_full_buff_len], iot_buff, iot_buff_len);
                iot_full_buff_len += iot_buff_len;

                // 清空临时缓冲区
                memset(iot_buff, 0, IOT_BUFF_MAX_LEN);
                iot_buff_len = 0;
            }
        }
        // // 使用uart_recv函数接收数据
        // iot_buff_len = uart_recv(iot_buff, IOT_BUFF_MAX_LEN, 1000, );
        // // 存储接收到的数据到 iot_full_buff
        // if (iot_buff_len > 0)
        // {
        //     memcpy(&iot_full_buff[iot_full_buff_len], iot_buff, iot_buff_len);
        //     iot_full_buff_len += iot_buff_len;

        //     // 清空临时缓冲区
        //     memset(iot_buff, 0, IOT_BUFF_MAX_LEN);
        //     iot_buff_len = 0;
        // }

        // 检查是否接收到期望的字符串（如 "OK" 或 "ERROR"）
        if (exceptStr != NULL)
        {
            if (strstr((char *)iot_full_buff, (char *)exceptStr) != NULL)
            {
                break;  // 找到期望的字符串，结束接收
            }
        }
        else if (strstr((char *)iot_full_buff, "OK") != NULL || strstr((char *)iot_full_buff, "ERROR") != NULL)
        {
            break;  // 默认接收到 "OK" 或 "ERROR" 后结束接收
        }
    }

    // 打印接收到的数据
    printf("\nReceived: %s", iot_full_buff);
}

Common_Status_t Int_QS100_CheckNet(void)
{
  // 1. 发送AT指令
  Int_QS100_SendCmd("AT+CGATT?\r\n", NULL);
  // 2. 判断AT指令是否发送成功
  if (strstr((char *)iot_full_buff, "OK") == NULL)
  {
    return Common_ERROR;
  }

  // 3. 判断附着状态
  if (strstr((char *)iot_full_buff, "+CGATT:1") == NULL)
  {
    return Common_ERROR;
  }

  return Common_OK;
}

Common_Status_t Int_QS100_Create_Client(uint8_t *socket)
{
  // 1. 发送AT指令
  Int_QS100_SendCmd("AT+NSOCR=STREAM,6,0,1\r\n", NULL);
  // 2. 判断AT指令是否发送成功
  if (strstr((char *)iot_full_buff, "OK") == NULL)
  {
    return Common_ERROR;
  }

  // 3. 提取socket
  /*
    AT+NSOCR=STREAM,6,10005,1
    +NSOCR:1
    OK
  */

  char *subStr = strstr((char *)iot_full_buff, "+NSOCR:");
  *socket = subStr[7] - 48;
  printf("socket: %d", *socket);
  return Common_OK;
}

Common_Status_t Int_QS100_Connect_Server(uint8_t socket, uint8_t *server_addr, uint16_t server_port)
{
  // 1. 拼接字符串
  uint8_t send_cmd[32] = {0};
  sprintf((char *)send_cmd, "AT+NSOCO=%d,%s,%d\r\n", socket, server_addr, server_port);
  // 2. 发送AT指令连接服务器
  Int_QS100_SendCmd(send_cmd, NULL);
  // 3. 判断是否成功
  if (strstr((char *)iot_full_buff, "OK") == NULL)
  {
    return Common_ERROR;
  }
  return Common_OK;
}

uint8_t send_data_cmd[512] = {0};
Common_Status_t Int_QS100_SendDataToServer(uint8_t socket, uint8_t *pData, uint16_t data_len)
{
  /* 思路分析：
    - 1. 将字符串转换成16进制字符串
    - 2. 示例：
      - 十进制： 123456
      - 十六进制： 010203040506
  */
  // 1. 准备十六进制字符串的容器并计算长度
  uint16_t hex_data_len = data_len * 2 + 1; // +1 是为了存放字符串结束符 '\0'
  uint8_t hex_data[hex_data_len];

  // uint8_t *hex_data = malloc(hex_data_len);
  memset(hex_data, 0, hex_data_len);
  // 2. 将十进制的字符串转换成十六进制
  for (uint16_t i = 0; i < data_len; i++)
  {

    sprintf((char *)hex_data + i * 2, "%02X", pData[i]);
  }

  // 3. 拼接字符串
  sprintf((char *)send_data_cmd, "AT+NSOSD=%d,%d,%s,0x100,%d\r\n", socket, data_len, hex_data, SEQUENCE);

  // 4. 发送数据
  Int_QS100_SendCmd(send_data_cmd, "+NSOSTR:");

  // 5. 判断AT指令是否发送成功
  if (strstr((char *)iot_full_buff, "OK") == NULL)
  {
    return Common_ERROR;
  }

  // 6. 确认数据包是否发送成功
  uint8_t isSend = iot_full_buff[iot_full_buff_len - 3] - 48;
  if (isSend == 0)
  {
    return Common_ERROR;
  }

  return Common_OK;
}

Common_Status_t Int_QS100_CloseClient(uint8_t socket)
{
  uint8_t close_cmd[32] = {0};
  sprintf((char *)close_cmd, "AT+NSOCL=%d\r\n", socket);
  // 发送命令
  Int_QS100_SendCmd(close_cmd, NULL);
  // 判断AT指令是否发送成功
  if (strstr((char *)iot_full_buff, "OK") == NULL)
  {
    return Common_ERROR;
  }

  return Common_OK;
}
void Int_QS100_Enter_LowPower(void)
{
  // debug_printfln("QS100进入低功耗...");

  Int_QS100_SendCmd("AT+FASTOFF=0\r\n", NULL);
}

//实现uart1收发 中断服务函数
void Qs100_Deinit(void)
{
    USART_DeInit(USART1);
}



void Qs100_Off(void)
{
    GPIO_SetBits(QS100_EN_PORT, QS100_EN_PIN);
}
