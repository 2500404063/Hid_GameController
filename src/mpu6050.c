#include "mpu6050.h"
#include "iic.h"
#include "math.h"
#include "stdlib.h"
#include "timers.h"
#include "stdio.h"

static volatile float gVal_A_Roll_RAD = 0;
static volatile float gVal_A_Pitch_RAD = 0;
static volatile float gVal_A_Roll_ANG = 0;
static volatile float gVal_A_Pitch_ANG = 0;

static volatile float gVal_G_Roll_ANG = 0;
static volatile float gVal_G_Pitch_ANG = 0;
static volatile float gVal_G_Yaw_ANG = 0;

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

void MPU_Compute()
{
    // Start to time.
    TIMER0_Init(TIMERX_Duration_ms(20)); // max: 134217ms

    // Raw data of Accelerometer
    INT16 val_AX = ((MPU_ReadByte(MPU_Addr_Accel_X_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_X_L));
    INT16 val_AY = ((MPU_ReadByte(MPU_Addr_Accel_Y_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Y_L));
    INT16 val_AZ = ((MPU_ReadByte(MPU_Addr_Accel_Z_H) << 8) | MPU_ReadByte(MPU_Addr_Accel_Z_L));

    // Raw data of Gyroscope
    INT16 val_GX = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_X_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_X_L));
    INT16 val_GY = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_Y_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_Y_L));
    INT16 val_GZ = (INT16)((MPU_ReadByte(MPU_Addr_Gyro_Z_H) << 8) | MPU_ReadByte(MPU_Addr_Gyro_Z_L));
    // Record time stamp
    double delta_t = TIMER0_Clock_s();

    // Linear filter 1 / 300
    val_AX = (INT16)((float)val_AX * 0.003333f);
    val_AY = (INT16)((float)val_AY * 0.003333f);
    val_AZ = (INT16)((float)val_AZ * 0.003333f);

    // Compute roll and pitch
    gVal_A_Roll_RAD = atanf((float)val_AY / (float)val_AZ);
    gVal_A_Pitch_RAD = -atanf((float)val_AX / sqrtf(powf((float)val_AZ, 2.0) + powf((float)val_AY, 2.0)));
    gVal_A_Roll_ANG = gVal_A_Roll_RAD * MPU_Rad2Ang;
    gVal_A_Pitch_ANG = gVal_A_Pitch_RAD * MPU_Rad2Ang;

    // Convert to angle per second
    val_GX = (INT16)((double)val_GX * 2000 / 32767);
    val_GY = (INT16)((double)val_GY * 2000 / 32767);
    val_GZ = (INT16)((double)val_GZ * 2000 / 32767);

    // Compute volocity of angle for Global Coordinate
    float val_Vroll = (float)val_GX +
                      (float)val_GY * tanf(gVal_A_Pitch_RAD) * sinf(gVal_A_Roll_RAD) +
                      (float)val_GZ * tanf(gVal_A_Pitch_RAD) * cosf(gVal_A_Roll_RAD);
    float val_Vpitch = (float)val_GY * cosf(gVal_A_Roll_RAD) +
                       (float)val_GZ * -sinf(gVal_A_Roll_RAD);
    float val_Vyaw = (float)val_GY * sinf(gVal_A_Roll_RAD) / cosf(gVal_A_Pitch_RAD) +
                     (float)val_GZ * cosf(gVal_A_Roll_RAD) / cosf(gVal_A_Pitch_RAD);

    float delta_roll = (float)((double)val_Vroll * delta_t);
    float delta_pitch = (float)((double)val_Vpitch * delta_t);
    float delta_yaw = (float)((double)val_Vyaw * delta_t);

    // 0.1f precision
    delta_roll = (float)(INT32)(delta_roll * 10) / 10.0f;
    delta_pitch = (float)(INT32)(delta_pitch * 10) / 10.0f;
    delta_yaw = (float)(INT32)(delta_yaw * 10) / 10.0f;

    // Integrate
    gVal_G_Roll_ANG += delta_roll;
    gVal_G_Pitch_ANG += delta_pitch;
    gVal_G_Yaw_ANG += delta_yaw;

    // Print
    // UART1_SendString("A_Roll:", 7);
    // PrintFloat(gVal_A_Roll_ANG, 2);
    // UART1_SendString("    ", 4);
    // UART1_SendString("A_Pitch:", 8);
    // PrintFloat(gVal_A_Pitch_ANG, 2);
    // UART1_SendByte('\n');

    UART1_SendString("G_Roll:", 7);
    // PrintINT16((INT16)gVal_G_Roll_ANG);
    PrintFloat(gVal_G_Roll_ANG, 2);
    UART1_SendString("    ", 4);
    UART1_SendString("G_Pitch:", 8);
    // PrintINT16((INT16)gVal_G_Pitch_ANG);
    PrintFloat(gVal_G_Pitch_ANG, 2);
    UART1_SendString("    ", 4);
    UART1_SendString("G_Yaw:", 6);
    // PrintINT16((INT16)gVal_G_Yaw_ANG);
    PrintFloat(gVal_G_Yaw_ANG, 2);
    UART1_SendByte('\n');

    // UINT16 t = (UINT16)(-gVal_aRX * 1.5f);
    // pEP1_RAM_Addr[64] = *((PUINT8)&t);
    // pEP1_RAM_Addr[65] = *((PUINT8)&t + 1);
    // t = (UINT16)(-gVal_aRY * 5.0f);
    // pEP1_RAM_Addr[66] = *((PUINT8)&t);
    // pEP1_RAM_Addr[67] = *((PUINT8)&t + 1);
    // DevEP1_IN_Deal(0x04);
}

void PrintFloat(float val, UINT8 index)
{
    char buf[32];
    __itoa((INT32)val, buf, 10);
    UART1_SendString(buf, 0);
    DelsyMs(1);
    UART1_SendByte('.');
    __itoa((UINT32)((fabsf(val) - (UINT32)fabsf(val) + 1) * powf(10, (float)index)), buf, 10);
    UART1_SendString(buf + 1, 0);
    DelsyMs(1);
}

void PrintDouble(double val, UINT8 index)
{
    char buf[32];
    __itoa((INT32)val, buf, 10);
    UART1_SendString(buf, 0);
    DelsyMs(1);
    UART1_SendByte('.');
    __itoa((UINT32)((fabsf(val) - (UINT16)fabsf(val) + 1) * powf(10, (float)index)), buf, 10);
    UART1_SendString(buf + 1, 0);
    DelsyMs(1);
}

void PrintINT16(INT16 val)
{
    char buf[32];
    __itoa((INT16)val, buf, 10);
    UART1_SendString(buf, 0);
    DelsyMs(1);
}

void PrintINT32(INT32 val)
{
    char buf[32];
    __itoa(val, buf, 10);
    UART1_SendString(buf, 0);
    DelsyMs(1);
}