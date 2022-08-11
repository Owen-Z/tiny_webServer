#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<iostream>
#include<fstream>

using namespace std;

int main(){
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1) cout << "FD error";
    sockaddr_in intnetSockAddr, theirAddr; // socket address in internet

    // convert "xxx.xxx.xxx.xxx" to internet binary data
    intnetSockAddr.sin_addr.s_addr = INADDR_ANY;
    intnetSockAddr.sin_family = AF_INET;
    intnetSockAddr.sin_port = htons(4000);
    memset(&intnetSockAddr.sin_zero, 0, sizeof(intnetSockAddr.sin_zero));

    // if set port 80 will permisson denied
    // the address should let the os set, otherwise the address will show be used
    if(bind(socketFD, (sockaddr *)&intnetSockAddr,  sizeof(sockaddr_in)) == -1)
        perror("bind");
    
    if(listen(socketFD, 10) == -1) perror("listen");

    int acceptFD;
    socklen_t sockaddrInSize = sizeof(sockaddr);
    acceptFD = accept(socketFD, (struct sockaddr *)&theirAddr, &sockaddrInSize);
    if(acceptFD == -1) perror("accept");

    char buf[500];
    cout << "recv length: " << recv(acceptFD, buf, 500, 0) << endl;

    char *sendMessage = (char*)malloc(sizeof(char) * 500);
    
    fstream sendFile;
    sendFile.open("/home/zy/Desktop/myproject/web_server/test.html", 
                    ios::binary | ios::out | ios::in | ios::ate);                
    int size = sendFile.tellg();
    cout << "size is : " << size << endl;
    char* text = (char*)malloc(sizeof(char) * (size+1));
    sendFile.seekg(0);
    sendFile.read(text, size);
    // text[size] = NULL;
    // cout << "read from file: " << text;
    printf("read from file:%s", text);

    if(send(acceptFD, text, size+1, 0) == -1) perror("send");
    // printf("Received: %s",buf);
    return 0;


}