/*
 * @Description: 主程序
 * @Author: chenxi
 * @Date: 2020-01-01 13:06:46
 * @LastEditTime : 2020-02-16 20:36:58
 * @LastEditors: chenxi
 */

#define LOG_TAG "main"

#include <elog.h>


#include "init.h"
#include "DataType.h"

#include <stdio.h>
#include <signal.h>
#include <wiringPi.h>



static void signal_handler(int sig)
{
	printf("\n interrupt Key\n");

	exit(0);
}

int main(int argc, char **argv)
{
	system_init();


	/* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets) */
	signal(SIGPIPE, SIG_IGN);

	/* register signal handler for <CTRL>+C in order to clean up */
	if(signal(SIGINT, signal_handler) == SIG_ERR) 
	{
		printf("123\n");
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
