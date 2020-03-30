/*
 * @Description: 数据链路服务器，用于传输与接收数据
 */

#define LOG_TAG "server"

#include <elog.h>
#include "server.h"
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <wiringPi.h>

static int sever_sock  = -1;
static int client_sock = -1;

/* 包头位固定为：0xAA,0x55;  数据长度位：0x16 */
static uint8_t return_data[RETURN_DATA_LEN] = {0xAA, 0x55, 0x16};

/**
  * @brief  获取对应网卡的IP地址
  * @param  eth_name:网卡名   ip:数组首地址
  */
void get_localip(const char *eth_name, char *ip)
{
    struct ifreq ifr;

	if (eth_name == NULL)
	{
		return;
	}

    strcpy(ifr.ifr_name, eth_name);
    if (!(ioctl(sever_sock, SIOCGIFADDR, &ifr)))
    {
        strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    }
}



void print_hex_data(const char *name, uint8_t *data, int len)
{
    printf("%s:", name);
    for(int i = 0; i < len; i++)
    {
        printf("%2x ", data[i]);
    }
    printf("\n");
}


/**
  * @brief  数据发送线程
  * @param  void *arg
  * @retval NULL
  * @notice 
  */
void *send_thread(void *arg)
{
    while (1)
    {
        /* 转换ROV状态数据 */
        convert_rov_status_data(return_data);
        if (write(client_sock, return_data, RETURN_DATA_LEN) < 0)
        {
            if (client_sock != -1)
            {
                log_i("IP [%s] client closed", arg);
                close(client_sock);
                client_sock = -1;
            }
            return NULL;
        }
        //print_hex_data("send", return_data, RETURN_DATA_LEN);
        sleep(1); // 1s更新一次
    }
    return NULL;
}

/**
  * @brief  数据接收线程
  * @param  void *arg
  * @retval NULL
  * @notice 
  */
void *recv_thread(void *arg)
{
    while (1)
    {
        if (recv(client_sock, recv_buff, RECV_DATA_LEN, 0) < 0)
        {
            if (client_sock != -1)
            {
                log_i("IP [%s] client closed", arg);
                close(client_sock);
                client_sock = -1;
            }
            return NULL;
        }
        //print_hex_data("recv", recv_buff, RECV_DATA_LEN);
        /* 遥控数据解析 */
        remote_control_data_analysis(recv_buff);
    }
    return NULL;
}

/**
  * @brief  数据服务器线程
  * @param  void *arg
  * @notice 该线程创建两个线程用于接收与发送数据
  */
void *server_thread(void *arg)
{
    static struct sockaddr_in serverAddr;
    static struct sockaddr_in clientAddr; // 用于保存客户端的地址信息
    static unsigned int addrLen;
	static unsigned int clientCnt; // 记录客户端连接的次数
    static int opt = 1;    // 套接字选项 Enable address reuse
    static char *clientip; // 保存客户端 IP 地址
    static char serverip[20]; // 保存本地 eth0 IP地址

    pthread_t send_tid;
    pthread_t recv_tid;
    
    /* 1.初始化服务器socket */
    if ((sever_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        log_e("create server socket error:%s(errno:%d)\n", strerror(errno), errno);
        exit(1);
    }

    // 设置套接字, SO_REUSERADDR 允许重用本地地址和端口，充许绑定已被使用的地址（或端口号）
    if (setsockopt(sever_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        log_e("setsockopt port for reuse error:%s(errno:%d)\n", strerror(errno), errno);
    }

    /* 2.设置服务器sockaddr_in结构 */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(LISTEN_PORT);

    /* 3.绑定socket和端口 */
    if (bind(sever_sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        log_e("bind socket error:%s(errno:%d)", strerror(errno), errno);
        exit(1);
    }

    /* 4.监听,最大连接客户端数 BACKLOG */
    if (listen(sever_sock, BACKLOG) < 0)
    {
        log_e("listen socket error :%s(errno:%d)", strerror(errno), errno);
        exit(1);
    }
    log_i("waiting for clients to connect ...");

    // 获取eth0的 ip地址
    get_localip("eth0", serverip);
    log_i("ip   [%s]",serverip);
    log_i("port [%d]", LISTEN_PORT);

    while (1)
    {
        addrLen = sizeof(struct sockaddr);
        /* 5.接受客户请求，并创建线程处理 */
        if ((client_sock = accept(sever_sock, (struct sockaddr *)&clientAddr, &addrLen)) < 0)
        {
            log_e("accept socket error:%s(errorno:%d)", strerror(errno), errno);
            continue;
        }
        //打印客户端连接次数及IP地址
        clientip = inet_ntoa(clientAddr.sin_addr);
        log_i("conneted success from clinet [NO.%d] IP: [%s]", ++clientCnt, clientip);

        pthread_create(&send_tid, NULL, send_thread, clientip);
        pthread_detach(send_tid);

        pthread_create(&recv_tid, NULL, recv_thread, clientip);
        pthread_detach(recv_tid);
    }
    close(sever_sock);
}


/**
  * @brief  数据服务器线程初始化
  */
int server_thread_init(void)
{
    pthread_t server_tid;

    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_detach(server_tid);

    return 0;
}