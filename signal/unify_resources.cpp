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
#include <signal.h>

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];

int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epfd,int fd){
    epoll_event ev;
    ev.data.fd=fd;
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
    setNonBlocking(fd);
}

/*信号处理函数*/
void sig_handler(int sig){
    /*保留原来的errno 在函数最后恢复 保证函数的可重入性*/
    int save_errno=errno;
    int msg=sig;
    send(pipefd[1],(char*)&msg,1,0);//将信号值写入管道 通知主循环
    errno=save_errno;
}