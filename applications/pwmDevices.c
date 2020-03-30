/*
 * @Description: 推进器设备
 * @Author: chenxi
 * @Date: 2020-03-01 22:36:16
 * @LastEditTime: 2020-03-18 19:22:50
 * @LastEditors: chenxi
 */

#define LOG_TAG "propeller"

#include <elog.h>
#include "../drivers/pca9685.h"

#include "pwmDevices.h"
#include "data.h"

#include <stdlib.h>
#include <string.h>

#include <wiringPi.h>

uint8_t Propeller_Init_Flag = 0;

PropellerParameter_Type PropellerParameter = {
	//初始化推进器参数值【限幅】
	.PowerMax = 429, //正向最大值
	.PowerMed = 287, //中值
	.PowerMin = 172, //反向最小值【反向推力最大】

	.PowerDeadband = 10 //死区值
};

PropellerDir_Type PropellerDir = {1, 1, 1, 1, 1, 1};		//推进器方向，默认为1
PropellerPower_Type PropellerPower = {0, 0, 0, 0, 0, 0, 0}; //推进器推力控制器
PropellerError_Type PropellerError = {0, 0, 0, 0, 0, 0};	//推进器偏差值

PropellerPower_Type power_test; //调试用的变量

PropellerError_Type Forward = {0, 0, 0, 0, 0, 0};
PropellerError_Type Retreat = {0, 0, 0, 0, 0, 0};
PropellerError_Type TurnLeft = {0, 0, 0, 0, 0, 0};
PropellerError_Type TurnRight = {0, 0, 0, 0, 0, 0};
Adjust_Parameter AdjustParameter = {1, 1, 1, 1};

int16_t PowerPercent;

/*******************************************
* 函 数 名：Propeller_Init
* 功    能：推进器的初始化
* 输入参数：none
* 返 回 值：none
* 注    意：初始化流程：
*           1,接线,上电，哔-哔-哔三声,表示开机正常
*           2,给电调2ms或1ms最高转速信号,哔一声
*           3,给电调1.5ms停转信号,哔一声
*           4,初始化完成，可以开始控制
********************************************/
void Propeller_Init(void) //这边都需要经过限幅在给定  原先为2000->1500
{
	// 初始化推进器
	//pca9685PWMWrite(0, 0, PropellerPower_Max); //最高转速信号  水平推进器1号  右上
	//pca9685PWMWrite(5, 0, PropellerPower_Max); //最高转速信号  水平推进器2号  左下
	//pca9685PWMWrite(3, 0, PropellerPower_Max); //最高转速信号  水平推进器3号  左上
	//pca9685PWMWrite(2, 0, PropellerPower_Max); //最高转速信号  水平推进器4号  右下

	//pca9685PWMWrite(4, 0, PropellerPower_Max); //最高转速信号  垂直推进器1号  左中
	//pca9685PWMWrite(1, 0, PropellerPower_Max); //最高转速信号  垂直推进器2号  右中

	//pca9685PWMWrite(10, 0, PropellerPower_Max); //机械臂

	delay(2000); //2s

	//pca9685PWMWrite(0, 0, PropellerPower_Med); //停转信号
	//pca9685PWMWrite(5, 0, PropellerPower_Med); //停转信号
	//pca9685PWMWrite(3, 0, PropellerPower_Med); //停转信号
	//pca9685PWMWrite(2, 0, PropellerPower_Med); //停转信号

	//pca9685PWMWrite(4, 0, PropellerPower_Med); //停转信号
	//pca9685PWMWrite(1, 0, PropellerPower_Med); //停转信号

	//pca9685PWMWrite(10, 0, PropellerPower_Med); //机械臂

	// TODO 云台怎么初始化
	// TIM4_PWM_CH3_D14(1500); //机械臂中值 1000~2000
	// TIM4_PWM_CH4_D15(2000); //云台中值

	delay(1000);

	Propeller_Init_Flag = 1;
	log_i("Propeller_init()");
}

void PWM_Update(PropellerPower_Type *propeller)
{
	power_test.rightUp = PropellerPower_Med + propeller->rightUp;
	power_test.leftDown = PropellerPower_Med + propeller->leftDown;
	power_test.leftUp = PropellerPower_Med + propeller->leftUp;
	power_test.rightDown = PropellerPower_Med + propeller->rightDown;

	power_test.leftMiddle = PropellerPower_Med + propeller->leftMiddle;
	power_test.rightMiddle = PropellerPower_Med + propeller->rightMiddle;

	if (1 == Propeller_Init_Flag)
	{

		//pca9685PWMWrite(0, 0, power_test.rightUp);   // 水平推进器1号  右上
		//pca9685PWMWrite(5, 0, power_test.leftDown);  // 水平推进器2号  左下
		//pca9685PWMWrite(3, 0, power_test.leftUp);	// 水平推进器3号  左上
		//pca9685PWMWrite(2, 0, power_test.rightDown); // 水平推进器4号  右下

		//pca9685PWMWrite(4, 0, power_test.leftMiddle);  // 垂直推进器1号  左中
		//pca9685PWMWrite(1, 0, power_test.rightMiddle); // 垂直推进器2号  右中
	}
}




short light_value = 0;

