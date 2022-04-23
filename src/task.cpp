#include "task.h"

namespace
{
    const char* host = "192.168.10.129";
    const char* user = "root";
    const char* passwd = "password";
    const char* db = "tank";
    const unsigned int port = 3306;
}

Logger Task::logger;
pthread_mutex_t Task::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Task::m_pthreadMutexTank = PTHREAD_MUTEX_INITIALIZER;
vector<Tank*> Task::m_vecTank;

Task::Task(int connfd)
{
    SetConnFd(connfd);
    this->m_net.setNetConnfd(connfd);

    pthread_mutex_lock(&m_pthreadMutex);
    mysql_init(&m_mysql);
    if (!mysql_real_connect(&m_mysql, host, user, passwd, db, port, nullptr, 0))
    {
        logger("mysql connect failed! " + to_string(mysql_errno(&m_mysql)));
        exit(-1);
    }
    pthread_mutex_unlock(&m_pthreadMutex);
}

bool Task::validateLogon()
{
    for (int i = 0; i < 3; i++)
    {
        string sql("select * from user where uName = ''");
        char* ch = m_net.netIn();
        if (nullptr == ch)
        {
            return false;
        }
        string str(ch);

        string filter(str.substr(0, 5));
        if (filter != string("user:"))
        {
            return false;
        }

        unsigned long head = str.find("user:");
        head = head + 5;
        unsigned long end = str.find('+', head);

        string user(str.substr(head, end - head));
        sql.insert(sql.size()-1, user);

        m_sql = &sql[0];
        m_re = mysql_real_query(&m_mysql, m_sql, sql.size());
        if (m_re)
        {
            logger("mysql_real_query faied! " + sql + to_string(mysql_errno(&m_mysql)));
            m_net.netOut("Server busy");
            mysql_free_result(m_result);
            continue;
        }

        m_result = mysql_store_result(&m_mysql);
        if (!m_result)
        {
            unsigned int field_count = mysql_field_count(&m_mysql);
            if (!field_count)
            {
                m_net.netOut("There is no such user:" + user);
            }
            else
            {
                logger("mysql_use_result faied! " + to_string(mysql_errno(&m_mysql)));
                m_net.netOut("Server busy");
            }
            mysql_free_result(m_result);
            continue;
        }

        head = str.find("password:");
        head = head + 9;
        end = str.find('+', head);
        string password(str.substr(head, end - head));

        m_row = mysql_fetch_row(m_result);
        if (m_row == NULL)
        {
            m_net.netOut("There is no such user:" + user);
            mysql_free_result(m_result);
            continue;
        }

        string passwordSql(m_row[2]);
        if (password == passwordSql)
        {
            string taskName(m_row[0]);
            taskName.insert(0, "1000");
            SetTaskName(taskName);
            m_net.netOut("Login:" + taskName);
            return true;
        }
        else
        {
            m_net.netOut("Password error");
            mysql_free_result(m_result);
            continue;
        }
    }

    return false;
}

bool Task::init()
{
    char * ch;
    ch = m_net.netIn();
    if (nullptr == ch)
    {
        return false;
    }

    string buff(ch);
    int head, end;
    string target;

    target = buff.substr(0, 4);
    if ("New:" != target)
    {
        return false;
    }

    head = buff.find("id:", 0);
    head += 3;
    end = buff.find(',', head);
    target = buff.substr(head, end - head);
    int id = stoi(target);

    head = buff.find("model:", end);
    head += 6;
    end = buff.find(',', head);
    target = buff.substr(head, end - head);
    int model = stoi(target);

    head = buff.find("color:", end);
    head += 6;
    end = buff.find(',', head);
    target = buff.substr(head, end - head);
    int color = stoi(target);

    head = buff.find("x:", end);
    head += 2;
    end = buff.find(',', head);
    target = buff.substr(head, end - head);
    int x = stoi(target);

    head = buff.find("y:", end);
    head += 2;
    end = buff.find(',', head);
    target = buff.substr(head, end - head);
    int y = stoi(target);

    this->tank = Tank(id, model, color, 0, x, y);

    loadTank();

    pthread_mutex_lock(&m_pthreadMutexTank);
    m_vecTank.push_back(&tank);
    pthread_mutex_unlock(&m_pthreadMutexTank);

    this->m_net.netBroadcast(ch);

    return true;
}

bool Task::loadTank()
{
    if (m_vecTank.empty())
    {
        return false;
    }

    for (Tank* t : m_vecTank)
    {
        m_net.netOut("New:id:" + to_string(t->m_id) + ",model:" + to_string(t->m_model) + ",color:" + to_string(t->m_color) + ",x:" +
                     to_string(t->x) + ",y:" + to_string(t->y) + ',');
    }

    return true;
}

int Task::Run()
{
    if (!validateLogon())
    {
        return -1;
    }

    if (!init())
    {
        return -1;
    }

    char* ch;
    string buff;
    string target;
    int head;
    int end;
    while (true)
    {
        ch = m_net.netIn();
        if (nullptr == ch)
        {
            string logout("Logout:");
            m_net.netBroadcast(logout + GetTaskName());
            break;
        }
        buff = ch;

        target = buff.substr(0, 4);
        if ("Move" == target)
        {
            head = buff.find("Move:", 0);
            head += 5;
            end = buff.find(',', head);
            target = buff.substr(head, end - head);
            if (target != m_StrTaskName)
            {
                continue;
            }

            head = buff.find("x:", end);
            head += 2;
            end = buff.find(',', head);
            target = buff.substr(head, end - head);
            tank.x = stoi(target);

            head = buff.find("y:", end);
            head += 2;
            end = buff.find(',', head);
            target = buff.substr(head, end - head);
            tank.y = stoi(target);

            head = buff.find("d:", end);
            head += 2;
            end = buff.find(',', head);
            target = buff.substr(head, end - head);
            tank.m_direction = stoi(target);
        }

        m_net.netBroadcast(ch);
    }

    return 0;
}

Task::~Task() noexcept
{

    auto info = find(m_vecTank.begin(), m_vecTank.end(), &tank);
    pthread_mutex_lock(&m_pthreadMutexTank);
    m_vecTank.erase(info);
    pthread_mutex_unlock(&m_pthreadMutexTank);

    mysql_close(&m_mysql);
    mysql_thread_end();
}
