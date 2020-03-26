/*
 * @Description: 运动总控制（未完成）
 * @Author: chenxi
 * @Date: 2020-02-19 13:13:56
 * @LastEditTime: 2020-03-18 19:32:46
 * @LastEditors: chenxi
 */

#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "DataType.h"
#include "../applications/data.h"

void Angle_Control(void);
void Depth_PID_Control(float expect_depth, float sensor_depth);
void FourtAxis_Control(Rocker_Type *rc);
void SixAxis_Control(Rocker_Type *rc);//未实现
void Convert_RockerValue(Rocker_Type *rc); //获取摇杆值
void Speed_Buffer(short *now_value, short *last_value, short BufferRange);

#endif
