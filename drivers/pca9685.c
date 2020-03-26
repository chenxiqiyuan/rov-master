/*
 * @Description: pca9685 PWM 驱动程序
 */

#define LOG_TAG "pca9685"

#include <elog.h>
#include "pca9685.h"

#include <wiringPi.h>
#include <wiringPiI2C.h>

static pca9685_t pca9685_dev;
static pca9685_t *pca9685 = &pca9685_dev;


/**
 * @brief 获取对应 LEDX_ON_L 输出通道的寄存器地址 (datasheet P20)
 */
int baseReg(int pin)
{
  	// 计算获得对应引脚寄存器地址 (见datasheet P9)
	return (pin >= PIN_ALL ? LEDALL_ON_L : LED0_ON_L + 4 * pin);
}
 

/**
  * @brief  pca9685 设置PWM频率
  */
void pca9685PWMSetFreq(int fd, float freq)
{
	// 限制频率范围在 [40, 1000]
	freq = (freq > 1000 ? 1000 : (freq < 40 ? 40 : freq));

	/*
	* 设置 PWM 频率, prescale 寄存器公式如下: (datasheet Page 24)
	* prescale = round(osc_clock / (4096 * frequency)) - 1 , osc_clock = 25 MHz
	* round 为四舍五入,可以通过 +0.5 来实现
	*/
	int prescale = (int)(PCA9685_OSC_CLK / (4096 * freq) - 0.5f);

	// 获取 MODE 寄存器状态
	int pca9685_dev = wiringPiI2CReadReg8(fd, PCA9685_MODE1);	
	pca9685->restart = DISABLE_RESTART; // 关闭重启位

	// 按照手册 Page 24 中的说明,先设置睡眠,才能设置周期
	pca9685->sleep = SLEEP_MODE;  // 设置休眠模式
	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, pca9685_dev); 

	wiringPiI2CWriteReg8(fd, PCA9685_PRESCALE, prescale);

	pca9685->sleep = NORNAL_MODE; // 切换为正常模式
	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, pca9685_dev);

	// 等待晶振稳定后重启
	delay(10);
	pca9685->restart = ENABLE_RESTART; // 使能重启
	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, pca9685_dev);
}

/**
  * @brief  设置所有输出为0
  */
void pca9685PWMReset(int fd)
{
	wiringPiI2CWriteReg16(fd, LEDALL_ON_L    , 0x0);    // ALL_LED full ON  失能 
	wiringPiI2CWriteReg16(fd, LEDALL_ON_L + 2, 0x1000); // ALL_LED full OFF 使能
}

/**
  * @brief  向pca9685对应 pin 写入PWM值
  */
void pca9685PWMWrite(int fd, int pin, int on, int off)
{
  	int reg = baseReg(pin);

  	// 可写入位 12bit，最大值为 4095    on + off = 4095
	wiringPiI2CWriteReg16(fd, reg	 , on  & 0x0FFF);
	wiringPiI2CWriteReg16(fd, reg + 2, off & 0x0FFF);
}

/**
  * @brief  对应 pin 输出1(相当于当做普通io口用)
  */
void pca9685FullOn(int fd, int pin, int tf)
{
	int reg = baseReg(pin) + 1;	// LEDX_ON_H 寄存器
	int state = wiringPiI2CReadReg8(fd, reg);

	// 根据 tf 设置 第4bit 为 0 or 1
	state = tf ? (state | 0x10) : (state & 0xEF);

	wiringPiI2CWriteReg8(fd, reg, state);

	//  full-off 优先级高于 full-on (datasheet P23)
	if (tf)
		pca9685FullOff(fd, pin, 0);
}

/**
  * @brief  对应 pin 输出0(相当于当做普通io口用)
  */
void pca9685FullOff(int fd, int pin, int tf)
{
	int reg = baseReg(pin) + 3;	// LEDX_OFF_H 寄存器
	int state = wiringPiI2CReadReg8(fd, reg);

	// 根据 tf 设置 第4bit 为 0 or 1
	state = tf ? (state | 0x10) : (state & 0xEF);

	wiringPiI2CWriteReg8(fd, reg, state);
}




//------------------------------------------------------------------------------------------------------------------
//
//	用于 WiringPi functions
//
//------------------------------------------------------------------------------------------------------------------


/**
  * @brief  pwm 写入
  */
static void myPwmWrite(struct wiringPiNodeStruct *node, int pin, int value)
{
	int fd   = node->fd;
	int ipin = pin - node->pinBase;

	if (value >= 4096)
		pca9685FullOn(fd, ipin, 1);
	else if (value > 0)
		pca9685PWMWrite(fd, ipin, 0, value);
	else
		pca9685FullOff(fd, ipin, 1);
}

/**
  * @brief  数字量写入 (相当于当做普通io口)
  */
static void myOnOffWrite(struct wiringPiNodeStruct *node, int pin, int value)
{
	int fd   = node->fd;
	int ipin = pin - node->pinBase;

	if (value)
		pca9685FullOn(fd, ipin, 1);
	else
		pca9685FullOff(fd, ipin, 1);
}


/**
 * @brief  初始化并设置 pca9685
 * @param 
 *  int pinBase  pinBase > 64
 * 	float freq   频率范围在 [40, 1000]
 *
 *  wiringPi 中，pin编号小于64认为是板载GPIO，如果编号大于64则认为是外扩GPIO
 * 	因此 pinBase > 64
 */
int pca9685Setup(const int pinBase, float freq)
{
	static int fd;
	struct wiringPiNodeStruct *node;

  	// PCA9685_OE_PIN，即 GPIOG11 低电平使能
  	pinMode(PCA9685_OE_PIN, OUTPUT);
  	digitalWrite(PCA9685_OE_PIN, LOW);

  	fd = wiringPiI2CSetupInterface(PCA9685_I2C_DEV, PCA9685_I2C_ADDR);
  	if (fd < 0)
  	{
    	log_e("pca9685 init failed");
    	return fd;
  	}

	if (freq < 40 || freq > 1000)
  	{
     	log_e("pca9685 freq range in [40, 1000]");
  	}
	// 设置 PWM 频率，启动输出
	pca9685PWMFreq(fd, freq);
	
	// 创建节点 16 pins [0..15] + [16] for all
	node = wiringPiNewNode(pinBase, PIN_ALL + 1);
	if (!node)
		return -1;

  	// 注册方法
	node->fd			= fd;
	node->pwmWrite		= myPwmWrite;
	node->digitalWrite	= myOnOffWrite;

	// 重置所有输出
	pca9685PWMReset(fd);

  	return fd;
}