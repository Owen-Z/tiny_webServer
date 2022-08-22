#ifndef USER_DATA
#define USER_DATA
#include <netinet/in.h>
#include "./timer/time_wheel.h"

struct client_data{
    sockaddr_in address;
    int sockfd;
    time_timer *timer;
};

#endif