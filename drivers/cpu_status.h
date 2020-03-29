/*
 * @Description: 获取 CPU、内存、硬盘 状况
 */

#ifndef __CPU_STATUS_H_
#define __CPU_STATUS_H_

#include "../user/DataType.h"

typedef struct // 定义一个cpu occupy的结构体
{
    char name[20];       // 
    unsigned int user;   // 在用户模式下花费的时间
    unsigned int nice;   // 在低优先级用户模式下花费的时间
    unsigned int system; // 在系统模式下花费的时间
    unsigned int idle;   // 等待任务花费的时间
} CPU_OCCUPY;

/* 内存使用状况 描述符 */
typedef struct 
{
    uint32_t total;     // 内存总数
    uint32_t available; // 可用内存数 MemAvailable≈MemFree+Buffers+Cached
    float    usage_rate;// 使用率

} memory_t;

memory_t *get_memory_status(void); // 获取 内存情况

float get_cpu_temp(void);            // 获取 CPU 温度


float get_cpu_usage(void);           // 获取 5s 内的 cpu 使用率

#endif /* _DRV_CPU_TEMP_H_ */
