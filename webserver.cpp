#include "webserver.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>

int Webserver::pipefd[2]{0, 0};
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

// 信号处理函数
void sig_handle(int sig){
    int save_errno = errno;
    int msg = sig;
    send(Webserver::pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

// 添加需要处理的信号
void addsig(int sig){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handle;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void Webserver::timer_handler()
{
    tw.tick();
    alarm(1);
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
        users[connfd].address = client_addr;
        users[connfd].sockfd = connfd;
        time_timer* timer = tw.add_timer(10);
        users[connfd].timer = timer;
        timer->user_data = &users[connfd];
    }
    return true;
}

void Webserver::dealClientData(int sockfd){
    char buff[BUFFER_SIZE];
    memset(buff, '\0', BUFFER_SIZE);
    int recv_len = recv(sockfd, buff, BUFFER_SIZE - 1, 0);

    printf("get msg from client: %s", buff);
    time_timer* timer = users[sockfd].timer;
    if(recv_len < 0){
        // 收到异常
        if(errno != EAGAIN){
            cb_func(&users[sockfd]);
            if(timer){
                tw.delete_timer(timer);
            }
        }
    }else if(recv_len == 0){
        // 对方关闭连接
        cb_func(&users[sockfd]);
        if(timer){
            tw.delete_timer(timer);
        }
    }else{
        // 正常收到数据，修改定时器
        if(timer){
            tw.delete_timer(timer);
            time_timer* update_timer = tw.add_timer(10);
            users[sockfd].timer = update_timer;
            update_timer->user_data = &users[sockfd];
        }
    }

}

void Webserver::deal_signal(){
    char signals[1024];
    int ret = recv(pipefd[0], signals, sizeof(signals), 0);
    if(ret == -1){
        // TODO handle error
        return;
    }else if(ret == 0){
        return;
    }else{
        for(int i = 0; i < ret; ++i){
            switch (signals[i])
            {
            case SIGALRM:
                time_out = true;
                break;
            
            case SIGTERM:
                stop_server = true;
                break;
            }
        }
    }
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
    time_wheel::epollfd = epollfd;

    // 将listenfd注册到到epoll内核事件表中
    addfd(epollfd, listenfd, true);

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    printf("%d, %d", pipefd[0], pipefd[1]);
    // 将写管道设为非阻塞
    setNonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0], true);

    addsig(SIGALRM);
    addsig(SIGTERM);
    alarm(5);

}

void Webserver::eventLoop(){

    
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
                printf("receive connetion request\n");
                bool conn_state = dealClientConn();
                if(conn_state == false){
                    continue;
                }
            }else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                // 从管道读取到信息
                printf("get msg from pipe\n");
                deal_signal();
            }else if(events[i].events & EPOLLIN){
                // 处理从客户端传输的数据
                dealClientData(sockfd);
            }
        }
        if(time_out){
            timer_handler();
            time_out = false;
        }
    }
}
