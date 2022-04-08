#include "CH57x_common.h"
#include "CH57x_clk.h"
#include "CH57x_pwr.h"
#include "CH57x_gpio.h"
#include "CH57x_uart.h"
#include "CH57x_usbdev.h"
#include "mpu6050.h"
#include "timers.h"

extern void PopAll();

void DebugInit()
{
    GPIOB_ModeCfg(GPIO_Pin_0, GPIO_ModeOut_PP_20mA);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    NVIC_EnableIRQ(UART1_IRQn);
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
}

int main()
{
    //切换外部时钟源
    SetSysClock(CLK_SOURCE_HSE_32MHz);
    //开启PLL倍频器
    PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);
    DelsyMs(5);
    DebugInit();
    USB_DeviceInit();
    MPU_InitIIC();
    MPU_SelfTest();
    MPU_Init();

    NVIC_EnableIRQ(USB_IRQn);
    GPIOB_SetBits(GPIO_Pin_0);
    DelsyMs(2000);
    while (1)
    {
        MPU_Compute();
    }
}
