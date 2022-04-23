#ifndef SERVER_CTASK_H
#define SERVER_CTASK_H

#include <string>

//执行任务的类
class Ctask
{
protected:
    std::string m_StrTaskName;
    int m_connfd;

public:
    Ctask() = default;

    //子类通过重写纯虚函数Run指定任务内容
    virtual int Run() = 0;

    //设置任务名称
    void SetTaskName(std::string name);

    //获取任务名
    std::string GetTaskName();

    //接收网络套接字
    void SetConnFd(int data);

    //返回网络套接字
    int GetConnFd();

    //子类写虚析构函数让父类指针可以析构子类对象
    virtual ~Ctask();
};

#endif //SERVER_CTASK_H