/*******************************************
* 函 数 名：Light_Output_Limit
* 功    能：灯光亮度输出限制
* 输入参数：灯光亮度值 0~90%
* 返 回 值：None
* 注    意：
********************************************/
int Light_Output_Limit(short *value)
{
    *value = *value >= 90 ? 90 : *value; //限幅
    *value = *value <= 0 ? 0 : *value;   //限幅
    return *value;
}

/**
  * @brief  Light_Control(探照灯控制)
  * @param  控制指令 0x00：不动作  0x01：向上  0x02：向下
  * @retval None
  * @notice 
  */
void Search_Light_Control(uint8_t *action)
{
    switch (*action)
    {
    case 0x01:
        light_value += 5; //变亮
        break;
    case 0x02:
        light_value -= 5; //变暗
        break;
    default:
        break;
    }
    Light_Output_Limit(&light_value);
    //pca9685PWMWrite(8, 0, light_value);
    //pca9685PWMWrite(9, 0, light_value);
    *action = 0x00; //清除控制字
}




/*---------------------- Constant / Macro Definitions -----------------------*/

#define RoboticArm_MedValue 1500
#define YunTai_MedValue 2000

/*----------------------- Variable Declarations -----------------------------*/

ServoType RoboticArm = {
    .MaxValue = 2000, //机械臂 正向最大值
    .MinValue = 1000, //机械臂 反向
    .MedValue = 1500,
    .Speed = 5 //机械臂当前值
};             //机械臂
ServoType YunTai = {
    .MaxValue = 2500, //机械臂 正向最大值
    .MinValue = 1500, //机械臂 反向
    .MedValue = 2000,
    .Speed = 10 //云台转动速度
};              //云台

uint16_t propeller_power = 1500;
short _test_value = 0;

/*----------------------- Function Implement --------------------------------*/

/*******************************************
* 函 数 名：Servo_Output_Limit
* 功    能：舵机输出限制
* 输入参数：输入值：舵机结构体地址 
* 返 回 值：None
* 注    意：
********************************************/
void Servo_Output_Limit(ServoType *Servo)
{
    Servo->CurrentValue = Servo->CurrentValue > Servo->MaxValue ? Servo->MaxValue : Servo->CurrentValue; //正向限幅
    Servo->CurrentValue = Servo->CurrentValue < Servo->MinValue ? Servo->MinValue : Servo->CurrentValue; //反向限幅
}

/**
  * @brief  RoboticArm_Control(机械臂控制)
  * @param  控制指令 0x00：不动作  0x01：张开  0x02：关闭
  * @retval None
  * @notice 
  */
void RoboticArm_Control(uint8_t *action)
{
    static uint8_t on_off = 0; //自锁开关
    if (0 == on_off)
    {
        on_off = 1;
        RoboticArm.CurrentValue = RoboticArm.MedValue;
    }

    switch (*action)
    {
    case 0x01:
        RoboticArm.CurrentValue += RoboticArm.Speed;
        if (RoboticArm.CurrentValue >= RoboticArm.MaxValue)
        {
            device_hint_flag |= 0x01;
        } //机械臂到头标志
        else
        {
            device_hint_flag &= 0xFE;
        }; //清除机械臂到头标志

        break;
    case 0x02:
        RoboticArm.CurrentValue -= RoboticArm.Speed;
        if (RoboticArm.CurrentValue <= RoboticArm.MinValue)
        {
            device_hint_flag |= 0x01;
        } //机械臂到头标志
        else
        {
            device_hint_flag &= 0xFE;
        }; //清除机械臂到头标志

        break;
    default:
        break;
    }
    Servo_Output_Limit(&RoboticArm); //机械臂舵机限幅
    //pca9685PWMWrite(10, 0, RoboticArm.CurrentValue);
    *action = 0x00; //清除控制字
}

/**
  * @brief  YunTai_Control(云台控制)
  * @param  控制指令 0x00：不动作  0x01：向上  0x02：向下
  * @retval None
  * @notice 
  */

void YunTai_Control(uint8_t *action)
{
    static uint8_t on_off = 0; //自锁开关
    if (0 == on_off)
    {
        on_off = 1;
        YunTai.CurrentValue = YunTai.MedValue;
    }
    switch (*action)
    {
    case 0x01:
        YunTai.CurrentValue -= YunTai.Speed; //向上
        if (YunTai.CurrentValue <= YunTai.MaxValue)
        {
            device_hint_flag |= 0x02;
        } //云台到头标志
        else
        {
            device_hint_flag &= 0xFD;
        }; //清除云台到头标志
        break;

    case 0x02:
        YunTai.CurrentValue += YunTai.Speed; //向下
        if (YunTai.CurrentValue >= YunTai.MinValue)
        {
            device_hint_flag |= 0x02;
        } //云台到头标志
        else
        {
            device_hint_flag &= 0xFD;
        }; //清除云台到头标志
        break;

    case 0x03:
        YunTai.CurrentValue = YunTai.MedValue;
        break; //归中

    default:
        break;
    }
    Servo_Output_Limit(&YunTai);
    // TODO 云台1还是云台2
    //pca9685PWMWrite(6, 0, YunTai.CurrentValue);
    *action = 0x00; //清除控制字
}

