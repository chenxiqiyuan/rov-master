/*
 * @Description: io设备线程(RGB、KEY、BUZZER)
 */

#define LOG_TAG "ioDevices"

#include "ioDevices.h"

#include <elog.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <wiringPi.h>

// 定义模拟pwm设备描述符，并指定pin、name
static softPWM_t ledr = {
    .pin = LEDR_PIN,
    .name = "ledr",
};
static softPWM_t ledg = {
    .pin = LEDG_PIN,
    .name = "ledg",
};
static softPWM_t ledb = {
    .pin = LEDB_PIN,
    .name = "ledb",
};
static softPWM_t beep = {
    .pin = BUZZER_PIN,
    .name = "beep",
};



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


/**
 * @brief  模拟pwm设备设置
 * @param 
 *  softPwm_t *pwm  pwm描述符
 *  uint32_t  time  持续时间 (单位 ms)
 *  uint8_t   per   周期 (单位 ms)
 *  uint8_t   duty  占空比 (0~100)
 *  uint8_t   flag  如果为1，则为无尽模式
 */
void sotfPwmSet(softPWM_t *pwm,
                uint32_t  time, // 持续时间
                uint32_t  per,
                uint8_t   duty, // 占空比 (0~100)
                uint8_t   flag)
{
    /* 
     * div取决于 softPwm_process所在线程休眠的时间 
     * eg.休眠时间为10ms，则div = 10 (建议休眠时间为10ms，超过设定周期，则无效)
    */
    static uint8_t div = 10;

    pwm->time   = time/div; // 持续时间
    pwm->period = per /div; // 周期
    pwm->duty   = duty;     // 占空比 (0~100)
    pwm->flag   = flag;     // 如果为1,则为无尽模式
}


/**
 * @brief  模拟pwm设备任务
 *  该任务放置于线程中，并设定休眠时间
 */
void softPwm_process(softPWM_t *pwm)
{
    if(pwm->time >= 1) 
        pwm->time--; // 持续时间-1

    // 判断 持续时间未清理 或 无尽模式是否开启
    if((pwm->time != 0) || (pwm->flag == 1)) 
    {
        pwm->cnt++; 
        // 计数 >= 周期，清零
        pwm->cnt = pwm->cnt >= pwm->period ? 0 : pwm->cnt; 

        // 小于占空比时，继续输出高
        if(pwm->cnt <= pwm->period * pwm->duty/100) 	
            IO_OUPUT_HIGH(pwm->pin); 
        else
            IO_OUPUT_LOW(pwm->pin);
    }
    else 
    {
        IO_OUPUT_LOW(pwm->pin);		
    }
}

/**
 * @brief  模拟pwm设备线程
 *
 *  注意：休眠时间与周期有关，若休眠时间更改，sotfPwmSet中的静态变量div需要对应修改
 */
void *softPWM_thread(void *arg)
{
    sotfPwmSet(&ledr, 1000, 500, 50 , 0);
    sotfPwmSet(&ledg, 1000, 500, 50 , 0);
    sotfPwmSet(&ledb, 1000, 1000, 50 , 1);
    sotfPwmSet(&beep, 500, 200, 10, 0);
    while(1)
    {
        delay(10); // 休眠时间与周期有关

        /* 模拟PWM设备任务 */
        softPwm_process(&ledr); 
        softPwm_process(&ledg);
        softPwm_process(&ledb);
        softPwm_process(&beep);
    }
}

/**
 * @brief  按键与蜂鸣器线程
 */
void *button_thread(void *arg)
{
    static uint8_t button_up = 1; // 按键按松开标志，1表示松开

    while(1)
    {
        delay(10);
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
  * @brief  io设备线程初始化
  */
int ioDevs_thread_init(void)
{
    pthread_t softPWM_tid;
    pthread_t buttons_tid;

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LEDR_PIN,   OUTPUT);
    pinMode(LEDG_PIN,   OUTPUT);
    pinMode(LEDB_PIN,   OUTPUT);

    pinMode(BUTTON_PIN,  INPUT);
    pullUpDnControl(BUTTON_PIN, PUD_UP); // 上拉

    digitalWrite(LEDR_PIN,   LOW);
    digitalWrite(LEDG_PIN,   LOW);
    digitalWrite(LEDB_PIN,   LOW);

    digitalWrite(BUZZER_PIN, LOW);
   
    pthread_create(&softPWM_tid, NULL, softPWM_thread, NULL);
    pthread_detach(softPWM_tid);

    // wringNP暂不支持 按键中断，因此只能循环检测
    pthread_create(&buttons_tid, NULL, button_thread, NULL);
    pthread_detach(buttons_tid);

    log_d("ioDevices thread init");
    return 0;
}

