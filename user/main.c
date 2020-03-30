/*
 * @Description: 主程序
 * @Author: chenxi
 * @Date: 2020-01-01 13:06:46
 * @LastEditTime : 2020-02-16 20:36:58
 * @LastEditors: chenxi
 */

#define LOG_TAG "main"


#include "init.h"
#include "DataType.h"

#include <elog.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <wiringPi.h>


/*
static void signal_handler(int sig)
{
	// ctrl + c程序中断回调
	printf("\n interrupt Key\n");

	// TODO 退出后清除 mjpg-streamer

	exit(0);
}*/

// TODO 是否需要使用fork函数
/*
void *mjpg_streamer_thread(void *arg)
{

	system("sudo chmod +x video.sh"); // 使 video.sh 脚本可执行

	system("./video.sh &"); // 打开视频推流脚本
	return NULL;
}
*/
int main(int argc, char **argv)
{
	pthread_t mjpg_tid;
	
	system_init();

	/* register signal handler for <CTRL>+C in order to clean up */
	/*if(signal(SIGINT, signal_handler) == SIG_ERR) 
	{
		printf("123\n");
	}*/

	//pthread_create(&mjpg_tid, NULL, mjpg_streamer_thread, NULL);
	//pthread_detach(mjpg_tid);


	while (1)
	{
		//oled_show();
		sleep(1);
		//delay(10);
	}
	return 0;
}
