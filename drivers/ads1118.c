
/*
 * @Description: ads1118 ADC 驱动程序
 *
 * Copyright (c) 2019-2020, Ian, <zengwangfa@outlook.com>
 */

#define LOG_TAG "ads1118"

#include "ads1118.h"

#include <elog.h>
#include <stdio.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

static ads1118_t ads1118_dev;
static ads1118_t *ads1118 = &ads1118_dev;

// 存放增益放大参数
static float vref[6] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

/**
  * @brief  ads1118 spi数据写入与读取
  */
static int16_t ads1118_transmit(int fd, uint8_t *data)
{
    unsigned char buff[4] = {0};

    buff[0] = data[1]; // SPI传输高位在前 (datasheet P24)
    buff[1] = data[0];
    buff[2] = 0;
    buff[3] = 0;

    wiringPiSPIDataRW(fd, buff, 4);

    return buff[0] << 8 | buff[1]; // SPI传输高位在前
}


/**
  * @brief  ads1118 根据通道获取数据
  */
static int16_t ads1118_convert(int fd, int channel)
{
    switch (channel) {
        case 0:
            ads1118->regs.mux = MUX_S0; break;
        case 1:
            ads1118->regs.mux = MUX_S1; break;
        case 2:
            ads1118->regs.mux = MUX_S2; break;
        case 3:
            ads1118->regs.mux = MUX_S3; break;
        default:
            log_e("ads1118 channel range in [0, 3]");
    }

    return ads1118_transmit(fd, (uint8_t *)ads1118);
}



//------------------------------------------------------------------------------------------------------------------
//
//	用于 WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------

/**
  * @brief  ads1118 根据增益转换实际电压值
  */
static int myAnalogRead(struct wiringPiNodeStruct *node, int pin)
{
    // 获取是 ads1118 的第几通道
    int channel = pin - node->pinBase;
    int fd      = node->fd;
    int vol     = 0;

    int adcVal = ads1118_convert(fd, channel);

    // 注意FS为 正负2.048，所以计算时为2.048/32768. (满量程是65535)
    // verf[ads1118->regs.pga] 即为 FSR增益值 ±2.048
    vol = adcVal * (vref[ads1118->regs.pga] / 32768);

    return vol;
}


/**
 * @brief  初始化并设置 ads1118
 * @param 
 *  int pinBase  pinBase > 64
 *
 *  可修改相关配置: 增益放大、采样率...
 */
int ads1118Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;

    fd = wiringPiSPISetup(1, ADS1118_OSC_CLK); // ads1118使用的 /dev/spidev1.0   1MHz
    if (fd < 0)
    {
        log_e("ads1118 spi init failed");
    }
    /* 设置配置寄存器 */
    ads1118->regs.ss = SS_NONE;               // 不启动单发转换
    ads1118->regs.mux = MUX_S0;               // 通道 AIN0
    ads1118->regs.pga = PGA_2_048;            // 可编程增益放大 ±2.048v
    ads1118->regs.mode = MODE_CONTINOUS;      // 连续转换模式
    ads1118->regs.dr = DR_128_SPS;            // 每秒采样率 128
    ads1118->regs.ts_mode = TS_MODE_ADC;      // ADC 模式
    ads1118->regs.pull_up_en = PULL_UP_EN_ON; // 数据引脚上拉使能
    ads1118->regs.nop = NOP_DATA_VALID;       // 有效数据,更新配置寄存器
    ads1118->regs.reserved = RESERVED_BIT;    // 保留位

    ads1118_transmit(fd, (uint8_t *)ads1118);     // 写入配置寄存器

	// 创建节点 4 pins 共4个通道
    node = wiringPiNewNode(pinBase, 4);
    if (!node)
    {
        log_e("ads1118 node create failed");
		return -1;
    }
    node->fd         = fd;
    node->analogRead = myAnalogRead;

    return fd;
}
