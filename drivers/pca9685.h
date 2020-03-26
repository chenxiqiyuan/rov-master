/*
 * @Description: 通过 I2C 控制 PCA9685 来使用 PWM 功能
 * @Author: chenxi
 * @Date: 2020-02-10 12:15:34
 * @LastEditTime: 2020-03-15 12:52:46
 * @LastEditors: chenxi
 */

#ifndef __PWM_H__
#define __PWM_H__

#include "../user/DataType.h"

#define PCA9685_I2C_DEV  "/dev/i2c-0" // PCA9685 使用的 I2C设备
#define PCA9685_I2C_ADDR 0x40         // 默认 PCA9685 I2C 地址
#define PCA9685_OE_PIN   7            // 芯片使能引脚 -> GPIO11引脚序号为 7

#define PCA9685_OSC_CLK 25000000.0f   // PCA9685晶振时钟频率Hz

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1    0x0  
#define PCA9685_PRESCALE 0xFE // prescaler to program the output frequency

#define PIN_ALL      16

#define LED0_ON_L    0x6
#define LED0_ON_H    0x7
#define LED0_OFF_L   0x8
#define LED0_OFF_H   0x9

#define LEDALL_ON_L  0xFA
#define LEDALL_ON_H  0xFB
#define LEDALL_OFF_L 0xFC
#define LEDALL_OFF_H 0xFD

/* sleep 模式 */
#define NORNAL_MODE     0b0
#define SLEEP_MODE      0b1

/* restart 开关 */
#define DISABLE_RESTART 0b0
#define ENABLE_RESTART  0b1


typedef struct {
    uint8 allcall :1; 
    uint8 sub3    :1; 
    uint8 sub2    :1;
    uint8 sub1    :1;
    uint8 sleep   :1;
    uint8 ai      :1; 
    uint8 extclk  :1;
    uint8 restart :1;
} pca9685_t; // pca9685 MODE1 寄存器配置结构体



extern int pca9685Setup(const int pinBase, float freq/* = 50*/);

extern void pca9685PWMFreq(int fd, float freq);
extern void pca9685PWMReset(int fd);
extern void pca9685PWMWrite(int fd, int pin, int on, int off);
extern void pca9685PWMRead(int fd, int pin, int *on, int *off);

extern void pca9685FullOn(int fd, int pin, int tf);
extern void pca9685FullOff(int fd, int pin, int tf);


#endif
