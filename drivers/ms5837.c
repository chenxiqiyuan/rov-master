/*
 * @Description: MS5837 深度传感器驱动
 *
 *       Notes: 水深传感器设备驱动
 *   Attention: SCL - E10 (黑色)   
 *				SDA - E12 (黄色)   
 */
#define LOG_TAG "ms5837"

#include "ms5837.h"

#include <elog.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

static ms5837_t ms5837_dev;
static ms5837_t *ms5837 = &ms5837_dev;

/**
  * @brief  crc4校验函数 (datasheet P12)
  * @param  数组
  * @retval 返回crc 4bit校验
  */
uint8_t _crc4(uint16_t *n_prom)
{
	int32_t  cnt;
	uint32_t n_rem = 0; // crc 余数
	uint8_t  n_bit;

	n_prom[0] = ((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
	n_prom[7] = 0; // Subsidiary value, set to 0
	for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
	{ 	
		// choose LSB or MSB
		if (cnt%2==1) 
			n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else 
			n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000)) 
				n_rem = (n_rem << 1) ^ 0x3000;
			else 
				n_rem = (n_rem << 1);
		}
	}
	n_rem= ((n_rem >> 12) & 0x000f); // final 4-bit remainder is CRC code
	return (n_rem ^ 0x00);
}

/**
  * @brief  ms5837 复位
  */
void ms5837_reset(int fd)
{
	wiringPiI2CWrite(fd, MS583703BA_RESET);
}

/**
  * @brief  ms5837获取出厂标定参数
  * @param  None
  * @retval 返回出厂标定参数 crc校验 是否成功标志：1成功，-1失败
  *  若成功表示为ms5837传感器
  *  若失败表示为其他类型传感器或无
  */
int ms5837_get_calib_param(int fd)
{	
	static int i;
	for (i = 0; i <= 6; i++) 
	{
		// 读取prom中的出厂标定参数
    	ms5837->c[i] = \
		wiringPiI2CReadReg16(fd, MS583703BA_PROM_RD + (i * 2));
	}
	/* crc校验为 C[0]的 bit[15,12] */
	ms5837->crc = (uint8_t)(ms5837->c[0] >> 12);
	// 工厂定义参数为 c[0] 的bit[14,0]
	ms5837->factory_id = (uint8_t)(ms5837->c[0] & 0x0fff);

	/* 
	 * crc校验为用于判断ms5837是否初始化成功 
	 * 或者说是否是 ms5837传感器
	*/
	if(ms5837->crc == _crc4(ms5837->c))
	{
		return 1;
	}
	return -1;
}


 /**
 * @brief  ms5837 获取转换数据
 * @param 
 *  uint8_t command  带精度温度命令  带精度温度压力(见头文件)
 * @retval
 *  uint32_t 数据结果
 */
uint32_t ms5837_get_conversion(int fd, uint8_t command)
{
 	uint8_t temp[3];
	uint32_t conversion;
	// 1.先写入转换命令(即指定转换传感器及精度) (datasheet P11)
	wiringPiI2CWrite(fd, command);

 	// 2.延时等待转换完成  读取8196转换值得关键，必须大于 datasheet P2页中的18.08毫秒
	delay(30);
	
	// 3.在写入 ADC read命令
	wiringPiI2CWrite(fd, MS583703BA_ADC_RD);

	// 4.读取 24bit的转换数据 高位在前
	temp[0] = wiringPiI2CRead(fd); // bit 23-16
	temp[1] = wiringPiI2CRead(fd); // bit 8-15
	temp[2] = wiringPiI2CRead(fd); // bit 0-7

	conversion = ((uint32_t)temp[0] <<16) | ((uint32_t)temp[1] <<8) | ((uint32_t)temp[2]);

	return conversion;
}

/**
 * @brief  获取并计算温度值
 *  此时的温度值还没经过补偿，并不准确
 */
void ms5837_cal_raw_temperature(int fd)
{
	// 获取原始温度数字量
	ms5837->D2_Temp = ms5837_get_conversion(fd, MS583703BA_D2_OSR_2048);
	// 实际温度与参考温度之差 (公式见datasheet P7)
	ms5837->dT = ms5837->D2_Temp - (((uint32_t)ms5837->c[5]) * 256);
	// 实际的温度
	ms5837->TEMP = 2000 + ms5837->dT * ((uint32_t)ms5837->c[6]) / 8388608; // 8388608 = 2^23,这里不采用右移23位，因为该数据为有符号 
}

