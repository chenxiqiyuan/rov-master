/*
 * @Description: spl1301 深度传感器驱动程序
 */

#define LOG_TAG "spl1301"

#include <elog.h>
#include "spl1301.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

static int fd;
static spl1301_t spl1301_dev;
static spl1301_t *spl1301 = &spl1301_dev;


void spl1301_get_calib_param(void);


/**
 * @brief 设置 SPL1301 压力或温度 的采样率和过采率 
 * @param 
 *  uint8 iSensor     0: Pressure; 1: Temperature 
 *  uint8 u8SmplRate  sample rate(Hz)   Maximal = 128
 *  uint8 u8OverSmpl  oversample rate   Maximal = 128
 */
void spl1301_rateset(uint8 iSensor, uint8 u8SmplRate, uint8 u8OverSmpl)
{
    uint8 reg = 0;
    int32 i32kPkT = 0;
    switch (u8SmplRate)
    {
    case 2:
        reg |= (1 << 4);
        break;
    case 4:
        reg |= (2 << 4);
        break;
    case 8:
        reg |= (3 << 4);
        break;
    case 16:
        reg |= (4 << 4);
        break;
    case 32:
        reg |= (5 << 4);
        break;
    case 64:
        reg |= (6 << 4);
        break;
    case 128:
        reg |= (7 << 4);
        break;
    case 1:
    default:
        break;
    }
    switch (u8OverSmpl)
    {
    case 2:
        reg |= 1;
        i32kPkT = 1572864;
        break;
    case 4:
        reg |= 2;
        i32kPkT = 3670016;
        break;
    case 8:
        reg |= 3;
        i32kPkT = 7864320;
        break;
    case 16:
        i32kPkT = 253952;
        reg |= 4;
        break;
    case 32:
        i32kPkT = 516096;
        reg |= 5;
        break;
    case 64:
        i32kPkT = 1040384;
        reg |= 6;
        break;
    case 128:
        i32kPkT = 2088960;
        reg |= 7;
        break;
    case 1:
    default:
        i32kPkT = 524288;
        break;
    }

    if (iSensor == PRESSURE_SENSOR)
    {
        spl1301->i32kP = i32kPkT;
        wiringPiI2CWriteReg8(fd, 0x06, reg);
        if (u8OverSmpl > 8)
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg | 0x04);
        }
        else
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg & (~0x04));
        }
    }
    if (iSensor == TEMPERATURE_SENSOR)
    {
        spl1301->i32kT = i32kPkT;
        wiringPiI2CWriteReg8(fd, 0x07, reg | 0x80); //Using mems temperature
        if (u8OverSmpl > 8)
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg | 0x08);
        }
        else
        {
            reg = wiringPiI2CReadReg8(fd, 0x09);
            wiringPiI2CWriteReg8(fd, 0x09, reg & (~0x08));
        }
    }
}

/**
 * @brief 获取 spl1301 内部ram的出厂校准数据，用于后续的数据转换
 */
void spl1301_get_calib_param(void)
{
    uint32 h;
    uint32 m;
    uint32 l;
    h = wiringPiI2CReadReg8(fd, 0x10);
    l = wiringPiI2CReadReg8(fd, 0x11);
    spl1301->calib_param.c0 = (int16)h << 4 | l >> 4;
    spl1301->calib_param.c0 = (spl1301->calib_param.c0 & 0x0800) ? (0xF000 | spl1301->calib_param.c0) : spl1301->calib_param.c0;
    h = wiringPiI2CReadReg8(fd, 0x11);
    l = wiringPiI2CReadReg8(fd, 0x12);
    spl1301->calib_param.c1 = (int16)(h & 0x0F) << 8 | l;
    spl1301->calib_param.c1 = (spl1301->calib_param.c1 & 0x0800) ? (0xF000 | spl1301->calib_param.c1) : spl1301->calib_param.c1;
    h = wiringPiI2CReadReg8(fd, 0x13);
    m = wiringPiI2CReadReg8(fd, 0x14);
    l = wiringPiI2CReadReg8(fd, 0x15);
    spl1301->calib_param.c00 = (int32)h << 12 | (int32)m << 4 | (int32)l >> 4;
    spl1301->calib_param.c00 = (spl1301->calib_param.c00 & 0x080000) ? (0xFFF00000 | spl1301->calib_param.c00) : spl1301->calib_param.c00;
    h = wiringPiI2CReadReg8(fd, 0x15);
    m = wiringPiI2CReadReg8(fd, 0x16);
    l = wiringPiI2CReadReg8(fd, 0x17);
    spl1301->calib_param.c10 = (int32)(h & 0x0F) << 16 | (int32)m << 8 | l;
    spl1301->calib_param.c10 = (spl1301->calib_param.c10 & 0x080000) ? (0xFFF00000 | spl1301->calib_param.c10) : spl1301->calib_param.c10;
    h = wiringPiI2CReadReg8(fd, 0x18);
    l = wiringPiI2CReadReg8(fd, 0x19);
    spl1301->calib_param.c01 = (int16)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1A);
    l = wiringPiI2CReadReg8(fd, 0x1B);
    spl1301->calib_param.c11 = (int16)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1C);
    l = wiringPiI2CReadReg8(fd, 0x1D);
    spl1301->calib_param.c20 = (int16)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x1E);
    l = wiringPiI2CReadReg8(fd, 0x1F);
    spl1301->calib_param.c21 = (int16)h << 8 | l;
    h = wiringPiI2CReadReg8(fd, 0x20);
    l = wiringPiI2CReadReg8(fd, 0x21);
    spl1301->calib_param.c30 = (int16)h << 8 | l;
}


/**
 * @brief 启动 spl1301 温度测量
 * @notice 如果设置 spl1301 模式位连续采样，则该函数不需要使用
 */
