/*
 * @Description: io设备线程
 */

#define LOG_TAG "ioDevices"

#include "ioDevices.h"

#include <elog.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include <wiringPi.h>




// 错误状态灯，只亮红灯
void errorStatus_led(void)
{
    LED_ON (LEDR_PIN);
    LED_OFF(LEDG_PIN);
    LED_OFF(LEDB_PIN);
}

void all_led_off(void)
{
    LED_OFF(LEDR_PIN);
    LED_OFF(LEDG_PIN);
    LED_OFF(LEDB_PIN);
}


void *leds_thread(void *arg)
{
    while(1)
    {
        LED_ON(LEDB_PIN);
        sleep(1);
        LED_OFF(LEDB_PIN);
        sleep(1);

    }
}

void *button_thread(void *arg)
{
    static uint8_t button_up = 1; // 按键按松开标志，1表示松开
    while(1)
    {
        if(button_up && (LOW == digitalRead(BUTTON_PIN)))
        {
            delay(10); // 去抖动 
            button_up = 0;
            if(LOW == digitalRead(BUTTON_PIN))
            {
                printf("button down\n");
            }
        }
        else if(HIGH == digitalRead(BUTTON_PIN)) // 如果松开
        {
            button_up = 1; 	
        }
    }
}



/**
  * @brief  io设备线程
  */
int ioDevices_thread_init(void)
{
    pthread_t leds_tid;
    pthread_t buttons_tid;

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LEDR_PIN,   OUTPUT);
    pinMode(LEDG_PIN,   OUTPUT);
    pinMode(LEDB_PIN,   OUTPUT);

    pinMode(BUTTON_PIN,  INPUT);
    pullUpDnControl(BUTTON_PIN, PUD_UP);

    all_led_off();
    digitalWrite(BUZZER_PIN, LOW);

    pthread_create(&leds_tid, NULL, leds_thread, NULL);
    pthread_detach(leds_tid);

    // wringNP暂不支持 按键中断，因此只能循环检测
    pthread_create(&buttons_tid, NULL, button_thread, NULL);
    pthread_detach(buttons_tid);

    return 0;
}

