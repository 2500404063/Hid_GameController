#include "mpu6050.h"
#include "iic.h"
#include "math.h"
#include "stdlib.h"
#include "timers.h"

float gVal_gRX = 0;
float gVal_gRY = 0;
float gVal_gRZ = 0;
float gVal_aRX = 0;
float gVal_aRY = 0;

void MPU_InitIIC()
{
    IIC_Init();
}

UINT8 MPU_ReadByte(UINT8 addr)
{
    UINT8 buf;
    IIC_Start();
    IIC_Address_Mode(MPU_Address, IIC_Mode_Send);
    IIC_WaitACK();
    IIC_SendByte(addr);
    IIC_WaitACK();

    IIC_Start();
    IIC_Address_Mode(MPU_Address, IIC_Mode_Read);
    IIC_WaitACK();
    IIC_ReadByte(&buf);
    IIC_SendNACK();
    IIC_Stop();
    return buf;
}

void MPU_WriteByte(UINT8 addr, UINT8 val)
{
    IIC_Start();
    IIC_Address_Mode(MPU_Address, IIC_Mode_Send);
    IIC_WaitACK();
    IIC_SendByte(addr);
    IIC_WaitACK();
    IIC_SendByte(val);
    IIC_WaitACK();
    IIC_Stop();
}

void MPU_SelfTest()
{
    // Enable Self Test
    MPU_WriteByte(MPU_Addr_Gyroscope_CTRL, 0xF0);
    MPU_WriteByte(MPU_Addr_Accelerometer_CTRL, 0xF0);
    UINT8 x = MPU_ReadByte(MPU_Addr_X_TEST);
    UINT8 y = MPU_ReadByte(MPU_Addr_Y_TEST);
    UINT8 z = MPU_ReadByte(MPU_Addr_Z_TEST);
    UINT8 xg = x & 0x1F;
    UINT8 yg = y & 0x1F;
    UINT8 zg = z & 0x1F;
    UINT8 xa = x & 0xE0 >> 5;
    UINT8 ya = y & 0xE0 >> 5;
    UINT8 za = z & 0xE0 >> 5;
    float FT_xg = xg == 0 ? 0.0f : 25.0f * 131.0f * powf(1.046, (float)xg - 1);
    float FT_yg = yg == 0 ? 0.0f : -25.0f * 131.0f * powf(1.046, (float)yg - 1);
    float FT_zg = zg == 0 ? 0.0f : 25.0f * 131.0f * powf(1.046, (float)zg - 1);
    float FT_xa = xa == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(xa - 1) / 30) / 0.34;
    float FT_ya = ya == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(ya - 1) / 30) / 0.34;
    float FT_za = za == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(za - 1) / 30) / 0.34;
    // Disable Self Test
    MPU_WriteByte(MPU_Addr_Gyroscope_CTRL, 0x10);
    MPU_WriteByte(MPU_Addr_Accelerometer_CTRL, 0x10);
    x = MPU_ReadByte(MPU_Addr_X_TEST);
    y = MPU_ReadByte(MPU_Addr_Y_TEST);
    z = MPU_ReadByte(MPU_Addr_Z_TEST);
    xg = x & 0x1F;
    yg = y & 0x1F;
    zg = z & 0x1F;
    xa = x & 0xE0 >> 5;
    ya = y & 0xE0 >> 5;
    za = z & 0xE0 >> 5;
    float FT_xg_dis = xg == 0 ? 0.0f : 25.0f * 131.0f * powf(1.046, (float)xg - 1);
    float FT_yg_dis = yg == 0 ? 0.0f : -25.0f * 131.0f * powf(1.046, (float)yg - 1);
    float FT_zg_dis = zg == 0 ? 0.0f : 25.0f * 131.0f * powf(1.046, (float)zg - 1);
    float FT_xa_dis = xa == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(xa - 1) / 30) / 0.34;
    float FT_ya_dis = ya == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(ya - 1) / 30) / 0.34;
    float FT_za_dis = za == 0 ? 0.0f : 4096.0f * 0.34f * powf(0.92, (float)(za - 1) / 30) / 0.34;

    float STR_xg = FT_xg - FT_xg_dis;
    float STR_yg = FT_yg - FT_yg_dis;
    float STR_zg = FT_zg - FT_zg_dis;

    float STR_xa = FT_xa - FT_xa_dis;
    float STR_ya = FT_ya - FT_ya_dis;
    float STR_za = FT_za - FT_za_dis;
}

