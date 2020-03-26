/*
 * @Description: 
 * @Author: chenxi
 * @Date: 2020-03-18 09:54:53
 * @LastEditTime: 2020-03-18 20:28:46
 * @LastEditors: chenxi
 */

#define LOG_TAG "DeviceThread"

#include <elog.h>
#include "../drivers/pca9685.h"
#include "../applications/pwmDevices.h"
#include "../applications/data.h"
#include "../applications/focus.h"

#include <wiringPi.h>
#include <pthread.h>

#include "DeviceThread.h"
#include "Control.h"
#include "PropellerControl.h"


/**
  * @brief  propeller_thread(推进器控制任务函数)
  * @param  void* arg
  * @retval None
  * @notice 
  */
void *propeller_thread(void *arg)
{
    Propeller_Init(); //推进器初始化
    while (1)
    {
        // 接收到的上位机数据已于 applications\server.c -> recv_thread() 解析
        if (UNLOCK == ControlCmd.All_Lock)
        {                                 //如果解锁
            Convert_RockerValue(&Rocker); //遥控数据 转换 为推进器动力
        }
        else
        {
            Propeller_Stop();
        }

        if (FOUR_AXIS == VehicleMode && UNLOCK == ControlCmd.All_Lock)
        { //安全保护措施
            FourtAxis_Control(&Rocker);
            ROV_Depth_Control(&Rocker);
        }
        else if (SIX_AXIS == VehicleMode && UNLOCK == ControlCmd.All_Lock)
        {
            //SixAxis_Control(&Rocker);
            ROV_Depth_Control(&Rocker);
        }

        Propeller_Output(); //推进器真实PWM输出
        delay(5);
    }
}

/**
  * @brief  servo_thread(舵机控制任务函数)
  * @param  void* arg
  * @retval None
  * @notice 
  */
void *devices_thread(void *arg) //高电平1.5ms 总周期20ms  占空比7.5% volatil
{
    while (1)
    {
        // 接收到的上位机数据已于 applications\server.c -> recv_thread() 解析
        if (WORK == WorkMode)
        { //工作模式
            //Extractor_Control(&ControlCmd.Arm); //吸取器控制
            RoboticArm_Control(&ControlCmd.Arm);     //机械臂控制
            Search_Light_Control(&ControlCmd.Light); //探照灯控制
            YunTai_Control(&ControlCmd.Yuntai);      //云台控制

            //Focus_Zoom_Camera_Control(&ControlCmd.Focus);//变焦摄像头控制
        }
        else if (DEBUG == WorkMode) //调试模式
        {
            // Debug_Mode(get_button_value(&ControlCmd));
        }
        delay(20);
    }
}

int devices_thread_init(void)
{
    pthread_t devices_tid, propeller_tid;
 /*   
    pca9685Init();
    pca9685PWMSetFreq(50.0);
    delay(1000); // 没有延时可能会导致 PWM 调节出现问题

    // PWM 初始置 0
    for (int i = 0; i < 16; i++)
    {
        //pca9685PWMWrite(i, 0, 0);
    }
    delay(2000);
*/
    if (pthread_create(&devices_tid, NULL, devices_thread, NULL) == -1)
    {
        log_e("devices_thread create error!");
        return -1;
    }
    if (pthread_detach(devices_tid))
    {
        log_w("devices_thread detach failed...");
        return -1;
    }

    if (pthread_create(&propeller_tid, NULL, propeller_thread, NULL) == -1)
    {
        log_e("propeller_thread create error!");
        return -1;
    }
    if (pthread_detach(propeller_tid))
    {
        log_w("propeller_thread detach failed...");
        return -1;
    }

    return 0;
}
