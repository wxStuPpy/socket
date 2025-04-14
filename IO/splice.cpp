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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP> <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int ret = bind(sockfd, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    ret = listen(sockfd, 5);
    assert(ret != -1);

    int connfd = accept(sockfd, nullptr, nullptr);
    assert(connfd > 0);

    // 设置非阻塞模式
    int flags = fcntl(connfd, F_GETFL, 0);
    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

    int pipefd[2];
    ret = pipe(pipefd);
    assert(ret != -1);

    while (true) {
        // 从客户端读取数据到管道写入端（pipefd[1]）
        ssize_t bytes_read = splice(connfd, nullptr, pipefd[1], nullptr, 32768, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
        if (bytes_read < 0) {
            if (errno == EAGAIN) continue; // 非阻塞重试
            perror("splice read failed");
            break;
        } else if (bytes_read == 0) {
            break; // 连接关闭
        }

        // 从管道读取端（pipefd[0]）写回客户端
        ssize_t bytes_written = splice(pipefd[0], nullptr, connfd, nullptr, bytes_read, SPLICE_F_MORE);
        if (bytes_written <= 0) {
            perror("splice write failed");
            break;
        }
    }

    // 循环结束后关闭管道
    close(pipefd[0]);
    close(pipefd[1]);
    close(connfd);
    close(sockfd);
    return 0;
}