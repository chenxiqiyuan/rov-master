

#ifndef __SENSOR_H_
#define __SENSOR_H_

#include "../user/DataType.h"

#define DEPTH_SENSOR_PIN_BASE 100


/* 深度传感器 枚举类型 */
enum depthSensor_type 
{
    NO_DEPTH_SENSOR, // 0 : no depthSensor
    MS5837_TYPE ,    // 1 : ms5837
    SPL1301_TYPE,    // 2 : spl1301
};

/* 深度传感器设备描述符 */
typedef struct
{
    char    *name;        // 深度传感器名称
    float   depth;        // 实际深度
    float   pressure;     // 实时压力值
    float   init_pressure;// 初始化压力值

}depthSensor_t;






int sensor_thread_init(void);
void print_sensor_info(void); // 打印传感器信息










extern char *Depth_Sensor_Name[3];
extern Sensor_Type Sensor; //传感器参数

#endif
