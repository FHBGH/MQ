#include "socket_service.h"
#include"singleton.h"
#include "requesthandle.h"







int main(int argc,char** argv){
    
    //zookeeper::get_mutable_instance().init("9.135.10.161:2181");
    int sThreadN = 6;
    int pThreadN = 6;
    if(argc > 1)
        sThreadN = atoi(argv[1]);
    if(argc > 2)
        pThreadN = atoi(argv[2]);
    tHandlePool::get_mutable_instance().init(sThreadN);
    socketService::get_mutable_instance().init(pThreadN);
    socketService::get_mutable_instance().mainThread();


}

