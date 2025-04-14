#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 10

struct fds
{
    int epfd;
    int fd;
};

/* 将文件描述符设置为非阻塞 */
int setNOBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    if (old_option == -1)
    {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    int new_option = old_option | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, new_option) == -1)
    {
        perror("fcntl(F_SETFL)");
        return -1;
    }
    return old_option;
}

/* 将 fd 添加到 epoll 监听列表 */
void addfd(int epfd, int fd, bool oneshot)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    if (oneshot)
    {
        ev.events |= EPOLLONESHOT; // 保证一个socket只会被一个进程或线程处理
    }
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl");
        close(fd);
    }
    setNOBlocking(fd);
}
/*重置fd事件*/
void reset_oneshot(int epfd, int fd)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT; // 下一次可以继续操作fd 不过也只能由一个进程或线程操作
    ev.data.fd = fd;
    // 使用 EPOLL_CTL_MOD 操作来修改事件
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        perror("epoll_ctl");
        close(fd);
    }
}

/*工作线程*/
void* worker(void *arg)
{
    struct fds *fds_ptr = static_cast<struct fds *>(arg);
    int fd = fds_ptr->fd;
    int epfd = fds_ptr->epfd;
    delete fds_ptr; // 释放动态分配的内存
    printf("start a new thread to receive data on fd:%d\n", fd);
    char buf[BUFFER_SIZE];
    memset(buf, '\0', BUFFER_SIZE);
    while (1)
    {
        int ret = recv(fd, buf, BUFFER_SIZE - 1, 0);
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                printf("read later\n");
                reset_oneshot(epfd, fd);
                break;
            }
        }
        else if (ret == 0)
        {
            close(fd);
            printf("foreiner closed the connection\n");
            break;
        }
        else
        {
            printf("get %d bytes: %s\n", ret, buf);
            sleep(5);
        }
    }
    printf("end thread to receive data on fd:%d\n", fd);
    if(fds_ptr)
    delete fds_ptr;
    return nullptr;
}

int main()
{
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8888);

    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUM];
    int epfd = epoll_create(5);
    assert(epfd != -1);
    /*listened不能注册EPOLLONESHOT 否则程序只能处理一个客户端*/
    addfd(epfd, listenfd, false);

    while (1)
    {
        int ret = epoll_wait(epfd, events, MAX_EVENT_NUM, -1);
        if (ret < 0)
        {
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < ret; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
                if (connfd < 0)
                {
                    perror("accept");
                    continue;
                }
                /*对每个非监听文件描述符注册EPOLLIONESHOT事件*/
                addfd(epfd, connfd, true);
            }
            else if (events[i].events & EPOLLIN)
            {
                pthread_t pid;
                struct fds *ptr = new struct fds; // 动态分配内存
                ptr->epfd = epfd;
                ptr->fd = sockfd;
                // 启动一个新线程处理sockfd
                if (pthread_create(&pid, nullptr, worker, static_cast<void*>(ptr)) != 0)
                {
                    perror("pthread_create");
                    delete ptr; // 释放内存
                }
                // 分离线程，让系统自动回收资源
                pthread_detach(pid);
            }
            else
            {
                printf("unknown event\n");
            }
        }
    }
    close(listenfd);
    return 0;
}    