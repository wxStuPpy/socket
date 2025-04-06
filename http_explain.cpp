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

#define BUFFER_SIZE 4096 /*读缓冲区*/

/*主状态机的两种状态 1.正在分析请求行 2.正在分析请求头*/
enum CHECK_STATE{
    CHECK_STATE_REQUESTLINE,
    CHECK_STATE_HEADER
};

/*从状态机的三种状态 1.读取到完整行 2.行出错 3.行数据尚不完整*/
enum LINE_STATUS{
    LINE_OK,
    LINE_BAD,
    LINE_OPEN
};

// 定义一个名为 HTTP_CODE 的枚举类型，用于表示不同的 HTTP 请求状态码或处理结果
enum HTTP_CODE{
    // 表示没有接收到任何请求的状态
    NO_REQUESE,
    // 表示成功接收到 GET 请求的状态
    GET_REQUEST,
    // 表示接收到的请求格式错误或不符合规范的状态
    BAD_REQUSET,
    // 表示客户端请求被服务器禁止访问的状态
    FORBIDDEN_REQUEST,
    // 表示服务器内部出现错误，无法正常处理请求的状态
    INTERNAL_ERROR,
    // 表示客户端与服务器之间的连接已关闭的状态
    CLOSED_CONNECTION
};

static const char *szert[]={"I get a correct relust\n","Something wrong\n"};

/*从状态机 用来解析一行的内容*/
LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index) {
    char temp;
    // 循环条件：checked_index 到 read_index - 1 之间的内容需要被分析
    // 0 到 checked_index 已经分析完毕
    for (; checked_index < read_index; ++checked_index) {
        // 获取当前要解析的字节
        temp = buffer[checked_index];
        // 如果当前字符是 '\r'，说明可能读到一个完整的行
        if (temp == '\r') {
            // 如果 '\r' 是缓冲区的最后一个字符，说明行数据尚不完整
            if (checked_index + 1 == read_index) {
                return LINE_OPEN;
            } 
            // 如果 '\r' 后面紧跟着 '\n'，说明读到了一个完整的行
            else if (buffer[checked_index + 1] == '\n') {
                // 将 '\r' 替换为字符串结束符 '\0'
                buffer[checked_index++] = '\0';
                // 将 '\n' 替换为字符串结束符 '\0'
                buffer[checked_index++] = '\0';
                // 返回表示读到完整行的状态
                return LINE_OK;
            }
            // 如果 '\r' 后面不是 '\n'，说明行格式错误
            return LINE_BAD;
        } 
        // 如果当前字符是 '\n'
        else if (temp == '\n') {
            // 如果 '\n' 前面是 '\r'，且前面至少还有一个字符
            if ((checked_index > 1) && buffer[checked_index - 1] == '\r') {
                // 将 '\r' 替换为字符串结束符 '\0'
                buffer[checked_index - 1] = '\0';
                // 将 '\n' 替换为字符串结束符 '\0'
                buffer[checked_index++] = '\0';
                // 返回表示读到完整行的状态
                return LINE_OK;
            }
            // 如果 '\n' 前面不是 '\r'，说明行格式错误
            return LINE_BAD;
        }
    }
    // 如果循环结束都没有找到完整的行，说明行数据尚不完整
    return LINE_OPEN;
}



int main(int argc,char*argv[]){

}