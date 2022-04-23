#include "logger.h"

volatile int Logger::loggerNum = 0;
ofstream Logger::ofs(LOG_FILE, ios::out);

Logger::Logger()
{
    __sync_fetch_and_add(&loggerNum, 1);
}

void Logger::operator()(const char *str)
{
    m_now = time(0);
    m_time = ctime(&m_now);

    ofs << m_time << str << '\n' << endl;
}

void Logger::operator()(const string str)
{
    m_now = time(0);
    m_time = ctime(&m_now);

    ofs << m_time << str << '\n' <<endl;
}

Logger::~Logger() throw()
{
    if (!__sync_sub_and_fetch(&loggerNum, 1))
    {
        ofs.close();
    }
}