/**
 * @brief  获取并计算压力值
 *  并进行温度补偿
 */
void ms5837_cal_pressure(int fd)
{
	int64_t  Ti, OFFi, SENSi;
	uint32_t dT_squ;
	uint32_t temp_minus_squ, temp_plus_squ;

	// 获取原始压力数字量
	ms5837->D1_Pres= ms5837_get_conversion(fd, MS583703BA_D1_OSR_8192);
	// 实际温度偏移
	ms5837->OFF  = (int64_t)ms5837->c[2] * 65536 + ((int64_t)ms5837->c[4] * ms5837->dT) / 128;
	// 实际温度灵敏度
	ms5837->SENS = (int64_t)ms5837->c[1] * 32768 + ((int64_t)ms5837->c[3] * ms5837->dT) / 256;

	dT_squ   = (ms5837->dT * ms5837->dT); // dT的2次方
	temp_minus_squ = (2000 - ms5837->TEMP) * (2000 - ms5837->TEMP); // 温度差的2次方

	/* 对温度和压力进行二阶修正 (datasheet P8) */
	if(ms5837->TEMP < 2000) // 低温情况:低于20℃时
	{
		Ti    = 3 * dT_squ / 0x200000000;
		OFFi  = 3 * temp_minus_squ / 2;
		SENSi = 5 * temp_minus_squ / 8;

		if(ms5837->TEMP < -1500) // 超低温情况:低于-15℃时
		{
			temp_plus_squ = (ms5837->TEMP + 1500) * (ms5837->TEMP + 1500); // 温度和的2次方
			OFFi  += 7 * temp_plus_squ;
			SENSi += 4 * temp_plus_squ;
		}
	}
	else // 高温情况:高于20℃时
	{
		Ti    = 2 * dT_squ / 0x2000000000;
		OFFi  = 1 * temp_minus_squ / 16;
		SENSi = 0; 
	}
	ms5837->OFF  -= OFFi;
	ms5837->SENS -= SENSi;	

	// 温度补偿后的压力值
	ms5837->P = (ms5837->SENS / 0x200000 - ms5837->OFF) / 0x1000;

	// 实际温度值
	ms5837->temperature = (ms5837->TEMP - Ti) / 100;
	// 实际压力值
	ms5837->pressure = ms5837->P / 10;
}


//------------------------------------------------------------------------------------------------------------------
//
//	用于 WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------


/**
  * @brief  ms5837 根据引脚转换为通道获取相应数值
  */
static int myDigitalRead(struct wiringPiNodeStruct *node, int pin)
{
    /* 0为压力通道，1为温度通道 */
    int channel = pin - node->pinBase;
    int fd      = node->fd;

    /* 先获取温度数据，因为需要进行温度补偿 
	 * 在计算压力函数中，会计算温度二阶，使得温度更加准确
	 * 因此不管获取压力或者温度，都应调用这以下两个函数
	*/
    ms5837_cal_raw_temperature(fd);
	ms5837_cal_pressure(fd);

    if(PRESSURE_SENSOR == channel)
    {
        return ms5837->pressure;
    }
    else if(TEMPERATURE_SENSOR == channel)
    {
        return ms5837->temperature;
    }

    log_e("ms5837 channel range in [0, 1]");
    return -1;
}


/**
 * @brief  初始化并设置 ms5837
 * @param 
 *  int pinBase  pinBase > 64
 */
int ms5837Setup(const int pinBase)
{
    static int fd;
	struct wiringPiNodeStruct *node;

    if ((fd = wiringPiI2CSetupInterface(MS5837_I2C_DEV, MS583703BA_I2C_ADDR)) < 0)
    {
        log_e("ms5837 i2c init failed");
        return -1;
    }

	/* 先复位再读取prom数据 (datasheet P10) */
	ms5837_reset(fd);	     
	
	// 获取标定参数
	if(ms5837_get_calib_param(fd) < 0) 
	{
		return -1;
	}
	
    // 创建节点，2个通道，一个为压力值，一个为温度值
    node = wiringPiNewNode(pinBase, 2);
	if (!node)
    {
        log_e("ms5837 node create failed");
        return -1;
    }

    // 注册方法
    node->fd         = fd;
    node->analogRead = myDigitalRead;

	return fd;
}

