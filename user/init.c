/*
 * @Description: 初始化程序
 */
#define LOG_TAG "init"
#include "../drivers/cpu_status.h"
#include "../drivers/pca9685.h"
#include "../drivers/jy901.h"
#include "../drivers/oled.h"
#include "../applications/data.h"
#include "../applications/sensor.h"
#include "../applications/server.h"
#include "../applications/ioDevices.h"

#include "DataType.h"

#include <elog.h>
#include <stdio.h>
#include <string.h>

#include <wiringPi.h>

void easylogger_init(void)
{
  // 初始化 EasyLogger
  elog_init();
  /* 设置 EasyLogger 日志格式 */
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);

  elog_set_text_color_enabled(true);
  elog_start();
}



int system_init(void)
{
	easylogger_init();
	if (wiringPiSetup() < 0)
	{
		log_e("Unable to start wiringPi: %s", strerror(errno));
		return -1;
	}

	sensor_thread_init(); // 传感器线程 初始化

	server_thread_init(); // 服务器线程 初始化

	ioDevs_thread_init(); // IO设备线程 初始化

	system_status_thread_init(); // 获取系统状态线程 初始化

	oledSetup();




	return 0;
}