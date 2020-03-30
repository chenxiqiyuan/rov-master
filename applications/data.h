
#ifndef __RC_DATA_H_
#define __RC_DATA_H_

#include "../user/DataType.h"

/*
 * 接收数据包长度【包含包头、长度位、校验位】
 * 包头：2 数据长度位: 1 包体：16 校验字：1
*/
#define RECV_DATA_LEN  20 
            
/*
 * 返回数据包长度【包含包头、长度位、校验位】
 * 包头：1 数据长度位: 1 包体：22 校验字：1
*/
#define RETURN_DATA_LEN 26


typedef struct
{
    int16_t X; //摇杆 X、Y轴模拟量 （X正方向为机头方向）
    int16_t Y;
    int16_t Z;     //Z为垂直方向
    int16_t Fx;    //转换的 X轴分力
    int16_t Fy;    //转换的 Y轴分力
    int16_t Yaw;   //偏航
    int16_t Force; //合力
    float Angle; //合力的角度
} Rocker_Type;

typedef struct
{
    uint16_t THR;
    uint16_t YAW;
    uint16_t ROL;
    uint16_t PIT;
} ReceiveData_Type;

typedef struct
{
    /* 锁定类 */
    uint8_t all_lock;   //总开关
    uint8_t depth_lock; //【深度】	 0x01 深度锁定、0x02 人工控制  depth_lock
    uint8_t sport_lock; //【运动】	 0x01 运动锁定、0x02 随水动    sport_lock

    /* 运动类 */
    uint8_t move_back;  //【运动】	 
    uint8_t left_right; //【平移】	 
    uint8_t up_down;    //【垂直】	 
    uint8_t rotate;     //【旋转】	 

    /* 设备类 */
    uint16_t power; //【油门】	0x00~0xff (0~255) 表示的油门大小：4档位可调，LB加档，LT减档，可分别设置4个档位油门大小
    uint8_t light;  //【灯光】	0x01 打开、0x00 关闭（默认）
    uint8_t camera; //【聚焦】  0x01 聚焦、0x02 放焦、  0x11 放大、0x12 缩小、0x00表示不动作（默认）
    uint8_t yuntai; //【云台】	0x01 向上、0x02 向下、  0x00表示不动作（默认）
    uint8_t arm;    //【机械臂】0x01 张开、0x02 关闭、  0x00表示不动作（默认）

} cmd_t;

uint8_t calculate_check_sum(uint8_t *buff, uint8_t len);

void remote_control_data_analysis(uint8_t *recv_buff); //控制数据解析

void convert_rov_status_data(uint8_t *buff); // 转换需要返回上位机数据



extern Rocker_Type Rocker;







int system_status_thread_init(void);







#endif