void MPU_Init()
{
    MPU_WriteByte(MPU_Addr_Rate_DIV, 0x07);           // 1+7 DIV
    MPU_WriteByte(MPU_Addr_Accelerometer_CTRL, 0x18); //+-16 g
    MPU_WriteByte(MPU_Addr_Gyroscope_CTRL, 0x18);     //+-2000 dps
    MPU_WriteByte(MPU_Addr_PWR1, 0x08);               // Wake, Internal Clock
}

void MPU_Gyroscope()
{
    char buf[32];
    // Start to time.
    TIMER0_Init(TIMERX_Duration_ms(50));
    // Raw data of Gyroscope
    INT16 val_GX = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_X_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_X_L));
    INT16 val_GY = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_Y_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_Y_L));
    INT16 val_GZ = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_Z_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_Z_L));
    // Record time stamp
    float t_s = TIMER0_Clock_us() * 1e-4;

    INT16 val_gRX = val_GX * 0.061037 * 0.01; // r per ms
    INT16 val_gRY = val_GY * 0.061037 * 0.01; // r per ms
    INT16 val_gRZ = val_GZ * 0.061037 * 0.01; // r per ms

    gVal_gRX += val_gRX * t_s;
    gVal_gRY += val_gRY * t_s;
    gVal_gRZ += val_gRZ * t_s;

    // UART1_SendString("GX:", 4);
    // __itoa((INT16)gVal_gRX, buf, 10);
    // UART1_SendString(buf, 0);
    // DelsyMs(1);

    // UART1_SendString("  GY:", 6);
    // __itoa((INT16)gVal_gRY, buf, 10);
    // UART1_SendString(buf, 0);
    // DelsyMs(1);

    // UART1_SendString("  GZ:", 6);
    // __itoa((INT16)gVal_gRZ, buf, 10);
    // UART1_SendString(buf, 0);
    // DelsyMs(1);
    // UART1_SendByte('\n');
    // DelsyMs(50);
}

void MPU_Accelerometer()
{
    // Raw data of Accelerometer
    INT16 val_AX = ((MPU_ReadByte(MPU_Addr_Accel_X_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_X_L));
    INT16 val_AY = ((MPU_ReadByte(MPU_Addr_Accel_Y_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Y_L));
    INT16 val_AZ = ((MPU_ReadByte(MPU_Addr_Accel_Z_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Z_L));
    // Processed data of Acceleromter
    // INT16 val_AX = (INT16)((float)(INT16)((MPU_ReadByte(MPU_Addr_Accel_X_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_X_L)) * 0.00048828125);
    // INT16 val_AY = (INT16)((float)(INT16)((MPU_ReadByte(MPU_Addr_Accel_Y_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Y_L)) * 0.00048828125);
    // INT16 val_AZ = (INT16)((float)(INT16)((MPU_ReadByte(MPU_Addr_Accel_Z_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Z_L)) * 0.00048828125);

    gVal_aRX = acosf((float)val_AZ / sqrtf(powf((float)val_AZ, 2) + powf((float)val_AY, 2))) * 57.295773f * (val_AY < 0 ? 1.0f : -1.0f);
    gVal_aRY = acosf((float)val_AZ / sqrtf(powf((float)val_AZ, 2) + powf((float)val_AX, 2))) * 57.295773f * (val_AX < 0 ? 1.0f : -1.0f);
}

void MPU_Compute()
{
    MPU_Gyroscope();
    MPU_Accelerometer();

    UINT16 t = (UINT16)gVal_aRX;
    pEP1_RAM_Addr[64] = *((PUINT8)&t);
    pEP1_RAM_Addr[65] = *((PUINT8)&t + 1);
    t = (UINT16)gVal_aRY;
    pEP1_RAM_Addr[66] = *((PUINT8)&t);
    pEP1_RAM_Addr[67] = *((PUINT8)&t + 1);
    DevEP1_IN_Deal(0x04);
}