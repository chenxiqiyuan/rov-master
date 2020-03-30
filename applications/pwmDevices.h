/*
 * @Description: 推进器设备
 * @Author: chenxi
 * @Date: 2020-03-01 22:36:16
 * @LastEditTime: 2020-03-17 23:25:15
 * @LastEditors: chenxi
 */

#ifndef __PROPELLER_H
#define __PROPELLER_H

#include "../user/DataType.h"

typedef struct //推进器中值、最大值、最小值
{
	int16_t PowerMax;
	int16_t PowerMed;
	int16_t PowerMin;
	int16_t PowerDeadband; //死区值
} PropellerParameter_Type; //推进器参数结构体

typedef struct //推进器方向
{
	int leftUp; //左上
	int rightUp;
	int leftDown; //左下
	int rightDown;
	int leftMiddle; //左中
	int rightMiddle;
} PropellerDir_Type;

typedef struct //调试时各个推进器的偏差量
{
	int16_t leftUp;
	int16_t rightUp;
	int16_t leftDown;
	int16_t rightDown;
	int16_t leftMiddle;
	int16_t rightMiddle;
} PropellerError_Type; //推进器偏移值

typedef struct
{
	int16_t Adjust1;
	int16_t Adjust2;
	int16_t Adjust3;
	int16_t Adjust4;
} Adjust_Parameter; //调节参数

typedef struct
{
	float PowerPercent; //推进器动力百分比
	
	short leftUp;
	short rightUp;
	short leftDown;
	short rightDown;
	short leftMiddle;
	short rightMiddle;
} PropellerPower_Type; //各个推进器推力

typedef enum
{
	RiseUp = 1, //上升
	Dive = 2,   //下潜
	Stop = 0
} ActionType_Enum; //动作指令枚举

void Propeller_Init(void);

void PWM_Update(PropellerPower_Type *power); //推进器PWM 更新
void Extractor_Control(uint8_t *action);

extern PropellerDir_Type PropellerDir;
extern PropellerParameter_Type PropellerParameter;
extern PropellerPower_Type PropellerPower; //推进器推理控制器
extern PropellerError_Type PropellerError; //推进器偏差值

extern PropellerError_Type Forward;
extern PropellerError_Type Retreat;
extern PropellerError_Type TurnLeft;
extern PropellerError_Type TurnRight;
extern Adjust_Parameter AdjustParameter;

extern int16_t PowerPercent;
extern PropellerPower_Type PropellerPower;



void Search_Light_Control(uint8_t *action);




typedef struct
{
    short MaxValue;     //舵机 正向最大值
    short MinValue;     //舵机 反向
    short MedValue;     //舵机 中值
    short CurrentValue; //舵机当前值
    uint8_t Speed;        //舵机速度值
} ServoType;

void RoboticArm_Control(uint8_t *action);
void YunTai_Control(uint8_t *action);
void DirectionProportion(int Mode);

extern ServoType RoboticArm; //机械臂
extern ServoType YunTai; //云台
extern int DirectionMode;

#endif
