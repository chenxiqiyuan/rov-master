/*
 * @Description: 深度传感器相关函数
 * @Author: chenxi
 * @Date: 2020-01-14 16:52:11
 * @LastEditTime : 2020-02-10 16:57:25
 * @LastEditors  : chenxi
 */

#ifndef SPL1301_H
#define SPL1301_H

#include "../user/DataType.h"

#define SPL1301_I2C_DEV  "/dev/i2c-1"  // SPL1301 使用的 I2C设备
#define SPL1301_I2C_ADDR 0x77          // 默认 SPL1301 I2C 地址

/* 寄存器地址 */
#define REG_ID 0x0D

/* 命令 */
#define CONTINUOUS_PRESSURE 1
#define CONTINUOUS_TEMPERATURE 2
#define CONTINUOUS_P_AND_T 3

/* 传感器标号 */
#define PRESSURE_SENSOR 0
#define TEMPERATURE_SENSOR 1

typedef struct
{
    int16 c0;
    int16 c1;
    int32 c00;
    int32 c10;
    int16 c01;
    int16 c11;
    int16 c20;
    int16 c21;
    int16 c30;
}spl1301_calib_param_t; // spl1301 出厂校准参数

typedef struct
{
    spl1301_calib_param_t calib_param; /* calibration data */
    uint8_t product_id;  // 产品ID     
    uint8_t revision_id; // 修订ID   

    int32 i32rawPressure;    // 原始压力值
    int32 i32rawTemperature; // 原始温度值
    int32 i32kP; // 压力系数
    int32 i32kT; // 温度系数

    float pressure;    // 实际压力值
    float temperature; // 实际温度值
}spl1301_t; 

//初始化调用
int spl1301Setup(const int pinBase);
//设置特定传感器的采样率和每秒过采样率
void spl1301_rateset(int fd, uint8_t iSensor, uint8_t u8SmplRate, uint8_t u8OverSmpl);
//开始一次温度测量
void spl1301_start_temperature(int fd);
//开始一次压力测量
void spl1301_start_pressure(int fd);
//选择连续测量模式
void spl1301_start_continuous(int fd, uint8_t mode);
// 获取原始温度值并将其转换为32位整数
void spl1301_get_raw_temp(int fd);
//调用该句转换数据,获取原始压力值并将其转换为32位整数
void spl1301_get_raw_pressure(int fd);
//根据原始值返回校准温度值。
float get_spl1301_temperature(void);
//根据原值返回校准压力值。
float get_spl1301_pressure(void);

#endif
