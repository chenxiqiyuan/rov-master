

#define LOG_TAG "rc_data"

#include <elog.h>
#include "data.h"
#include "sensor.h"

ReceiveData_Type ReceiveData = {
    .THR = 1500,
    .YAW = 1500,
    .ROL = 1500,
    .PIT = 1500};

ControlCmd_Type ControlCmd = {
    //上位机控制数据
    .Power = 0,
    .All_Lock = LOCK //初始化定义为锁定状态
};

Rocker_Type Rocker; // 摇杆数据结构体

uint8 RC_Control_Data[RECV_DATA_LEN] = {0};
uint8 Frame_EndFlag = 0; //接收数据包结束标志 
uint8 Control_RxCheck = 0; //尾校验字
uint8 recv_buff[RECV_DATA_LEN] = {0};

uint8 device_hint_flag = 0x0; //设备提示字符


/**
  * @brief  get_decimal(得到浮点型数据头两位小数的100倍)
  * @param  浮点型数据 data
  * @retval 头两位小数的100倍
  * @notice 
  */
uint8 get_decimal(float data)
{
    return (uint8)((float)(data - (int)data) * 100);
}

/**
  * @brief  计算校验和
  * @param  数据包*buff、数据包长度len
  * @retval SUM
  * @notice 
  */
uint8 calculate_check_sum(uint8 *buff, uint8 len)
{
    uint8 sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += buff[i];
    }
    return sum;
}


/**
  * @brief  上位机控制数据解析
  * @param  控制字符数据 *recv_buff
  * @retval None
  * @notice 从第四个字节开始为控制字符
  */
void remote_control_data_analysis(uint8 *recv_buff) //控制数据解析
{
    if (recv_buff[0] == 0xAA && recv_buff[1] == 0x55 && recv_buff[2] == 0x10) //检测包头
    {
        // 获取校验位
        Control_RxCheck = calculate_check_sum(recv_buff, RECV_DATA_LEN - 1);

        if (Control_RxCheck == recv_buff[RECV_DATA_LEN]) // 校验位核对
        {
            ControlCmd.Depth_Lock = RC_Control_Data[3];     //深度锁定
            ControlCmd.Direction_Lock = RC_Control_Data[4]; //方向锁定

            ControlCmd.Move = RC_Control_Data[5];        //前后
            ControlCmd.Translation = RC_Control_Data[6]; //左右平移
            ControlCmd.Vertical = RC_Control_Data[7];    //垂直
            ControlCmd.Rotate = RC_Control_Data[8];      //旋转

            ControlCmd.Power = RC_Control_Data[9];   //动力控制  推进器动力系数
            ControlCmd.Light = RC_Control_Data[10];  //灯光控制
            ControlCmd.Focus = RC_Control_Data[11];  //变焦摄像头控制
            ControlCmd.Yuntai = RC_Control_Data[12]; //云台控制
            ControlCmd.Arm = RC_Control_Data[13];    //机械臂控制
            // ControlCmd.Raspi = RC_Control_Data[14];  //树莓派启动位
            ControlCmd.All_Lock = RC_Control_Data[18];
        }
    }
}

/* 
【注意】这里仅清空控制数据指令，不能清除控制状态指令，因此，不能采用 meset 直接填充结构体为 0 
*/
void Control_Cmd_Clear(ControlCmd_Type *cmd)
{
    cmd->Move = 0;        //前后
    cmd->Translation = 0; //左右平移
    cmd->Vertical = 0;    //垂直
    cmd->Rotate = 0;      //旋转

    cmd->Light = 0;  //灯光控制
    cmd->Focus = 0;  //变焦摄像头控制
    cmd->Yuntai = 0; //云台控制
    cmd->Arm = 0;    //机械臂控制
}


/**
  * @brief  calculate_check_sum(计算校验和)
  * @param  数据包*buff、数据包长度len
  * @retval SUM
  * @notice 
  */
void convert_rov_status_data(uint8 *buff) // 转换需要返回上位机数据
{
    short troll; //暂存数据
    short tpitch;
    short tyaw;
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
    buff[13] = (uint8)tyaw; //低8位

    buff[14] = tpitch >> 8;   // Pitch 高8位
    buff[15] = (uint8)tpitch; //低8位

    buff[16] = troll >> 8;   // Roll 高8位
    buff[17] = (uint8)troll; //低8位

    buff[18] = (uint8)speed_test++;        //x轴航速
    buff[19] = device_hint_flag; //设备提示字符

    buff[20] = 0x01; // ControlCmd.All_Lock;

    buff[21] = (int)Sensor.PowerSource.Current;
    buff[22] = get_decimal(Sensor.PowerSource.Current); //小数的100倍;

    buff[23] = 0x0; // 保留
    buff[24] = 0x0; // 保留

    buff[25] = calculate_check_sum(buff, RETURN_DATA_LEN - 1);//获取校验和
}