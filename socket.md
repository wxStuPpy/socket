在 **Socket 编程** 中，`listen`、`bind`、`accept`、`connect`、`socket` 是最核心的几个函数，它们共同构成了 **TCP/IP 网络通信的基础流程**。以下是它们的作用和典型使用场景：

---

## **1. `socket()` —— 创建套接字**
**作用**：  
创建一个 **通信端点**（socket），返回一个 **文件描述符**（fd），用于后续的网络通信。  

**函数原型**：
```c
int socket(int domain, int type, int protocol);
```
**参数**：
- `domain`：协议族，如 `AF_INET`（IPv4）、`AF_INET6`（IPv6）。
- `type`：通信类型，如 `SOCK_STREAM`（TCP）、`SOCK_DGRAM`（UDP）。
- `protocol`：通常填 `0`（自动选择）。

**示例**：
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建 TCP 套接字
if (sockfd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
}
```

---

## **2. `bind()` —— 绑定 IP 和端口**
**作用**：  
将 `socket()` 创建的套接字 **绑定到指定的 IP 地址和端口**（通常用于 **服务端**）。

**函数原型**：
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
**参数**：
- `sockfd`：`socket()` 返回的文件描述符。
- `addr`：指向 `struct sockaddr`（存储 IP 和端口信息）。
- `addrlen`：`addr` 结构的大小。

**示例**：
```c
struct sockaddr_in serv_addr;
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
serv_addr.sin_port = htons(8080);       // 绑定 8080 端口

if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
}
```

---

## **3. `listen()` —— 监听连接（服务端）**
**作用**：  
让套接字进入 **被动监听模式**，等待客户端连接（仅用于 **TCP 服务端**）。

**函数原型**：
```c
int listen(int sockfd, int backlog);
```
**参数**：
- `sockfd`：`bind()` 后的套接字。
- `backlog`：**等待连接队列的最大长度**（通常设为 `5` 或 `10`）。

**示例**：
```c
if (listen(sockfd, 5) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
}
printf("Server listening on port 8080...\n");
```

---

## **4. `accept()` —— 接受连接（服务端）**
**作用**：  
从 `listen()` 的队列中 **接受一个客户端连接**，返回一个新的套接字（用于和该客户端通信）。

**函数原型**：
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
**参数**：
- `sockfd`：`listen()` 后的套接字。
- `addr`：存放客户端地址信息（可填 `NULL` 表示不关心）。
- `addrlen`：地址结构大小（可填 `NULL`）。

**示例**：
```c
struct sockaddr_in cli_addr;
int cli_len = sizeof(cli_addr);
int new_sock = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
if (new_sock < 0) {
    perror("accept failed");
    exit(EXIT_FAILURE);
}
printf("Client connected!\n");
```

---

## **5. `connect()` —— 发起连接（客户端）**
**作用**：  
客户端 **主动连接** 服务端（仅用于 **TCP 客户端**）。

**函数原型**：
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
**参数**：
- `sockfd`：`socket()` 创建的套接字。
- `addr`：服务端的地址信息（IP + 端口）。
- `addrlen`：地址结构大小。

**示例**：
```c
struct sockaddr_in serv_addr;
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(8080);
inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // 连接本地 8080 端口

if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connection failed");
    exit(EXIT_FAILURE);
}
printf("Connected to server!\n");
```

---

## **6. `send()` / `recv()` —— 发送和接收数据**
**作用**：  
在已连接的套接字上 **发送/接收数据**（TCP）或 **发送/接收数据报**（UDP）。

**函数原型**：
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```
**示例**：
```c
char buffer[1024] = {0};
recv(new_sock, buffer, sizeof(buffer), 0); // 接收数据
send(new_sock, "Hello from server!", 18, 0); // 发送数据
```

---

## **7. `close()` —— 关闭套接字**
**作用**：  
关闭套接字，释放资源。

**函数原型**：
```c
int close(int sockfd);
```
**示例**：
```c
close(new_sock); // 关闭客户端连接
close(sockfd);   // 关闭服务端监听
```

---

## **总结**
| 函数 | 作用 | 适用端 |
|------|------|--------|
| `socket()` | 创建套接字 | 客户端/服务端 |
| `bind()` | 绑定 IP 和端口 | **服务端** |
| `listen()` | 监听连接 | **服务端** |
| `accept()` | 接受连接 | **服务端** |
| `connect()` | 发起连接 | **客户端** |
| `send()` / `recv()` | 发送/接收数据 | 客户端/服务端 |
| `close()` | 关闭套接字 | 客户端/服务端 |

---

### **TCP 通信流程**
1. **服务端**：
   - `socket()` → `bind()` → `listen()` → `accept()` → `send()`/`recv()` → `close()`
2. **客户端**：
   - `socket()` → `connect()` → `send()`/`recv()` → `close()`

这些函数构成了 **TCP 网络编程的基础**，理解它们的作用和调用顺序非常重要！ 🚀