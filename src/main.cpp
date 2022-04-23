#include "net.h"
#include "task.h"
#include "threadPool.h"
#include "logger.h"

int main() {

    Net net;
    CThreadPool pool;
    Logger logger;
    int connfd;

    while (true)
    {
        connfd = net.popConnfd();
        if (0 > connfd)
        {
            logger("cli connect failed:" + string(strerror(errno)));
        }
        else
        {
            Ctask * ta = new Task(connfd);
            pool.AddTask(ta);
        }
    }

    return 0;
}
