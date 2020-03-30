/*
 * @Description: 服务器线程
 * @Author: chenxi
 * @Date: 2020-02-15 19:57:26
 * @LastEditTime : 2020-02-15 20:17:25
 * @LastEditors: chenxi
 */

#ifndef __SERVER_H_
#define __SERVER_H_

#include "../user/DataType.h"


#define LISTEN_PORT 8888
#define BACKLOG     10   //最大连接数


int server_thread_init(void);
void get_localip(const char *eth_name, char *ip);

#endif
