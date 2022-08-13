#ifndef TIME_WHEEL
#define TIME_WHEEL

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

class time_timer;

struct client_data{
    sockaddr_in address;
    int sockfd;
    time_timer *timer;
};

class time_timer{
public:
    time_timer(int rot, int ts): rotation(rot), time_slot(ts), next(NULL), prev(NULL){};

public:
    int rotation;   // 记录定时器在多少圈后生效
    int time_slot;  // 记录定时器属于哪个槽
    void (*cb_func) (client_data*); // 定时回调函数
    client_data* user_data;
    time_timer* prev;
    time_timer* next;

};

class time_wheel
{
private:
   static const int N = 60; // 时间轮上槽的数目
   static const int SI = 1; // 时间槽转换间隔
   time_timer* slots[N];
   int cur_slot;    // 当前槽
public:
    time_wheel();
    ~time_wheel();
    time_timer* add_timer(int timeout); // 添加一个定时器，传入超时时间
    void delete_timer(time_timer* timer);
    void tick();    // 心搏函数，槽转换
};


#endif