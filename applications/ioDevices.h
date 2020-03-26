/*
 * @Description: 蜂鸣器
 * @Author: chenxi
 * @Date: 2020-02-02 16:58:32
 * @LastEditTime : 2020-02-10 17:05:19
 * @LastEditors  : chenxi
 */

#ifndef __IO_DEVICES_H__
#define __IO_DEVICES_H__

#include "../user/DataType.h"

#define KEY_PIN 24    //PA7   按键IO
#define Buzzer_PIN 12 //PC0   蜂鸣器IO

typedef struct
{
    uint8 count; // 响的次数
    uint32 time; // 响的时间长度(ms)
} Buzzer_Type;

// RGB灯引脚号
#define LED_Red 13   // PC1
#define LED_Green 14 // PC2
#define LED_Blue 10  // PC3

// OV Camera 闪光灯
// #define LED_Camera 141 // PE0 高电平点亮

// 探照灯
// #define Light_PIN 114 //PD0

// RGB灯共阳极接到+3.3V，电平 0亮 1灭
#define LED_ON(led_pin) digitalWrite(led_pin, LOW)
#define LED_OFF(led_pin) digitalWrite(led_pin, HIGH)
// #define LED_Turn(led_pin, status) rt_pin_write(led_pin, status = !status) //取反

typedef struct
{
    uint32 Bling_Contiune_Time; //闪烁持续时间
    uint32 Bling_Period;        //闪烁周期
    float Bling_Percent;        //闪烁占空比
    uint32 Bling_Cnt;           //闪烁计数器
    uint32 Pin;                 //引脚
    uint8 Endless_Flag;         //无尽模式
} Bling_Light;

void LED_Init(void);
void ErrorStatus_LED(void);
void ALL_LED_OFF(void);
// void Light_Control(uint8 *action);

// void led_voltage_task(void);
// void system_init_led_blink(void);

// void Bling_Set(Bling_Light *Light,
//                uint32 Continue_time, //持续时间
//                uint32 Period,        //周期100ms~1000ms
//                float Percent,        //0~100%
//                uint32 Cnt,
//                uint32 Pin,
//                uint8 Flag);

// void Bling_Process(Bling_Light *Light);
// void Bling_Working(uint8 bling_mode);

// extern Bling_Light Light_Red, Light_Green, Light_Blue;
// extern uint8 Bling_Mode;

void key_down(void);

void Buzzer_Init(void);

#define Buzzer_ON() digitalWrite(Buzzer_PIN, HIGH)
#define Buzzer_OFF() digitalWrite(Buzzer_PIN, LOW)

/* count为响的次数  length响的时间长度(ms)  */
void Buzzer_Set(Buzzer_Type *buzzer, uint8 count, uint32 time);
void Buzzer_Process(Buzzer_Type buzzer);

extern Buzzer_Type Beep;

#endif
