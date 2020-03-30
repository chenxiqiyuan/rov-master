
#ifndef __RC_DATA_H_
#define __RC_DATA_H_

#include "../user/DataType.h"

/*
 * 数据包长度【包含包头、长度位、校验位】
 * 包头：3 包体：16 校验字：1
*/
#define RECV_DATA_LEN  20 
            
/*
 * 数据包长度【包含包头、长度位、校验位】
 * 包头：3 包体：22 校验字：1
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
    uint8_t All_Lock;       //总开关
    uint8_t Depth_Lock;     //【深度】	 0x01 深度锁定、0x02 人工控制
    uint8_t Direction_Lock; //【方向】	 0x01 方向锁定、0x02 随水动
    uint8_t Move;           //【运动】	 0x01 前进、    0x02 后退、  0x00 不动作（默认）
    uint8_t Translation;    //【平移】	 0x01 左平移、  0x02 右平移、0x00 不动作（默认）
    uint8_t Vertical;       //【垂直】	 0x01 向上、    0x02 向下、  0x00 不动作（默认）
    uint8_t Rotate;         //【旋转】	 0x01 左旋、    0x02 右旋、  0x00表示不动作（默认）
    uint16_t Power;         //【油门】	 0x00~0xff (0~255) 表示的油门大小：4档位可调，LB加档，LT减档，可分别设置4个档位油门大小
    uint8_t Light;          //【灯光】	 0x01 打开、    0x00 关闭（默认）
    uint8_t Focus;          //【聚焦】  0x01 聚焦、		0x02 放焦、  0x11 放大、0x12 缩小、0x00表示不动作（默认）
    uint8_t Yuntai;         //【云台】	 0x01 向上、    0x02 向下、  0x00表示不动作（默认）
    uint8_t Arm;            //【机械臂】0x01 张开、    0x02 关闭、  0x00表示不动作（默认）
    uint8_t Raspi;          //【树莓派】0x01 代表树莓派开机，0x00为无开机响应（默认）
    uint8_t Reserve;        //【机械臂】0x01 张开、    0x02 关闭、  0x00表示不动作（默认）
} ControlCmd_Type;

void remote_control_data_analysis(uint8_t *recv_buff); //控制数据解析
void Control_Cmd_Clear(ControlCmd_Type *cmd);        //控制命令清零
uint8_t calculate_check_sum(uint8_t *buff, uint8_t len);
void convert_rov_status_data(uint8_t *buff); // 转换需要返回上位机数据

extern uint8_t device_hint_flag; //设备提示字符

extern Rocker_Type Rocker;
extern ControlCmd_Type ControlCmd;
extern ReceiveData_Type ReceiveData;
extern uint8_t recv_buff[RECV_DATA_LEN];
extern uint8_t Frame_EndFlag;



int system_status_thread_init(void);







#endif

