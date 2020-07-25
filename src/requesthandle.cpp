#include "requesthandle.h"
#include "requestque.h"
#include <unistd.h>
//#include "spdlog/spdlog.h"

void THandlePool::init(int num) {
    numThread=num;
    for(int i = 0;i < numThread;i++) {
        thread_.push_back(new thread(&THandlePool::workThread,this));
        //spdlog::info("THandlePool create thread succ|id = {} ",i);
        cout<<"THandlePool create thread succ id "<<i<<endl;
        thread_[i]->detach();
    }
}
void THandlePool::workThread(){
    while(1) {
        struct mess m;
        requestQue::get_mutable_instance().getReq(m);
        cout<<"handle thread get a mess"<<endl;
        //spdlog::info("handle thread get a mess ");
        Head* head =reinterpret_cast<Head*>(m.dst);
        if(head->cmd == 0) {
            cout<<"processing"<<endl;
            //spdlog::info("processing");
            sleep(1);
            write(m.fd,"recved ok\n",10);
        }
        else {
            cout<<head->cmd<<endl;
            //spdlog::info("cmd = {}",head->cmd);
            write(m.fd,"ok",2);

        }

    }
}