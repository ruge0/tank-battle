#ifndef SERVER_NET_H
#define SERVER_NET_H

#include <deque>
#include <vector>
#include <cstring>
#include <string>
#include <ctime>
#include <sys/socket.h>
#include <csignal>
#include <unistd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <algorithm>
using namespace std;
#include "logger.h"

#define PORT 45555
#define MAXLINK 20
#define BUFFNUM 1024
#define BUFFSIZE 100

class Net
{
private:
    //接收网络套接字用来给自身对应的客户端发信息
    int m_connfd;
    //发送缓冲区工作指针
    char *m_ch;
    //接收缓冲区
    char m_buff[BUFFSIZE];

    //首次申请网络资源的socke套接字
    static int sockfd;
    //网络协议簇
    static struct sockaddr_in sockaddr;
    //缓冲区，一个数组，里面每个元素都是一个字符指针
    static char* buff[];
    //所有已连接的套接字的队列，用来给所有人广播信息
    static vector<int> m_vecCoordinate;
    //可用的缓冲区队列
    static deque<char*> m_deqBuffIn;
    //要广播的缓冲区队列
    static deque<char*> m_daqBuffOut;
    //三个队列的互斥锁
    static pthread_mutex_t m_pthreadMutexBuffIn;
    static pthread_mutex_t m_pthreadMutexBuffOut;
    static pthread_mutex_t m_pthreadMutexConnfd;
    //初始化和释放网络资源的条件变量
    static volatile int m_nNetNum;
    //用来广播的线程 的 线程id号
    static pthread_t netSendId;
    //日志类变量
    static Logger logger;

public:
    //网络类构造函数，初始化网络信息
    explicit Net();

    //设置网络套接字，用来单独给对应的客户端发信息
    void setNetConnfd(int connfd);

    //接收到的客户端信息
    char* netIn();

    //给客户端发信息
    long netOut(const char* str);
    long netOut(string str);

    //给所有客户端广播信息
    bool netBroadcast(const char* str);
    bool netBroadcast(string str);

    //获取一个已连接的客户端的网络套接字
    int popConnfd();

    //用来给所有客户端广播的方法，需要单独一个线程工作
    static void* netSend(void* ptr);

    //析构函数，释放网络资源
    ~Net();
};

#endif //SERVER_NET_H
