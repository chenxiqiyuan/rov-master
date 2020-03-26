/*
 * @Description: 主程序
 * @Author: chenxi
 * @Date: 2020-01-01 13:06:46
 * @LastEditTime : 2020-02-16 20:36:58
 * @LastEditors: chenxi
 */

#define LOG_TAG "main"

#include <elog.h>
#include "../drivers/pca9685.h"
#include "../drivers/jy901.h"
#include "../applications/sensor.h"
#include "../applications/server.h"

#include "DataType.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>

#include <wiringPi.h>

void easyloggerInit(void)
{
  setbuf(stdout, NULL);
  /* initialize EasyLogger */
  elog_init();
  /* set EasyLogger log format */
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
#ifdef ELOG_COLOR_ENABLE
  elog_set_text_color_enabled(true);
#endif
  /* start EasyLogger */
  elog_start();
}

static void signal_handler(int sig)
{
  printf("interruptKey \n");

  exit(0);
}

int main(int argc, char **argv)
{
  easyloggerInit();
  if (wiringPiSetup() < 0)
  {
    log_e("Unable to start wiringPi: %s", strerror(errno));
    return 1;
  }

  sensor_thread_init(); //初始化传感器
  server_thread_init(); //初始化服务器

  openlog("ROV", LOG_PID | LOG_CONS, LOG_LOCAL0);
  //openlog("MJPG-streamer ", LOG_PID|LOG_CONS|LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "starting application");

  /* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets) */
  signal(SIGPIPE, SIG_IGN);

  /* register signal handler for <CTRL>+C in order to clean up */
  if(signal(SIGINT, signal_handler) == SIG_ERR) {
      syslog(LOG_INFO, "starting application");
      closelog();
  }
  // 使 video.sh 脚本可执行
  system("sudo chmod +x video.sh");
  // 打开视频推流脚本
  system("./video.sh");


  while (1)
  {
      pause();
  }
  return 0;
}
