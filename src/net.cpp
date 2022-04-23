#include "net.h"

int Net::sockfd;
struct sockaddr_in Net::sockaddr;
char* Net::buff[BUFFNUM];
vector<int> Net::m_vecCoordinate;
deque<char*> Net::m_deqBuffIn;
deque<char*> Net::m_daqBuffOut;
pthread_mutex_t Net::m_pthreadMutexBuffIn = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Net::m_pthreadMutexBuffOut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Net::m_pthreadMutexConnfd = PTHREAD_MUTEX_INITIALIZER;
volatile int Net::m_nNetNum = 0;
pthread_t Net::netSendId;
Logger Net::logger;

Net::Net()
{
    if (!__sync_fetch_and_add(&m_nNetNum, 1))
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == sockfd || 0 == sockfd)
        {
            logger("Create socket error:" + string(strerror(errno)));
            exit(-1);
        }

        bzero(&sockaddr, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sockaddr.sin_port = htons(PORT);
        if (-1 == bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)))
        {
            logger("Bind error:" + string(strerror(errno)));
            exit(-1);
        }

        if (-1 == listen(sockfd, MAXLINK))
        {
            logger("Listen error:" + string(strerror(errno)));
            exit(-1);
        }

        logger("Server Listening...");

        pthread_mutex_lock(&m_pthreadMutexBuffIn);
        for (auto & i : buff)
        {
            i = new char[BUFFSIZE];
            bzero(i, BUFFSIZE);
            m_deqBuffIn.push_back(i);
        }
        pthread_mutex_unlock(&m_pthreadMutexBuffIn);

        pthread_create(&netSendId, nullptr, netSend, nullptr);
    }
}

void Net::setNetConnfd(int connfd)
{
    this->m_connfd = connfd;
}

int Net::popConnfd()
{
    int connfd = accept(sockfd, nullptr, nullptr);
    if (0 < connfd)
    {
        pthread_mutex_lock(&m_pthreadMutexConnfd);
        m_vecCoordinate.push_back(connfd);
        pthread_mutex_unlock(&m_pthreadMutexConnfd);
    }
    return connfd;
}

char* Net::netIn()
{
    bzero(this->m_buff, BUFFSIZE);
    if (0 == recv(this->m_connfd, this->m_buff, BUFFSIZE, MSG_NOSIGNAL))
    {
        return nullptr;
    }
    return m_buff;
}

long Net::netOut(string str)
{
    this->m_ch = &str[0];
    return send(this->m_connfd, this->m_ch, str.size(), MSG_NOSIGNAL);
}

long Net::netOut(const char *str)
{
    return send(this->m_connfd, str, strlen(str), MSG_NOSIGNAL);
}

bool Net::netBroadcast(string str)
{
    if (m_deqBuffIn.empty() || BUFFSIZE < str.size())
    {
        return false;
    }

    pthread_mutex_lock(&m_pthreadMutexBuffIn);
    this->m_ch = m_deqBuffIn.front();
    m_deqBuffIn.pop_front();
    pthread_mutex_unlock(&m_pthreadMutexBuffIn);

    memcpy(this->m_ch, &str[0], str.size());
    pthread_mutex_lock(&m_pthreadMutexBuffOut);
    m_daqBuffOut.push_back(this->m_ch);
    pthread_mutex_unlock(&m_pthreadMutexBuffOut);

    return true;
}

bool Net::netBroadcast(const char *str)
{
    if (m_deqBuffIn.empty() || BUFFSIZE < strlen(str))
    {
        return false;
    }

    pthread_mutex_lock(&m_pthreadMutexBuffIn);
    this->m_ch = m_deqBuffIn.front();
    m_deqBuffIn.pop_front();
    pthread_mutex_unlock(&m_pthreadMutexBuffIn);

    memcpy(this->m_ch, str, strlen(str));
    pthread_mutex_lock(&m_pthreadMutexBuffOut);
    m_daqBuffOut.push_back(this->m_ch);
    pthread_mutex_unlock(&m_pthreadMutexBuffOut);

    return true;
}

void *Net::netSend(void *ptr)
{
    char *ch;
    while (true)
    {
        if (m_daqBuffOut.empty())
        {
            if (!sockfd)
            {
                break;
            }
            continue;
        }

        pthread_mutex_lock(&m_pthreadMutexBuffOut);
        ch = m_daqBuffOut.front();
        m_daqBuffOut.pop_front();
        pthread_mutex_unlock(&m_pthreadMutexBuffOut);

        for (int temp : m_vecCoordinate)
        {
            send(temp, ch, strlen(ch), MSG_NOSIGNAL);
        }

        bzero(ch, BUFFSIZE);
        pthread_mutex_lock(&m_pthreadMutexBuffIn);
        m_deqBuffIn.push_back(ch);
        pthread_mutex_unlock(&m_pthreadMutexBuffIn);
    }

    pthread_exit(nullptr);
}

Net::~Net()
{
    if (!__sync_sub_and_fetch(&m_nNetNum, 1))
    {
        close(sockfd);
        sockfd = 0;
        for (auto & i : buff)
        {
            delete i;
            i = nullptr;
        }
        pthread_join(netSendId, nullptr);
    }
    else
    {
        auto ite = find(m_vecCoordinate.begin(), m_vecCoordinate.end(), this->m_connfd);
        pthread_mutex_lock(&m_pthreadMutexConnfd);
        m_vecCoordinate.erase(ite);
        pthread_mutex_unlock(&m_pthreadMutexConnfd);
        close(this->m_connfd);
    }
}