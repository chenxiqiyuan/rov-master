/*
 * @Description: 开启各个传感器线程 (深度传感器、九轴、CPU 设备数据解析并获取)
 */

#define LOG_TAG "sensor"

#include "../drivers/cpu_status.h"
#include "../drivers/ads1118.h"
#include "../drivers/spl1301.h"
#include "../drivers/ms5837.h"
#include "../drivers/jy901.h"
#include "../tools/filter.h"
#include "sensor.h"

#include <elog.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <wiringPi.h>
#include <wiringSerial.h>

/*----------------------- Variable Declarations -----------------------------*/
char *Depth_Sensor_Name[3] = {"MS5837", "SPL1301", "null"};

// extern struct rt_event init_event; /* ALL_init 事件控制块 */

Sensor_Type Sensor; //传感器参数
depthSensor_t depthSensor_dev;
depthSensor_t *depthSensor = &depthSensor_dev;

/*----------------------- Function Implement --------------------------------*/
/**
  * @brief  sensor_lowSpeed_callback_fun(低速获取传感器任务函数)
  */
void *sensor_lowSpeed_callback_fun(void *arg)
{
    if(ads1118Setup(100) < 0) //ADC电压采集初始化
    {
        log_e("ads1118 Setup failed");
    }
    while (1)
    {
        Sensor.CPU.Usage = get_cpu_usage();
        Sensor.CPU.Temperature = get_cpu_temp(); 

        //Sensor.PowerSource.Voltage = get_voltage_value(); //获取电源电压值
        //Sensor.PowerSource.Current = get_current_value();

    }
    return NULL;
}

//深度传感器数据转换
void depthSensor_Data_Convert(void)
{
    static uint32_t value[10] = {0};
    static uint8_t ON_OFF = 0; //自锁开关
    static uint8_t i = 0;

    if (SPL1301 == Sensor.DepthSensor.Type) //歌尔 SPL1301
    {


        if (ON_OFF == 0)
        {
            ON_OFF = 1;
            //Sensor.DepthSensor.Init_PessureValue = get_spl1301_pressure(); //获取初始化数据
        }
        for (i = 0; i < 10; i++)
        {
            value[i++] = get_spl1301_pressure(); //获取1次数据
        }
        //Sensor.DepthSensor.Temperature = get_spl1301_temperature();
        //Sensor.DepthSensor.PessureValue = Bubble_Filter(value);
        /* 深度数值 单位为cm   定标系数为 1.3 单位/cm */
        //Sensor.DepthSensor.Depth = ((Sensor.DepthSensor.PessureValue - Sensor.DepthSensor.Init_PessureValue) / 20);
    }
    else if (MS5837 == Sensor.DepthSensor.Type) //使用MS5837
    {

    }
}




/**
 * @brief  深度传感器处理线程
 */
void *depthSensor_thread(void *arg)
{
    /* 依次初始化 ms5837、spl1301，来判断接入的是哪种传感器 */
    if(ms5837Setup(DEPTH_SENSOR_PIN_BASE) > 0)
    {
        depthSensor->name = "ms5837";
        log_i("ms5837 init");
        while(1)
        {
            delay(10);
            //get_ms5837_depthSensor();
        }
    }
    else if(spl1301Setup(DEPTH_SENSOR_PIN_BASE) > 0)
    {
        depthSensor->name = "spl1301";
        log_i("spl1301 init");
        while(1)
        {
            delay(10);
            //get_spl1301_depthSensor();
        }
    }
    else
    {
        depthSensor->name = "no plugged a depthSensor";
        log_e("%s", depthSensor->name);
    }
    return NULL;
}

void *jy901_thread(void *arg)
{
    int fd = jy901Setup();
    while (1)
    {
        delay(10);
        while (serialDataAvail(fd))
        {
            copeJY901_data((uint8_t)serialGetchar(fd));
        }
    }
    return NULL;
}



int sensor_thread_init(void)
{
    pthread_t jy901_tid;
    pthread_t depthSensor_tid;
    pthread_t sensor_lowSpeed_tid;

    memset(&Sensor, 0, sizeof(Sensor_Type));
    Sensor.DepthSensor.Type = SPL1301;

    if (pthread_create(&jy901_tid, NULL, jy901_thread, NULL) < 0)
    {
        log_e("JY901_thread create error!");
        return 1;
    }
    if (pthread_detach(jy901_tid))
    {
        log_w("JY901_thread detach failed...");
        return -2;
    }

    if (pthread_create(&sensor_lowSpeed_tid, NULL, sensor_lowSpeed_callback_fun, NULL) < 0)
    {
        log_e("sensor_lowSpeed_thread create error!");
        return 1;
    }
    if (pthread_detach(sensor_lowSpeed_tid)){
        log_w("sensor_lowSpeed_thread detach failed...");
        return -2;
    }

    if (pthread_create(&depthSensor_tid, NULL, depthSensor_thread, NULL) < 0)
    {
        log_e("DepthSensor_thread create error!");
        return 1;
    }
    if (pthread_detach(depthSensor_tid))
    {
        log_w("DepthSensor_thread detach failed...");
        return -2;
    }

    return 0;
}

// 打印传感器信息
void print_sensor_info(void)
{
    log_d("      variable      |   value");
    log_d("--------------------|------------");

    log_d("        Roll        |  %+0.3f", Sensor.JY901.Euler.Roll);
    log_d("        Pitch       |  %+0.3f", Sensor.JY901.Euler.Pitch);
    log_d("        Yaw         |  %+0.3f", Sensor.JY901.Euler.Yaw);
    log_d("--------------------|------------");
    log_d("        Acc.x       |  %+0.3f", Sensor.JY901.Acc.x);
    log_d("        Acc.y       |  %+0.3f", Sensor.JY901.Acc.y);
    log_d("        Acc.z       |  %+0.3f", Sensor.JY901.Acc.z);
    log_d("--------------------|------------");
    log_d("       Gyro.x       |  %+0.3f", Sensor.JY901.Gyro.x);
    log_d("       Gyro.y       |  %+0.3f", Sensor.JY901.Gyro.y);
    log_d("       Gyro.z       |  %+0.3f", Sensor.JY901.Gyro.z);
    log_d("  JY901_Temperature |  %+0.3f", Sensor.JY901.Temperature);
    log_d("--------------------|------------");
    log_d("       Voltage      |  %0.3f",  Sensor.PowerSource.Voltage); // 电压
    log_d("       Current      |  %0.3f",  Sensor.PowerSource.Current); // 电流
    log_d("--------------------|------------");
    log_d(" Depth Sensor Type  |  %s",     Depth_Sensor_Name[Sensor.DepthSensor.Type]); // 深度传感器类型
    log_d(" Water Temperature  |  %0.3f",  Sensor.DepthSensor.Temperature);          // 水温
    log_d("sensor_Init_Pressure|  %0.3f",  Sensor.DepthSensor.Init_PessureValue);    // 深度传感器初始压力值
    log_d("   sensor_Pressure  |  %0.3f",  Sensor.DepthSensor.PessureValue);         // 深度传感器当前压力值
    log_d("        Depth       |  %0.3f",  Sensor.DepthSensor.Depth);                // 深度值
    log_d("--------------------|------------");
    log_d("   CPU.Temperature  |  %0.3f",  Sensor.CPU.Temperature); // CPU温度
    log_d("      CPU.Usages    |  %0.3f",  Sensor.CPU.Usage);       // CPU使用率
}
