/*
 * @Description: ROV状态数据回传与控制命令接收解析，获取 系统状态(CPU、内存、硬盘、网卡网速)
 */

#define LOG_TAG "data"

#include "../drivers/oled.h"
#include "../drivers/cpu_status.h"

#include "data.h"
#include "sensor.h"
#include "server.h"

#include <elog.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


Rocker_Type Rocker; // 摇杆数据结构体


cmd_t cmd_data;
cmd_t *cmd = &cmd_data;

system_status_t  system_dev;
system_status_t  *system = &system_dev;

/**
  * @brief  获取浮点型数据 头两位小数的100倍
  * @param  float data
  * @retval 头两位小数的100倍
  */
uint8_t get_decimal(float data)
{
    return (uint8_t) ((data - (int)data) * 100);
}

/**
  * @brief  计算校验和
  * @param  数据包 *buff、数据包长度len
  * @retval 累加和 SUM
  */
uint8_t calculate_check_sum(uint8_t *buff, uint8_t len)
{
    uint8_t sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += buff[i];
    }
    return sum;
}


/**
  * @brief  上位机控制数据解析
  * @param  控制数据包 *buff
  * @notice 从第四个字节开始为控制字符
  */
void remote_control_data_analysis(uint8_t *buff) //控制数据解析
{
    uint8_t rxCheck = 0; //尾校验字

    if (buff[0] == 0xAA && buff[1] == 0x55) // 检测包头
    {
        if(buff[2] == RECV_DATA_LEN) // 检测数据包长度(此判断暂无作用，用于后续 可变长度数据包)
        {
            // 获取校验位
            rxCheck = calculate_check_sum(buff, RECV_DATA_LEN - 1);


            if (rxCheck == buff[RECV_DATA_LEN]) // 校验位核对
            {
                cmd->depth_lock = buff[3];  // 深度锁定
                cmd->sport_lock = buff[4];  // 方向锁定

                cmd->move_back  = buff[5];  // 前后
                cmd->left_right = buff[6];  // 左右平移
                cmd->up_down    = buff[7];  // 垂直
                cmd->rotate     = buff[8];  // 旋转

                cmd->power      = buff[9];  // 动力控制  推进器动力系数
                cmd->light      = buff[10]; // 灯光控制
                cmd->camera     = buff[11]; // 变焦摄像头控制
                cmd->yuntai     = buff[12]; // 云台控制
                cmd->arm        = buff[13]; // 机械臂控制

                cmd->all_lock = buff[18];   // 总开关
            }
        }
    }
}

/* 
 * TODO 控制命令清零 【注意】这里仅清空控制数据指令，不能清除控制状态指令，因此，不能采用 meset 直接填充结构体为 0 
 */


/**
  * @brief  calculate_check_sum(计算校验和)
  * @param  数据包*buff、数据包长度len
  * @retval SUM
  */
void convert_rov_status_data(uint8_t *buff) // 转换需要返回上位机数据
{
    uint16_t troll; //暂存数据
    uint16_t tpitch;
    uint16_t tyaw;
    static unsigned char speed_test;

    troll = (short)((Sensor.JY901.Euler.Roll + 180) * 100); //数据转换:将角度数据转为正值并放大100倍
    tpitch = (short)((Sensor.JY901.Euler.Pitch + 180) * 100);
    tyaw = (short)((Sensor.JY901.Euler.Yaw + 180) * 100);

    buff[3] = (int)Sensor.PowerSource.Voltage;         //整数倍
    buff[4] = get_decimal(Sensor.PowerSource.Voltage); //小数的100倍

    buff[5] = (int)Sensor.CPU.Temperature;         //整数倍
    buff[6] = get_decimal(Sensor.CPU.Temperature); //小数的100倍

    buff[7] = (int)Sensor.DepthSensor.Temperature;         //整数倍
    buff[8] = get_decimal(Sensor.DepthSensor.Temperature); //小数的100倍

    buff[9] = (int)(Sensor.DepthSensor.Depth) >> 16; //高8位
    buff[10] = (int)(Sensor.DepthSensor.Depth) >> 8;  //中8位
    buff[11] = (int)(Sensor.DepthSensor.Depth);       //低8位

    buff[12] = tyaw >> 8;    // Yaw 高8位
    buff[13] = (uint8_t)tyaw; //低8位

    buff[14] = tpitch >> 8;   // Pitch 高8位
    buff[15] = (uint8_t)tpitch; //低8位

    buff[16] = troll >> 8;   // Roll 高8位
    buff[17] = (uint8_t)troll; //低8位

    buff[18] = (uint8_t)speed_test++;        //x轴航速
    buff[19] = 0; //设备提示字符

    buff[20] = 0x01; // cmd->All_Lock;

    buff[21] = (int)Sensor.PowerSource.Current;
    buff[22] = get_decimal(Sensor.PowerSource.Current); //小数的100倍;

    buff[23] = 0x0; // 保留
    buff[24] = 0x0; // 保留

    buff[25] = calculate_check_sum(buff, RETURN_DATA_LEN - 1);//获取校验和
}




