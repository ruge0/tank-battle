#include "threadPool.h"

Logger CThreadPool::logger;

//定义CThreadPool类声明的静态成员变量m_deqTaskList
deque<Ctask*> CThreadPool::m_deqTaskList;

//定义CThreadPool类声明的静态成员变量shutdowm
bool CThreadPool::shutdown = false;

//定义CThreadPool类声明的静态成员变量m_pthreadMutex并利用宏初始化这个锁
pthread_mutex_t CThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;

//定义CThreadPool类声明的静态成员变量m_pthreadCond并利用宏初始化这个条件
pthread_cond_t CThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;

CThreadPool::CThreadPool(int threadNum)
{
    this->m_iThreadNum = threadNum;
    logger("Thread pool startup, number of threads:" + to_string(this->m_iThreadNum));
    Create();
}

int CThreadPool::Create()
{
    pthread_id = new pthread_t[this->m_iThreadNum];
    for (int i = 0; i < this->m_iThreadNum; i++)
    {
        pthread_create(pthread_id + i, NULL, ThreadFunc, NULL);
    }
    return 0;
}

void *CThreadPool::ThreadFunc(void *threadData)
{
    pthread_t tid = pthread_self();
    while (true)
    {
        pthread_mutex_lock(&m_pthreadMutex);
        while (0 == m_deqTaskList.size() && !shutdown)
        {
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
        }

        if (shutdown)
        {
            pthread_mutex_unlock(&m_pthreadMutex);
            logger("thread will exit:" + to_string(tid));
            pthread_exit(NULL);
        }

        logger("thread run:" + to_string(tid));

        Ctask* task = m_deqTaskList.front();
        m_deqTaskList.pop_front();

        pthread_mutex_unlock(&m_pthreadMutex);

        task->Run();

        delete task;
    }

    return (void *)0;
}

int CThreadPool::AddTask(Ctask *task)
{
    pthread_mutex_lock(&m_pthreadMutex);
    this->m_deqTaskList.push_back(task);
    pthread_mutex_unlock(&m_pthreadMutex);

    pthread_cond_signal(&m_pthreadCond);
    return 0;
}

int CThreadPool::StopAll()
{
    if (shutdown)
    {
        return -1;
    }

    logger("Destroy thread pool");

    shutdown = true;
    pthread_cond_broadcast(&m_pthreadCond);

    for (int i = 0; i < m_iThreadNum; i++)
    {
        pthread_join(pthread_id[i], NULL);
    }

    delete[] pthread_id;
    pthread_id = NULL;

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    return 0;
}