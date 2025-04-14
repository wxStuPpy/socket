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

#define BUFFER_SIZE 1024
static const char* status_line[2] = { "200 OK", "500 Internal server error" };

// http 解析
int main(int argc, char* argv[]) {
    if (argc <= 3) {
        printf("Usage: %s <ip> <port> <file_name>\n", argv[0]);
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* file_name = argv[3];

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &address.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addr_len);
    assert(connfd >= 0);

    // HTTP 解析
    // 用于保存 HTTP 应答的状态行 头部字段和一个空的缓冲区
    char head_buf[BUFFER_SIZE];
    memset(head_buf, '\0', BUFFER_SIZE);
    // 用于存放目标文件内容的应用程序缓存
    char* file_buf = nullptr;
    // 用于获取目标的属性 比如是否为目录 文件大小等
    struct stat file_stat;
    // 记录目标是否为有效文件
    bool valid = true;
    // 缓冲区 header_buf 已经使用了多少字节的空间
    int len = 0;
    // 如果目标文件不存在
    if (stat(file_name, &file_stat) < 0) {
        valid = false;
    }
    else {
        // 目标是一个目录
        if (S_ISDIR(file_stat.st_mode)) {
            valid = false;
        }
        // 当前用户拥有读取目标文件的权限
        else if (file_stat.st_mode & S_IROTH) {
            int fd = open(file_name, O_RDONLY);
            if (fd == -1) {
                valid = false;
            }
            else {
                file_buf = new char[file_stat.st_size + 1];
                memset(file_buf, '\0', file_stat.st_size + 1);
                if (read(fd, file_buf, file_stat.st_size) < 0) {
                    valid = false;
                }
                close(fd);
            }
        }
        else {
            valid = false;
        }
    }
    // 如果目标文件有效 则正常发送 http 应答
    if (valid) {
        ret = snprintf(head_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
        len += ret;
        ret = snprintf(head_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %ld\r\n", file_stat.st_size);
        len += ret;
        ret = snprintf(head_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

        // 利用 writev 将 head_buf 和 file_buf 内容一并写出
        struct iovec iv[2];
        iv[0].iov_base = head_buf;
        iv[0].iov_len = strlen(head_buf);
        iv[1].iov_base = file_buf;
        iv[1].iov_len = file_stat.st_size;

        ret = writev(connfd, iv, 2);
        if (ret == -1) {
            perror("writev");
        }
    }
    else {
        ret = snprintf(head_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
        len += ret;
        ret = snprintf(head_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
        if (send(connfd, head_buf, strlen(head_buf), 0) == -1) {
            perror("send");
        }
    }
    close(connfd);
    if (file_buf) {
        delete[] file_buf;
    }
    close(sock);
    return 0;
}    