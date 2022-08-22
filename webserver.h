#ifndef WEBSERVER
#define WEBSERVER

#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "./timer/time_wheel.h"

const int MAX_EVENT_NUMBER = 10000; //最大事件数

class Webserver{
public:
    char* server_file_path;
    int listenfd;
    int port;
    int epollfd;
    bool time_out = false;
    bool stop_server = false;
    const int BUFFER_SIZE = 2048;
    time_wheel tw;
    epoll_event events[MAX_EVENT_NUMBER];
    static int pipefd[2];
    client_data* users = new client_data[65535];

public:
    Webserver();    // 设置服务端文件根路径
    ~Webserver(){};

    void setServerOption(const int);
    void eventListen();
    void eventLoop();
    bool dealClientConn();
    void dealClientData(int);
    void timer_handler();
    void deal_signal();
};

#endif