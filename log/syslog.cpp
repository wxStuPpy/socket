#include <stdio.h>
#include <syslog.h>

int main() {
    // 打开日志连接
    openlog("myapp", LOG_PID | LOG_CONS, LOG_USER);

    // 记录不同级别的日志
    syslog(LOG_INFO, "这是一条信息级别的日志");
    syslog(LOG_WARNING, "这是一条警告级别的日志");
    syslog(LOG_ERR, "这是一条错误级别的日志");

    // 关闭日志连接
    closelog();

    return 0;
}