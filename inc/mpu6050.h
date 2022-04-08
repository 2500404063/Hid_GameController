#ifndef __MPU6050_H__
#define __MPU6050_H__

#include "CH579SFR.h"
#include "CH57x_common.h"

#define MPU_Rad2Ang 57.295773f

#define MPU_Address 0x68
#define MPU_Addr_X_TEST 0x0D
#define MPU_Addr_Y_TEST 0x0E
#define MPU_Addr_Z_TEST 0x0F
#define MPU_Addr_Gyroscope_CTRL 0x1B
#define MPU_Addr_Accelerometer_CTRL 0x1C

#define MPU_Addr_Rate_DIV 0x19
#define MPU_Addr_Motion_THR 0x1F

#define MPU_Addr_FIFO_EN 0x23
#define MPU_Addr_INT_EN 0x38
#define MPU_Addr_INT_STATUS 0x3A

#define MPU_Addr_Accel_X_L 0x3C
#define MPU_Addr_Accel_X_H 0x3B
#define MPU_Addr_Accel_Y_L 0x3E
#define MPU_Addr_Accel_Y_H 0x3D
#define MPU_Addr_Accel_Z_L 0x40
#define MPU_Addr_Accel_Z_H 0x3F

#define MPU_Addr_Gyro_X_L 0x44
#define MPU_Addr_Gyro_X_H 0x43
#define MPU_Addr_Gyro_Y_L 0x46
#define MPU_Addr_Gyro_Y_H 0x45
#define MPU_Addr_Gyro_Z_L 0x48
#define MPU_Addr_Gyro_Z_H 0x47

#define MPU_Addr_PWR1 0x6B
#define MPU_Addr_PWR2 0x6C
#define MPU_Addr_WHOAMI 0x75

// Gyroscope X scale coefficient
#define MPU_K_GX 0.005f // 1/200
// Gyroscope Y scale coefficient
#define MPU_K_GY 0.0025f // 1/400
// Gyroscope Z scale coefficient
#define MPU_K_GZ 0.005f // 1/200

void MPU_InitIIC();
UINT8 MPU_ReadByte(UINT8 addr);
void MPU_WriteByte(UINT8 addr, UINT8 val);
void MPU_SelfTest();
void MPU_Init();

void MPU_Compute();
void PrintFloat(float val, UINT8 index);
void PrintDouble(double val, UINT8 index);
void PrintINT16(INT16 val);
void PrintINT32(INT32 val);
#endif