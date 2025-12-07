#include "lis3dh.h"
#include "log.h"
#include "systick.h"
#include  "stdint.h"
#include  "string.h"
#define I2Cx I2C1
#define I2Cx_SCL_PIN GPIO_PIN_6
#define I2Cx_SDA_PIN GPIO_PIN_7
#define GPIOx        GPIOB
#define TEST_BUFFER_SIZE  100
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))
#define I2C_MASTER_ADDR   0x30

stmdev_ctx_t lis3dh_ctx = {0};
volatile int8_t wake_up_reason = 0;

void i2c_master_init(void)
{
    I2C_InitType i2c1_master;
    GPIO_InitType i2c1_gpio;
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    GPIOx->POD |= (I2Cx_SCL_PIN | I2Cx_SDA_PIN);//pull up pin

    
    GPIO_InitStruct(&i2c1_gpio);
    i2c1_gpio.Pin               = GPIO_PIN_6 | GPIO_PIN_7;
    i2c1_gpio.GPIO_Slew_Rate    = GPIO_Slew_Rate_High;
    i2c1_gpio.GPIO_Mode         = GPIO_Mode_AF_OD;
    i2c1_gpio.GPIO_Alternate    = GPIO_AF1_I2C1;
    i2c1_gpio.GPIO_Pull         = GPIO_Pull_Up;	  
    GPIO_InitPeripheral(GPIOx, &i2c1_gpio);

    I2C_DeInit(I2C1);
	I2C_InitStruct(&i2c1_master);
    i2c1_master.BusMode     = I2C_BUSMODE_I2C;
    i2c1_master.FmDutyCycle = I2C_FMDUTYCYCLE_2;
    i2c1_master.OwnAddr1    = I2C_MASTER_ADDR;
    i2c1_master.AckEnable   = I2C_ACKEN;
    i2c1_master.AddrMode    = I2C_ADDR_MODE_7BIT;
    i2c1_master.ClkSpeed    = 400000; // 100K

    I2C_Init(I2C1, &i2c1_master);
    I2C_Enable(I2C1, ENABLE);
}
int32_t i2c_write_regs(void *handle, uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    // 等待i2c总线释放
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
        ;

    // 发送起始信号
    I2C_GenerateStart(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG))
        ;

    // 发送设备地址
    I2C_SendAddr7bit(I2C1, LIS3DH_I2C_ADD_H, I2C_DIRECTION_SEND);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)) // EV6
        ;

    // 发送寄存器地址
    I2C_SendData(I2C1, reg_addr | 0x80);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
        ;

    // 发送数据
    for (size_t i = 0; i < len; i++)
    {
        I2C_SendData(I2C1, data[i]);
        while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
            ;
    }

    // 等待发送完成
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
        ;

    // 发送停止信号，并等待总线释放
    I2C_GenerateStop(I2C1, ENABLE);
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
        ;
    return 0;
}

int32_t i2c_read_regs(void *handle, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    // 等待i2c总线释放
    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
        ;

    // 发送起始信号
    I2C_GenerateStart(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG))
        ;

    // 发送设备地址
    I2C_SendAddr7bit(I2C1, LIS3DH_I2C_ADD_H, I2C_DIRECTION_SEND);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)) // EV6
        ;

    // 发送寄存器地址
    I2C_SendData(I2C1, reg_addr | 0x80);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)) // EV8
        ;

    // 等待发送完成
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)) // EV8-2
        ;

    // 发送起始信号
    I2C_GenerateStart(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG))
        ;

    // 发送设备地址
    I2C_SendAddr7bit(I2C1, LIS3DH_I2C_ADD_H, I2C_DIRECTION_RECV);
    while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_RXMODE_FLAG)) // EV6
        ;

    for (size_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2C_ConfigAck(I2C1, DISABLE);
            I2C_GenerateStop(I2C1, ENABLE);
        }
        else
        {
            I2C_ConfigAck(I2C1, ENABLE);
        }
        // 等待数据可读
        while (!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_RECVD_FLAG))
            ;

        data[i] = I2C_RecvData(I2C1);
    }

    while (I2C_GetFlag(I2C1, I2C_FLAG_BUSY))
        ;

    return 0;
}
void lis3dh_gpio_init(void)
{
     RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

    GPIO_InitType GPIO_InitStructure;
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    // 上电
    GPIO_ResetBits(GPIOA, GPIO_PIN_10);
}

void KeyInputExtiInit(void )
{
    GPIO_InitType GPIO_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

  

    /* Enable the GPIO Clock */
   
   
    
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);
    
   

    /*Configure the GPIO pin as input floating*/
   
    
        GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin        = GPIO_PIN_4|GPIO_PIN_5;
        GPIO_InitStructure.GPIO_Pull  = GPIO_No_Pull;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Input;
        GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    
    /*Configure key EXTI Line to key input Pin*/

    GPIO_ConfigEXTILine(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE4);
    GPIO_ConfigEXTILine(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE5);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);
    
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE5;
    EXTI_InitPeripheral(&EXTI_InitStructure);
    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_Init(&NVIC_InitStructure);

}
// 有人偷井盖
void EXTI4_IRQHandler(void)
{
    if (RESET != EXTI_GetStatusFlag(EXTI_LINE4))
    {
        wake_up_reason |= WAKE_UP_REASON_STOLEN;
        EXTI_ClrITPendBit(EXTI_LINE4);
    }
}

