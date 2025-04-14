#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    int connfd = accept(sock, (sockaddr*)&client, &client_addr_len);
    assert(connfd >= 0);

    close(STDOUT_FILENO);
    if (dup(connfd) == -1) {  // 检查 dup 是否成功
        perror("dup failed");
        close(connfd);
        close(sock);
        return 1;
    }
    printf("abcd\n");
    fflush(stdout);  // 确保数据发送

    close(connfd);
    close(sock);
    return 0;
}