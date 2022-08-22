#include "time_wheel.h"

void cb_func(client_data* user_data){
    epoll_ctl(time_wheel::epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    printf("close client connection\n");
}

int time_wheel::epollfd = 0;

time_wheel::time_wheel() : cur_slot(0){
    for(int i = 0; i < N; ++i){
        slots[i] = NULL;
    }
}

time_wheel::~time_wheel(){
    for(int i = 0; i < N; ++i){
        time_timer* tmp = slots[i];
        while(tmp){
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i];
        }
    }
}

time_timer* time_wheel::add_timer(int timeout){
    if(timeout < 0){
        return NULL;
    }

    int ticks = 0;  // 该定时器要在时间轮中转多少下

    if(timeout < SI){
        ticks = 1;
    }else{
        ticks = timeout / SI;
    }

    int ratation = ticks / N;
    int ts = (cur_slot + (ticks % N)) % N;
    time_timer* timer = new time_timer(ratation, ts);
    timer->cb_func = cb_func;

    if(!slots[ts]){
        slots[ts] = timer;
    }else{
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
    return timer;
}

void time_wheel::delete_timer(time_timer* timer){
    if(!timer){
        return;
    }

    int ts = timer->time_slot;  // timer的时间槽
    if(slots[ts] == timer){
        slots[ts] = timer->next;
        if(slots[ts]){
            slots[ts]->prev = NULL;
        }
    }else{
        timer->prev->next = timer->next;
        if(timer->next){
            timer->next->prev = timer->prev;
        }
    }
    delete timer;
}

void time_wheel::tick(){
    time_timer* cur_timer = slots[cur_slot];
    while(cur_timer){
        if(cur_timer->rotation > 0){
            cur_timer->rotation--;
            cur_timer = cur_timer->next;
        }else{
            cur_timer->cb_func(cur_timer->user_data);
            time_timer* tmp = cur_timer->next;
            // 移除timer;
            delete_timer(cur_timer);
            cur_timer = tmp;
        }
    }
    cur_slot = ++cur_slot % N;
}