// 自由落体
void EXTI9_5_IRQHandler(void)
{
    if (RESET != EXTI_GetStatusFlag(EXTI_LINE5))
    {
        wake_up_reason |= WAKE_UP_REASON_FALL;
        EXTI_ClrITPendBit(EXTI_LINE5);
    }
}
void lis3dh_init(void)
{
    Log_Func();
    lis3dh_gpio_init();
    // 初始化I2C总线
    i2c_master_init();

    // 初始化外部中断引脚
    KeyInputExtiInit();
      lis3dh_ctx.read_reg = i2c_read_regs;
    lis3dh_ctx.write_reg = i2c_write_regs;
    lis3dh_ctx.mdelay = SysTick_Delay;

    SysTick_Delay(10);

    // 配置LIS3DH传感器
    uint8_t device_id;
    if (lis3dh_device_id_get(&lis3dh_ctx, &device_id) != 0)
    {
        Log_Error("LIS3DH device id read error\n");
        return;
    }
    if (device_id != LIS3DH_ID)
    {
        Log_Error("LIS3DH device id error\n");
        return;
    }
    Log_Info("LIS3DH device id ok\n");
    lis3dh_reg_t reg = {
        .ctrl_reg1 = {
            .odr = LIS3DH_ODR_100Hz,
            .lpen = 1,
            .zen = PROPERTY_ENABLE,
            .yen = PROPERTY_ENABLE,
            .xen = PROPERTY_ENABLE,
        },
    };
    lis3dh_write_reg(&lis3dh_ctx, LIS3DH_CTRL_REG1, (uint8_t *)&reg, 1);
    SysTick_Delay(5);
    // 运动中断1绑定到INT1，需要将其配置为偷井盖
    memset(&reg, 0, sizeof(lis3dh_reg_t));
    reg.ctrl_reg3.i1_ia1 = PROPERTY_ENABLE;
    lis3dh_pin_int1_config_set(&lis3dh_ctx, &reg.ctrl_reg3);

    // 将运动中断2绑定到INT2，需要将其配置为自由落体中断
    memset(&reg, 0, sizeof(reg));
    reg.ctrl_reg6.i2_ia2 = PROPERTY_ENABLE;
    lis3dh_pin_int2_config_set(&lis3dh_ctx, &reg.ctrl_reg6);

    // 偷井盖配置，首先读取xy上面的现值
    memset(&reg, 0, sizeof(reg));
    lis3dh_status_get(&lis3dh_ctx, &reg.status_reg);
    while (!reg.status_reg.zyxda)
    {
        lis3dh_status_get(&lis3dh_ctx, &reg.status_reg);
    }
    int16_t acc_data[3];
    lis3dh_acceleration_raw_get(&lis3dh_ctx, acc_data);
    Log_Debug("acc_data: %f mg, %f mg, %f mg\n",
              lis3dh_from_fs2_lp_to_mg(acc_data[0]),
              lis3dh_from_fs2_lp_to_mg(acc_data[1]),
              lis3dh_from_fs2_lp_to_mg(acc_data[2]));
    if (acc_data[0] < 0)
    {
        acc_data[0] = -acc_data[0];
    }
    if (acc_data[1] < 0)
    {
        acc_data[1] = -acc_data[1];
    }
    uint8_t threhold_x = (acc_data[0] >> 8) + 2;
    uint8_t threhold_y = (acc_data[1] >> 8) + 2;

    Log_Info("threhold_x: %u, threhold_y: %u\n", threhold_x, threhold_y);

    memset(&reg, 0, sizeof(reg));
    lis3dh_int1_gen_threshold_set(&lis3dh_ctx, threhold_y > threhold_x ? threhold_y : threhold_x);
    lis3dh_int1_gen_duration_set(&lis3dh_ctx, 0x03);
    reg.int1_cfg.xhie = 1;
    reg.int1_cfg.yhie = 1;
    lis3dh_int1_gen_conf_set(&lis3dh_ctx, &reg.int1_cfg);

    // 自由落体
    lis3dh_int2_gen_threshold_set(&lis3dh_ctx, 0x16);
    lis3dh_int2_gen_duration_set(&lis3dh_ctx, 0x03);
    memset(&reg, 0, sizeof(reg));
    reg.int2_cfg.aoi = 1;
    reg.int2_cfg.xlie = 1;
    reg.int2_cfg.ylie = 1;
    reg.int2_cfg.zlie = 1;
    lis3dh_int2_gen_conf_set(&lis3dh_ctx, &reg.int2_cfg);

    Log_Info("LIS3DH Config done");
    

}

bool lis3dh_get_stolen_flag(void)
{
  bool ret= wake_up_reason &= WAKE_UP_REASON_FALL;
    wake_up_reason &= ~WAKE_UP_REASON_FALL;
    return ret;
}

bool lis3dh_get_fall_flag(void)
{
    bool ret= wake_up_reason &= WAKE_UP_REASON_STOLEN;
        wake_up_reason &= ~WAKE_UP_REASON_STOLEN;
        return ret;
}
