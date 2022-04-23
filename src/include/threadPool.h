#ifndef SERVER_THREADPOOL_H
#define SERVER_THREADPOOL_H

#include <deque>
#include <string>
#include <pthread.h>
using namespace std;
#include "logger.h"
#include "cTask.h"

//线程池管理类
class CThreadPool
{
private:
    //声明任务队列
    static deque<Ctask*> m_deqTaskList;
    //声明线程是否退出标志
    static bool shutdown;
    //线程池中启动的线程数
    int m_iThreadNum;
    //维护所有线程号的数组
    pthread_t *pthread_id;

    //声明线程同步锁
    static pthread_mutex_t m_pthreadMutex;
    //声明线程同步条件变量
    static pthread_cond_t m_pthreadCond;
    //打印日志
    static Logger logger;

protected:
    //新线程回调函数
    static void* ThreadFunc(void *threadData);

    //创建线程池中的线程
    int Create();

public:
    //构造时不传参数默认创建10个进程
    CThreadPool(int threadNum = 10);

    //把任务添加到任务队列
    int AddTask(Ctask *task);

    //使线程池中的线程退出
    int StopAll();

    //获取当前任务队列中的任务数
    int getTaskSize();
};

#endif //SERVER_THREADPOOL_H
