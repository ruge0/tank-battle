#include "cTask.h"

void Ctask::SetTaskName(std::string name)
{
    this->m_StrTaskName = name;
}

std::string Ctask::GetTaskName()
{
    return this->m_StrTaskName;
}

void Ctask::SetConnFd(int data)
{
    this->m_connfd = data;
}

int Ctask::GetConnFd()
{
    return this->m_connfd;
}

Ctask::~Ctask() {}
