#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>

const int MAX_EVENT_NUMBER = 10000; //最大事件数

class Webserver{
public:
    char* server_file_path;
    int listenfd;
    int port;
    int epollfd;
    const int BUFFER_SIZE = 2048;
    epoll_event events[MAX_EVENT_NUMBER];

public:
    Webserver();    // 设置服务端文件根路径
    ~Webserver(){};

    void setServerOption(const int);
    void eventListen();
    void eventLoop();
    bool dealClientConn();
    void dealClientData(int);
};