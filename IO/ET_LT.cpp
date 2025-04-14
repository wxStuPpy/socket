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

/* 将文件描述符设置为非阻塞 */
int setNOBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    if (old_option == -1) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    int new_option = old_option | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, new_option) == -1) {
        perror("fcntl(F_SETFL)");
        return -1;
    }
    return old_option;
}

/* 将 fd 添加到 epoll 监听列表 */
void addfd(int epfd, int fd, bool enable_et) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (enable_et) {
        ev.events |= EPOLLET;
    }
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        close(fd);
    }
    setNOBlocking(fd);
}

/* 水平触发（LT）模式 */
void lt(epoll_event *events, int num, int epfd, int listenfd) {
    char buf[BUFFER_SIZE];
    for (int i = 0; i < num; ++i) {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd) {
            // 新连接
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
            if (connfd < 0) {
                perror("accept");
                continue;
            }
            addfd(epfd, connfd, false);  // LT 模式
        } else if (events[i].events & EPOLLIN) {
            // 可读事件
            printf("LT event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if (ret <= 0) {
                close(sockfd);
                continue;
            }
            printf("get %d bytes: %s\n", ret, buf);
        } else {
            printf("unknown event\n");
        }
    }
}

/* 边沿触发（ET）模式 */
void et(epoll_event *events, int num, int epfd, int listenfd) {
    char buf[BUFFER_SIZE];
    for (int i = 0; i < num; ++i) {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd) {
            // 新连接
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
            if (connfd < 0) {
                perror("accept");
                continue;
            }
            addfd(epfd, connfd, true);  // ET 模式
        } else if (events[i].events & EPOLLIN) {
            // 可读事件（必须循环读完）
            printf("ET event trigger once\n");
            while (1) {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if (ret < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                } else if (ret == 0) {
                    close(sockfd);  // 客户端关闭
                } else {
                    printf("get %d bytes: %s\n", ret, buf);
                }
            }
        } else {
            printf("unknown event\n");
        }
    }
}

int main()
{
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUM];
    int epfd = epoll_create(5);
    assert(epfd != -1);
    addfd(epfd, listenfd, true);

    while (1)
    {
        int number = epoll_wait(epfd, events, MAX_EVENT_NUM, -1);
        if (number < 0)
        {
            printf("epoll failure\n");
            break;
        }
        et(events, number, epfd, listenfd);
        //lt(events, number, epfd, listenfd);
    }

    close(listenfd);
    return 0;
}