void spl1301_start_temperature(void)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0x02);
}

/**
 * @brief 启动 SPL1301 压力测量
 * @notice 如果设置 SPL1301 模式位连续采样，则该函数不需要使用
 */
void spl1301_start_pressure(void)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0x01);
}

/**
 * @brief  选择传感器为连续测量模式
 * @param 
 *  uint8 mode  1: pressure; 2: temperature; 3: pressure and temperature  
 */
void spl1301_start_continuous(uint8 mode)
{
    wiringPiI2CWriteReg8(fd, 0x08, mode + 4);
}

/**
 * @brief  spl1301 停止转换
 */
void spl1301_stop(void)
{
    wiringPiI2CWriteReg8(fd, 0x08, 0);
}

/**
 * @brief  获取原始温度值，并将其转换为32位整数
 */
void spl1301_get_raw_temp(void)
{
    uint8 h, m, l;

    h = wiringPiI2CReadReg8(fd, 0x03);
    m = wiringPiI2CReadReg8(fd, 0x04);
    l = wiringPiI2CReadReg8(fd, 0x05);

    spl1301->i32rawTemperature = (int32)h << 16 | (int32)m << 8 | (int32)l;
    spl1301->i32rawTemperature = (spl1301->i32rawTemperature & 0x800000) ? (0xFF000000 | spl1301->i32rawTemperature) : spl1301->i32rawTemperature;
}

/**
 * @brief  获取原始压力值，并将其转换为32位整数
 */
void spl1301_get_raw_pressure(void)
{
    uint8 h, m, l;

    h = wiringPiI2CReadReg8(fd, 0x00);
    m = wiringPiI2CReadReg8(fd, 0x01);
    l = wiringPiI2CReadReg8(fd, 0x02);

    spl1301->i32rawPressure = (int32)h << 16 | (int32)m << 8 | (int32)l;
    spl1301->i32rawPressure = (spl1301->i32rawPressure & 0x800000) ? (0xFF000000 | spl1301->i32rawPressure) : spl1301->i32rawPressure;
}

/**
 * @brief  根据原始数据返回校准后的温度值
 * 
 *  调用此函数前需要先调用 spl1301_get_raw_temp,获取原始数据
 */
float get_spl1301_temperature(void)
{
    float fTCompensate;
    float fTsc;

    fTsc = spl1301->i32rawTemperature / (float)spl1301->i32kT;
    fTCompensate = spl1301->calib_param.c0 * 0.5 + spl1301->calib_param.c1 * fTsc;

    return fTCompensate;
}

/**
 * @brief  根据原始数据返回校准后的气压值
 * 
 *  调用此函数前需要先调用 spl1301_get_raw_pressure,获取原始数据
 */
float get_spl1301_pressure(void)
{
    float fTsc, fPsc;
    float qua2, qua3;
    float fPCompensate;

    fTsc = spl1301->i32rawTemperature / (float)spl1301->i32kT;
    fPsc = spl1301->i32rawPressure / (float)spl1301->i32kP;
    qua2 = spl1301->calib_param.c10 + fPsc * (spl1301->calib_param.c20 + fPsc * spl1301->calib_param.c30);
    qua3 = fTsc * fPsc * (spl1301->calib_param.c11 + fPsc * spl1301->calib_param.c21);

    fPCompensate = spl1301->calib_param.c00 + fPsc * qua2 + fTsc * spl1301->calib_param.c01 + qua3;

    return fPCompensate;
}

//------------------------------------------------------------------------------------------------------------------
//
//	用于 WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------

/**
  * @brief  spl1301 根据引脚转换为通道获取相应数值
  */
static int myAnalogRead(struct wiringPiNodeStruct *node, int pin)
{
    /* 0为压力通道，1为温度通道 */
    int channel = pin - node->pinBase;

    if(0 == channel)
    {
        /* 先获取原始数据 */
        spl1301_get_raw_pressure();
        // 此处
        /* 再根据内部ram定标数据进行转换 */
        return get_spl1301_pressure();
    }
    else if(1 == channel)
    {
        spl1301_get_raw_temp();
        return get_spl1301_temperature();
    }

    log_e("spl1301 channel range in [0, 1]");
    return -1;
}



/**
 * @brief  初始化并设置 spl1301
 * @param 
 *  int pinBase  pinBase > 64
 *
 * 注意，读取到的为 int型，使用时需要强制转换为float型
 */
int spl1301Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;

    if ((fd = wiringPiI2CSetupInterface(SPL1301_I2C_DEV, SPL1301_I2C_ADDR)) < 0)
    {
        log_e("spl1301 init failed");
        return -1;
    }

    spl1301->i32rawPressure = 0;
    spl1301->i32rawTemperature = 0;
    spl1301->chip_id = 0x10;

    spl1301_get_calib_param();
    // 采样率 = 32Hz; Pressure 超采样率 = 8;
    spl1301_rateset(PRESSURE_SENSOR, 32, 8);
    // 采样率 = 1Hz; Temperature 超采样率 = 8;
    spl1301_rateset(TEMPERATURE_SENSOR, 32, 8);

    /* 后台模式(自动开启转换 气压 及 温度)，即自动连续测量模式 */
    spl1301_start_continuous(CONTINUOUS_P_AND_T); 

    if (-1 == spl1301_dev.calib_param.c0)
    {
        return -1; //当为-1时，初始化失败(接入不是SPL1301)
    }

    // 2个通道，一个为压力值，一个为温度值
    node = wiringPiNewNode(pinBase, 2);
	if (!node)
        return -1;

    // 注册方法
    node->fd         = fd;
    node->analogRead = myAnalogRead;

    return fd;
}

