#ifndef __IIC_H__
#define __IIC_H__

#include "CH57x_common.h"
#include "CH57x_gpio.h"
#include "CH579SFR.h"

#define PIN_SDA GPIO_Pin_4
#define PIN_SCL GPIO_Pin_5

#define IIC_Set(pin) GPIOA_SetBits(pin)
#define IIC_Reset(pin) GPIOA_ResetBits(pin)
#define IIC_Read(pin) GPIOA_ReadPortPin(pin)

#define IIC_Keep() DelsyUs(1)
#define IIC_Wait() DelsyUs(1)

#define IIC_Mode_Send 0x00
#define IIC_Mode_Read 0x01

#define IIC_ACK 0x00
#define IIC_NACK 0x01

void IIC_Init();
void IIC_SDA_IN();
void IIC_SDA_OUT();
void IIC_Idle();
void IIC_Start();
void IIC_Stop();
void IIC_SendACK();
void IIC_SendNACK();
UINT8 IIC_WaitACK();
void IIC_0();
void IIC_1();
void IIC_Address_Mode(UINT8 addr, UINT8 mode);
void IIC_SendByte(UINT8 data);
void IIC_ReadByte(PUINT8 buf);
#endif