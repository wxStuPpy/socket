#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0

int main() {
    int pipefd[2];  // pipefd[0] 读端, pipefd[1] 写端
    char buf[100];

    // 1. 创建管道
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    // 2. 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  // 子进程（读取数据）
        close(pipefd[1]);  // 关闭写端
        read(pipefd[0], buf, sizeof(buf));
        printf("Child received: %s\n", buf);
        close(pipefd[0]);
    } else {         // 父进程（写入数据）
        close(pipefd[0]);  // 关闭读端
        const char* msg = "Hello from parent!";
        write(pipefd[1], msg, strlen(msg) + 1);  // +1 包含 '\0'
        close(pipefd[1]);
    }
    return 0;
}

#endif

int main() {
    int pipefd[2];
    pipe(pipefd);

    if (fork() == 0) {  // 子进程：执行 grep
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);  // 将管道读端重定向到 stdin
        close(pipefd[0]);
        execlp("grep", "grep", "cpp", NULL);  // 执行 grep "c"
    } else {            // 父进程：执行 ls
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);  // 将管道写端重定向到 stdout
        close(pipefd[1]);
        execlp("ls", "ls", NULL);        // 执行 ls
    }
    return 0;
}