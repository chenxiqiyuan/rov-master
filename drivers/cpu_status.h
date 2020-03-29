/*
 * @Description: 获取 CPU、内存、硬盘 状况
 */

#ifndef __CPU_STATUS_H_
#define __CPU_STATUS_H_

#include "../user/DataType.h"

typedef struct // 定义一个cpu occupy的结构体
{
    char name[8];      
    uint32_t user;   // 用户态时间 单位：jiffies (1 jiffies = 10ms)
    uint32_t nice;   // 用户态时间(低优先级，nice>0)
    uint32_t system; // 内核态时间
    uint32_t idle;   // 空闲时间
    uint32_t iowait; // I/O等待时间
    uint32_t irq;    // 硬中断
    uint32_t softirq;// 软中断
} cpuInfo_t;


/* 内存状况 描述符 */
typedef struct 
{
    uint32_t total;     // 内存总数
    uint32_t free;      // 完全未用到的物理内存
    uint32_t available; // 可用内存数 MemAvailable ≈ MemFree + Buffers(文件缓冲大小) + Cached(高速缓冲大小)
    float    usage_rate;// 使用率

} memory_t;


/* 硬盘状况 描述符 */
typedef struct
{
	uint32_t total;
    uint32_t available;
    float    usage_rate;
} disk_t;


/* 网络数据 描述符 */
typedef struct  
{
	uint64_t rp, rb;
	uint64_t tp, tb;
} netData_t;



float get_cpu_temp(void); // 获取 CPU 温度
float get_cpu_usage(void);// 获取 1s 内的 cpu 使用率

void get_disk_status(disk_t *disk);	
void get_memory_status(memory_t *memory); // 获取 内存情况

char *get_net_speed(char *eth);



#endif /* _DRV_CPU_TEMP_H_ */
