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

/*将文件描述符设置为非阻塞*/
int setNOBlocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=O_NONBLOCK|old_option;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

int main(int argc,char*argv[]){

}