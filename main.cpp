#include "webserver.h"

int main(int argc, char* argv[]){
    Webserver webserver;
     webserver.setServerOption(8888);
    webserver.eventListen();
   
    webserver.eventLoop();
    return 0;
}