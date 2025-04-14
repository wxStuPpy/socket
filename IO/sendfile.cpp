#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/sendfile.h>
//文件与网络之间
int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <IP> <Port> <FileName>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *fileName = argv[3];

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

    int filefd = open(fileName, O_RDONLY);
    assert(filefd != -1);

    struct stat stat_buf;  // 栈上分配结构体
    fstat(filefd, &stat_buf);  // 传递地址

    off_t offset = 0;  // 从文件开头发送
    ssize_t sent = sendfile(connfd, filefd, &offset, stat_buf.st_size);
    if (sent == -1) {
        perror("sendfile failed");
    }

    close(connfd);
    close(filefd);
    close(sockfd);

    return 0;
}