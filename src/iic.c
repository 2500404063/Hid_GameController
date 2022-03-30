#include "iic.h"

void IIC_Init()
{
    GPIOA_ModeCfg(PIN_SDA, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(PIN_SCL, GPIO_ModeOut_PP_5mA);
    IIC_Idle();
}

void IIC_SDA_IN()
{
    GPIOA_ModeCfg(PIN_SDA, GPIO_ModeIN_PU);
}

void IIC_SDA_OUT()
{
    GPIOA_ModeCfg(PIN_SDA, GPIO_ModeOut_PP_5mA);
}

void IIC_Idle()
{
    IIC_Reset(PIN_SCL);
    IIC_Set(PIN_SDA);
    IIC_Set(PIN_SCL);
}

void IIC_Start()
{
    IIC_Idle();
    IIC_Reset(PIN_SDA);
}

void IIC_Stop()
{
    IIC_SDA_OUT();
    IIC_Reset(PIN_SCL);
    IIC_Reset(PIN_SDA);
    IIC_Set(PIN_SCL);
    IIC_Set(PIN_SDA);
}

void IIC_SendACK()
{
    IIC_SDA_OUT();
    IIC_Reset(PIN_SCL);
    IIC_Reset(PIN_SDA);
    IIC_Set(PIN_SCL);
}

void IIC_SendNACK()
{
    IIC_SDA_OUT();
    IIC_Reset(PIN_SCL);
    IIC_Set(PIN_SDA);
    IIC_Set(PIN_SCL);
}

UINT8 IIC_WaitACK()
{
    IIC_Reset(PIN_SCL);
    IIC_SDA_IN();
    IIC_Wait();
    IIC_Set(PIN_SCL);
    if (IIC_Read(PIN_SDA) == IIC_ACK)
    {
        return IIC_ACK;
    }
    else
    {
        return IIC_NACK;
    }
}

void IIC_0()
{
    IIC_Reset(PIN_SCL);
    if (IIC_Read(PIN_SDA))
    {
        IIC_Reset(PIN_SDA);
    }
    IIC_Set(PIN_SCL);
    IIC_Keep();
}

void IIC_1()
{
    IIC_Reset(PIN_SCL);
    if (!IIC_Read(PIN_SDA))
    {
        IIC_Set(PIN_SDA);
    }
    IIC_Set(PIN_SCL);
    IIC_Keep();
}

void IIC_Address_Mode(UINT8 addr, UINT8 mode)
{
    addr = (addr << 1) | mode;
    IIC_SendByte(addr);
}

void IIC_SendByte(UINT8 data)
{
    IIC_SDA_OUT();
    UINT8 s = 8;
    while (s > 0)
    {
        if ((data >> --s) & 0x01)
        {
            IIC_1();
        }
        else
        {
            IIC_0();
        }
    }
}

void IIC_ReadByte(PUINT8 buf)
{
    IIC_SDA_IN();
    *buf = 0;
    for (UINT8 s = 0; s < 8; s++)
    {
        IIC_Reset(PIN_SCL);
        IIC_Wait();
        IIC_Set(PIN_SCL);
        *buf = (*buf << 1) | (IIC_Read(PIN_SDA) >> 4);
    }
}