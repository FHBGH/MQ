#include "socket_service.h"
#include"singleton.h"
#include "requesthandle.h"
#include "zk.h"



int main(){
    
    //zookeeper::get_mutable_instance().init("9.135.10.161:2181");
    tHandlePool::get_mutable_instance().init(6);
    socketService::get_mutable_instance().init(6);
    socketService::get_mutable_instance().mainThread();


}

