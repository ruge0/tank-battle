#ifndef SERVER_TASK_H
#define SERVER_TASK_H

#include <mysql/mysql.h>
#include <algorithm>
using namespace std;
#include "net.h"
#include "cTask.h"
#include "logger.h"
#include "tank.h"

//线程池通过重载的Run函数来执行任务
class Task :public Ctask
{
private:
    //日志类对象，所有任务共用一个日志对象减小开销
    static Logger logger;
    //mysql初始化方法线程不安全，所以加把锁
    static pthread_mutex_t m_pthreadMutex;
    //访问坦克基本信息队列的锁
    static pthread_mutex_t m_pthreadMutexTank;
    //坦克基本信息队列
    static vector<Tank*> m_vecTank;

    //操作mysql的变量
    Net m_net;
    MYSQL m_mysql;
    const char* m_sql;
    int m_re;
    MYSQL_RES* m_result;
    MYSQL_ROW m_row;
    //记录坦克信息的变量
    Tank tank;

public:
    //任务类构造函数，需要一个网络套接字作为参数
    explicit Task(int connfd);
    //线程池的任务入口
    int Run() override;
    //验证用户登录信息
    bool validateLogon();
    //记录用户坦克初始化信息
    bool init();
    //向用户同步场上已有的坦克信息
    bool loadTank();

    ~Task() override;
};


#endif //SERVER_TASK_H
