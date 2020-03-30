

#ifndef __SERVER_H_
#define __SERVER_H_

#include "../user/DataType.h"


#define LISTEN_PORT 8899
#define BACKLOG     10   //最大连接数


int server_thread_init(void);
void get_localip(const char *eth_name, char *ip);

#endif
