#include "webserver.h"
#include <unistd.h>
#include <string.h>

// 将文件描述符设置成非阻塞的
int setNonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_optino = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_optino);
    return old_option;
}

// 将事件注册到epoll事件表当中
void addfd(int epollfd, int fd, bool enable_et){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et){
        event.events |= EPOLLET;
    }
    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    assert(ret != -1);
    setNonblocking(fd);
}

bool Webserver::dealClientConn(){
    struct sockaddr_in client_addr;
    socklen_t client_socklen = sizeof(client_addr);
    
    // 默认为LT模式连接客户端
    if(1){
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_socklen);
        if(connfd < 0){
            printf("connection error");
        }
        addfd(epollfd, connfd, true);
    }
    return true;
}

void Webserver::dealClientData(int sockfd){
    char buff[BUFFER_SIZE];
    memset(buff, '\0', BUFFER_SIZE);
    int recv_len = recv(sockfd, buff, BUFFER_SIZE - 1, 0);
    printf("%s", buff);
}

Webserver::Webserver() {
    char cur_dic[4096]; // Linux系统中最长的路径名为4096bytes
    // TODO 字符串没有那么长，怎么去缩小他的空间
    getcwd(cur_dic, sizeof(cur_dic));
    char save_folder[6] = "/root";
    server_file_path = (char *) malloc(sizeof(save_folder) + sizeof(cur_dic) + 1);
    strcat(server_file_path, cur_dic);
    strcat(server_file_path, save_folder);
}

void Webserver::setServerOption(const int _port){
    port = _port;
}

// 开启服务器监听
void Webserver::eventListen(){
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_addr.s_addr = htons(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);
    
    // 创建epoll事件表
    epollfd = epoll_create(5);  // size暂时不起作用，只是给系统一个提示
    assert(epollfd != -1);

    // 将listenfd注册到到epoll内核事件表中
    addfd(epollfd, listenfd, true);
}

void Webserver::eventLoop(){
    bool stop_server = false;

    while(!stop_server){
        // 获取当前需要处理的事件数量
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno != EINTR){
            printf("epoll failure");
        }

        for(int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;

            if(sockfd == listenfd){
                // 处理连接请求
                bool conn_state = dealClientConn();
                if(conn_state == false){
                    continue;
                }
            }else if(events[i].events & EPOLLIN){
                // 处理从客户端传输的数据
                printf("get stream from client");
                dealClientData(sockfd);
            }
        }
    }
}
