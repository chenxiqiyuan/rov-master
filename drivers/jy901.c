/*
 * @Description: jy901 九轴数据读取程序
 */
 
#define LOG_TAG "jy901"

#include "jy901.h"

#include <elog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <wiringPi.h>
#include <wiringSerial.h>

static jy901_t jy901_dev;
static jy901_t *jy901 = &jy901_dev;

short Compass_Offset_Angle = 0;//指南针补偿角度   由于受到板子磁场干扰，需要加一个补偿角度  -360 ~ +360
extern Sensor_Type Sensor;

// 函数声明
void jy901_convert(JY901_Type *pArr);


/**
  * @brief  copeJY901_data为串口中断调用函数，串口每收到一个数据，调用一次这个函数
  *         判断数据是哪种数据，然后将其拷贝到对应的结构体中
  */ 
void copeJY901_data(uint8_t Data)
{
	static uint8_t rxBuffer[20] = {0}; // 数据包
	static uint8_t rxCheck = 0;		 // 尾校验字
	static uint8_t rxCount = 0;		 // 接收计数
	static uint8_t i = 0;				 // 接收计数

	rxBuffer[rxCount++] = Data; // 将收到的数据存入缓冲区中

	if (rxBuffer[0] != 0x55)
	{
		// 数据头不对，则重新开始寻找0x55数据头
		rxCount = 0; // 清空缓存区
		return;
	}
	if (rxCount < JY901_PACKET_LENGTH)
	{
		return;
	} // 数据不满11个，则返回

	/*********** 只有接收满11个字节数据 才会进入以下程序 ************/
	for (i = 0; i < 10; i++)
	{
		rxCheck += rxBuffer[i]; //校验位累加
	}

	if (rxCheck == rxBuffer[JY901_PACKET_LENGTH - 1]) // 判断数据包校验是否正确
	{
		// 判断数据是哪种数据，然后将其拷贝到对应的结构体中，有些数据包需要通过上位机打开对应的输出后，才能接收到这个数据包的数据
		switch (rxBuffer[1])
		{
			case 0x50:	memcpy(&jy901->stcTime,    &rxBuffer[2],8);break; // 拷贝舍去 包头与数据个数位
			case 0x51:	memcpy(&jy901->stcAcc,     &rxBuffer[2],8);break;
			case 0x52:	memcpy(&jy901->stcGyro,    &rxBuffer[2],8);break;
			case 0x53:	memcpy(&jy901->stcAngle,   &rxBuffer[2],8);break;
			case 0x54:	memcpy(&jy901->stcMag,     &rxBuffer[2],8);break;
			case 0x55:	memcpy(&jy901->stcDStatus, &rxBuffer[2],8);break;
			case 0x56:	memcpy(&jy901->stcPress,   &rxBuffer[2],8);break;
			case 0x57:	memcpy(&jy901->stcLonLat,  &rxBuffer[2],8);break;
			case 0x58:	memcpy(&jy901->stcGPSV,    &rxBuffer[2],8);break;
			case 0x59:	memcpy(&jy901->stcQ,       &rxBuffer[2],8);break;
		}
		rxCount = 0; // 清空缓存区
		rxCheck = 0; // 校验位清零

		jy901_convert(&Sensor.JY901); // JY901数据转换
	}
	else // 错误清零
	{
		rxCount = 0; // 清空缓存区
		rxCheck = 0; // 校验位清零
		return;
	}
}

// Sensor.JY901 数据转换
void jy901_convert(JY901_Type *pArr)
{
	pArr->Acc.x = (float)jy901->stcAcc.a[0] / 2048; // 32768*16
	pArr->Acc.y = (float)jy901->stcAcc.a[1] / 2048;
	pArr->Acc.z = (float)jy901->stcAcc.a[2] / 2048;

	pArr->Gyro.x = (float)jy901->stcGyro.w[0] / 2048 * 125; // 32768*2000
	pArr->Gyro.y = (float)jy901->stcGyro.w[1] / 2048 * 125;
	pArr->Gyro.z = (float)jy901->stcGyro.w[2] / 2048 * 125;

	pArr->Euler.Roll = (float)jy901->stcAngle.angle[0] / 8192 * 45; // 32768*180;
	pArr->Euler.Pitch = (float)jy901->stcAngle.angle[1] / 8192 * 45;
	pArr->Euler.Yaw = (float)jy901->stcAngle.angle[2] / 8192 * 45;

	// 偏移角度 等于当指向 正北时的角度(-360 ~ +360 )
	if (Compass_Offset_Angle != 0) // 如果未设置补偿角度，则不进行角度补偿【补偿为 正角度】
	{
		pArr->Euler.Yaw -= Compass_Offset_Angle; // 减去补偿角度
		if (pArr->Euler.Yaw < -180){
			pArr->Euler.Yaw += 360; // 角度反向补偿
		}
		if (pArr->Euler.Yaw > 180){
			pArr->Euler.Yaw -= 360; // 角度反向补偿
		}
	}

	pArr->Mag.x = jy901->stcMag.h[0];
	pArr->Mag.y = jy901->stcMag.h[1];
	pArr->Mag.z = jy901->stcMag.h[2];

	pArr->Temperature = (float)jy901->stcAcc.T / 100;
}


/**
  * @brief  打开对应 JY901 串口设备
  */
int jy901Setup(void)
{
	static int fd = 0;

	if ((fd = serialOpen(JY901_UART_DEV, JY901_UART_BAUD)) < 0)
	{
		log_e("Unable to open serial device: %s", strerror(errno));
		log_e("JY901 init failed");
		return -1;
	}

	return fd;
}
