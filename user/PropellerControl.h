/*
 * @Description: 推进器控制（未完成）
 * @Author: chenxi
 * @Date: 2020-02-19 13:13:56
 * @LastEditTime: 2020-03-18 19:32:15
 * @LastEditors: chenxi
 */

#ifndef __PROPELLERCONTROL_H
#define __PROPELLERCONTROL_H

#include "DataType.h"
#include "../applications/data.h"

void AUV_Depth_Control(Rocker_Type *rc);
void ROV_Depth_Control(Rocker_Type *rc);
void ROV_Rotate_Control(Rocker_Type *rc);

void robot_upDown(float *depth_output);
void Horizontal_Propeller_Power_Clear(void); //水平方向推力清零 未实现
void Propeller_Stop(void);                    //推进器停转

void robotForward(void);  //前进 未实现
void robotBackAway(void); //后退 未实现

void turnRight(uint16 power); //右转
void turnLeft(uint16 power);  //左转

void Propeller_Output(void); /* 推进器的真实输出 */

extern float Expect_Depth;
extern uint8 VehicleMode; //【FOUR_AXIS】 or 【SIX_AXIS】
extern uint8 WorkMode;     //【WORK】 or 【DEBUG】

#endif
