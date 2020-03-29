/*
 * @Description: 获取 CPU、内存、硬盘 状况
 */

#define LOG_TAG "cpu_status"

#include "cpu_status.h"

#include <elog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE 32



 /**
 * @brief  获取 cpu温度值
 * @return float cpu温度值
 */
float get_cpu_temp(void)
{
    int fd;
    float temp = 0;
    char buf[MAX_SIZE];
    // 打开/sys/class/thermal/thermal_zone0/temp
    fd = open(TEMP_PATH, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open thermal_zone0/temp\n");
        return -1;
    }
    // 读取内容
    if (read(fd, buf, MAX_SIZE) < 0)
    {
        fprintf(stderr, "failed to read temp\n");
        return -1;
    }
    // 转换为浮点数打印
    temp = atoi(buf) / 1000.0;
    // 关闭文件
    close(fd);

    return temp;
}

 /**
 * @brief  获取 内存情况
 * @return 内存使用状况 结构体指针
 */
memory_t *get_memory_status(void)
{
    static FILE *fd;
    static char buff[50];
    static char name1[20]; // 用于保存 内存名称(eg. total/available)
    static char name2[20]; // 用于保存 单位    (eg. kB)

    static memory_t memory_dev;
    static memory_t *memory = &memory_dev;

    fd = fopen("/proc/meminfo", "r");

    fgets(buff, sizeof(buff), fd); // 读取第1行 total
    sscanf(buff, "%s %u %s", name1, &memory->total, name2);
    // printf("%s %u %s\n", name1, memory->total, name2); // 用于调试打印

    fgets(buff, sizeof(buff), fd); // 跳过第2行 free

    fgets(buff, sizeof(buff), fd); // 读取第3行 available
    sscanf(buff, "%s %u %s", name1, &memory->available, name2);
    //printf("%s %u %s\n", name1, memory->available, name2);

    memory->usage_rate = (float)(memory->total - memory->available) / memory->total * 100.0f;
    // printf("mem usage_rate %f%% \n", memory->usage_rate);

    fclose(fd); // 关闭文件

    return memory;
}

// 获取 cpu 使用情况
void get_cpuoccupy(CPU_OCCUPY *cpust)
{
    FILE *fd;
    char buff[256];
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy = cpust;

    fd = fopen("/proc/stat", "r");
    fgets(buff, sizeof(buff), fd);

    sscanf(buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle);

    fclose(fd);
}

// 获取 5s 内的 cpu 使用率
float get_cpu_usage(void)
{
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;

    //第一次获取cpu使用情况
    get_cpuoccupy(&cpu_stat1);
    sleep(5);
    //第二次获取cpu使用情况
    get_cpuoccupy(&cpu_stat2);

    unsigned long od, nd;
    unsigned long id, sd;
    float cpu_use = 0;

    od = (unsigned long)(cpu_stat1.user + cpu_stat1.nice + cpu_stat1.system + cpu_stat1.idle); //第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long)(cpu_stat2.user + cpu_stat2.nice + cpu_stat2.system + cpu_stat2.idle); //第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (unsigned long)(cpu_stat2.user - cpu_stat1.user);     // 用户第一次和第二次的时间之差再赋给id
    sd = (unsigned long)(cpu_stat2.system - cpu_stat1.system); // 系统第一次和第二次的时间之差再赋给sd
    if ((nd - od) != 0)
        cpu_use = (float)((sd + id) * 100) / (nd - od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else
        cpu_use = 0;

    return cpu_use;
}
