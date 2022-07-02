#include<sys/socket.h>
#include<netdb.h>
#include<iostream>
#include<arpa/inet.h>
#include<strings.h>
#include<unistd.h>
#include<string.h>



using namespace std;

int main(){
    int socketFileDescriber = socket(AF_INET, SOCK_STREAM, 0);

    hostent *h = gethostbyname("www.baidu.com");
    char *IPAdress = inet_ntoa(*(in_addr *)h->h_addr);
    
    sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(4000);
    server.sin_family = AF_INET;
    bzero(&(server.sin_zero), 8);
    int i = connect(socketFileDescriber, (sockaddr *)&server, sizeof(sockaddr));
    
    char buf[500];
    string headString("GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");

    char *header;
    header = &headString[0];
    send(socketFileDescriber, header, strlen(header),0);
    cout << recv(socketFileDescriber, buf, 500, 0);

    cout << buf;
    // for(auto const item : buf){
    //     cout << item;    
    // }
    close(socketFileDescriber);
    return 0;
}