void oled_show_status(void)
{
    char str[20];
	sprintf(str,"IP  %s", system->net.ip);
	OLED_ShowString(0, 0, (uint8_t *)str, 12);

	sprintf(str,"Mem: %0.1f%% of %d Mb", system->memory.usage_rate, system->memory.total / 1024);
	OLED_ShowString(0,  16, (uint8_t *)str, 12);

	sprintf(str,"Disk: %0.1f%% of %0.1f G", system->disk.usage_rate, (float)system->disk.total / 1024);
	OLED_ShowString(0,  32, (uint8_t *)str, 12);

	sprintf(str,"CPU: %0.1f%%", system->cpu.usage_rate);
	OLED_ShowString(0,  48, (uint8_t *)str, 12);

    if(system->net.netspeed < 1024) 
    {
        // 此时单位为 kbps
        sprintf(str,"%0.1f kb/s", system->net.netspeed);
        OLED_ShowString(70,  48, (uint8_t *)str, 12);
    }
    else 
    {
        // 转换单位为 Mbps
        sprintf(str,"%0.1f Mb/s", system->net.netspeed / 1024);
        OLED_ShowString(70,  48, (uint8_t *)str, 12);
    }

}


/**
 * @brief  获取CPU状态 线程
 *  get_cpu_usage函数内已经休眠1s，因此不再设置休眠  
 */
void *cpu_status_thread(void *arg)
{
    while(1)
    {
        system->cpu.temperature = get_cpu_temp();
        system->cpu.usage_rate  = get_cpu_usage();
        oled_show_status();
    }
}

/**
 * @brief  获取网速 线程
 *  get_net_speed函数内已经休眠1s，因此不再设置休眠  
 */
void *net_speed_thread(void *arg)
{
    system->net.name = "eth0"; // 指定 eht0 网卡
    // 获取ip地址
    get_localip(system->net.name, system->net.ip);
    while(1)
    {
        system->net.netspeed = get_net_speed(system->net.name);
    }  
}

/**
 * @brief  获取内存、硬盘状态 线程
 */
void *mem_disk_status_thread(void *arg)
{
    while(1)
    {
        // 获取内存使用情况
        get_memory_status(&system->memory);
        // 获取硬盘使用情况
        get_disk_status(&system->disk);   
        // 1s更新一次
        sleep(1); 
    }
}

/**
  * @brief  系统状态获取 线程初始化
  *  CPU、网卡网速 状态都是通过休眠一段时间，两次数据对比进行测算的，因此都需要各开一个线程
  *  内存、硬盘 状态共用1个即可
  */

  
int system_status_thread_init(void)
{
    pthread_t cpu_tid;
    pthread_t net_tid;  
    pthread_t mem_disk_tid;

    pthread_create(&cpu_tid, NULL, cpu_status_thread, NULL);
    pthread_detach(cpu_tid);

    pthread_create(&net_tid, NULL, net_speed_thread, NULL);
    pthread_detach(net_tid);    

    // 内存、硬盘 状态获取共用1个线程
    pthread_create(&mem_disk_tid, NULL, mem_disk_status_thread, NULL);
    pthread_detach(mem_disk_tid);

    return 0;
}


