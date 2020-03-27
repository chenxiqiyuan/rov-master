
#ifndef __IO_DEVICES_H__
#define __IO_DEVICES_H__

#include "../user/DataType.h"

#include <wiringPi.h>

// 蜂鸣器引脚号
#define BUZZER_PIN 12 
// 按键引脚号
#define BUTTON_PIN 24  

// RGB灯引脚号
#define LEDR_PIN 13  
#define LEDG_PIN 14 
#define LEDB_PIN 10  


// RGB灯共阳极接到+3.3V，电平 0亮 1灭
#define LED_ON(pin)  digitalWrite(pin, LOW)
#define LED_OFF(pin) digitalWrite(pin, HIGH)

#define Buzzer_ON(pin)  digitalWrite(pin, HIGH)
#define Buzzer_OFF(pin) digitalWrite(pin, LOW)



int ioDevices_thread_init(void);


#endif
