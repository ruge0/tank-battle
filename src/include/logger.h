#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <fstream>
#include <string>
#include <ctime>
using namespace std;

#define LOG_FILE "Server.log"

class Logger
{
private:
    //记录写入时的时间信息
    time_t m_now;
    char* m_time;
    //打开和关闭文件io流的条件变量
    static volatile int loggerNum;
    //文件io流
    static ofstream ofs;

public:
    Logger();

    //重载括号运算符让日志对象的变量可以像函数一样用
    void operator()(const char *str);

    //string类型的重载
    void operator()(const string str);

    ~Logger();
};


#endif //SERVER_LOGGER_H
