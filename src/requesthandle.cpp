#include "requesthandle.h"

#include <unistd.h>
#include "socket_service.h"
#include "topic.h"
//#include "spdlog/spdlog.h"

void THandlePool::init(int num) {
    numThread = num;
    for(int i = 0;i < numThread;i++) {
        thread_.push_back(new thread(&THandlePool::workThread,this));
        //spdlog::info("THandlePool create thread succ|id = {} ",i);
        cout<<"THandlePool create thread succ id "<<i<<endl;
        thread_[i]->detach();
    }
}
void THandlePool::workThread() {
    while(1) {
        struct mess m;
        requestQue::get_mutable_instance().getReq(m);
        //cout<<"handle thread get a mess"<<endl;
        //spdlog::info("handle thread get a mess ");
        Head* head =reinterpret_cast<Head*>(m.dst);
        switch (head->cmd) {
            case PUSH:push(m);break;
            case PULL:pull(m);break;
            case CREATE:create(m);break;
            case DELETE:dele(m);break;
            case SUBSCRIBE:subscribe(m);break;
            case DELESUB:dele(m);break;
            case GETLIST:getList(m);break;
            default: others(m);
        }
    }

}
void THandlePool::push(mess &m) {
    Head* head =reinterpret_cast<Head*>(m.dst);
    uint32_t topicL = head->topicL;
    bool ack = head->ack;
    char* t =m.dst+sizeof(Head);
    string topic(t,topicL);
    int len = m.len-sizeof(Head)-topicL;
    //new 分配内存for 共享指针
    //char *data = (char*) malloc(len);
    shared_ptr<char> data(new char[len], [](char* p) { delete[] p; });
    memcpy(data.get(),m.dst+sizeof(Head)+topicL,len);
    free(m.dst);
    messInTopic mt;
    mt.len = len;
    mt.data = data;
    Topic* partion = topicMgr::get_mutable_instance().get(topic);
    if(ack == false)
    {
        if(partion == NULL) {
            cout<< "no topic" <<endl;
            //free(mt.data);
            return;
        }
        partion->push(mt);
    }
    else {
        int ret = 0;
        if(partion == NULL) {
            cout<< "no topic" <<endl;
            //free(mt.data); 
          
            ret = socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,NO_TOPIC,0,0,0,"",NULL,0);
       
            if(ret == 0) {
                //cout<<"write rsp  succ "<<endl;
                return;
            }
            cout<<"write rsp fail"<<endl;
            return ;
        
        }
        partion->push(mt);
        //cout <<"push mess to topic succ"<<endl;
        
        
        ret = socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,0,0,0,"",NULL,0);       
        if(ret == 0) {
            //cout<<"write rsp  succ "<<endl;
            return;
        }
        //cout<<"write rsp succ"<<endl;
        return;
    }
    return;
}
void THandlePool::pull(mess& m) {
    Head* head =reinterpret_cast<Head*>(m.dst);
    uint32_t topicL = head->topicL;
    string topic(m.dst+sizeof(Head),topicL);
    size_t groupId = head->groupId;
    size_t offset = head->offset;
    free(m.dst);
    Topic* T = topicMgr::get_mutable_instance().get(topic);
    int ret = 0;
   
    if(T == NULL) {
        cout<< "no topic" <<endl;
     
        ret = socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,NO_TOPIC,groupId,offset,topicL,topic,NULL,0);
    
        if(ret == 0){
            //cout<<"write rsp  succ "<<endl;
            return;
        }
        cout<<"write rsp faill"<<endl;
        return ;       
    }
    size_t idx;
    messInTopic mt = T->front(groupId,offset,idx);
    if(mt.len == 0)
    {
        //cout<<"offset out"<<endl;
        
 

        ret = socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OFFSET_OUT,groupId,offset,topicL,topic,NULL,0);
            
        if(ret == 0){
            //cout<<"write rsp  succ "<<endl;
            return;
        }
        cout<<"write rsp fail"<<endl;
        return ;
       
    }
    
    ret = socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,groupId,idx,topicL,topic,mt.data,mt.len);
    if(ret == 0){
        //cout<<"write rsp  succ "<<endl;
        return;
    }
    cout<<"write rsp fail"<<endl;
    //T->upOffset(groupId,idx);
    return ;
}

void THandlePool::create(mess& m) {
    Head* head = reinterpret_cast<Head*>(m.dst);
    uint32_t topicL = head->topicL;
    bool ack = head -> ack;
    char* t =m.dst+sizeof(Head);
    string topic(t,topicL);
    free(m.dst);
    int ret;
    ret = topicMgr::get_mutable_instance().createTopic(topic);
    if(ack == true) {
        
        if(ret == -1) {
            cout<<"create topic fail"<<endl;
            socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,TOPICED,0,0,0,"",NULL,0);
            return ;
        }
            
        if(ret == -2)
            //cout<<"create fail ,because createed a topic"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,0,0,0,"",NULL,0);
    }
    else {
        
        if(ret == -1) {
            cout<<"create topic fail"<<endl;
            return ;
        }
        if(ret == -2) 
            //cout<<"create fail ,because createed a topic"<<endl;
        return ;
        
    }

}
void THandlePool::dele(mess& m) {
    Head* head = reinterpret_cast<Head*>(m.dst);
    uint32_t topicL = head->topicL;
    bool ack = head -> ack;
    char* t =m.dst+sizeof(Head);
    string topic(t,topicL);
    free(m.dst);
    int ret = 0;
    ret = topicMgr::get_mutable_instance().dele(topic);
    if(ack == true) {
        if(ret == 0 || ret == -2){
            socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,0,0,0,"",NULL,0);
            //cout<<"dele topic succ"<<endl;
            return ;
        }
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,FAIL,0,0,0,"",NULL,0);
        cout<<"dele topic fail"<<endl;
        return ;
    }
    else {
        if(ret == 0 || ret == -2) {
            //cout<<"dele topic succ"<<endl;
            return ;
        }
        cout<<"dele topic fail"<<endl;
        return ;
    }
}
void THandlePool::others(mess& m) {
    Head* head = reinterpret_cast<Head*>(m.dst);
    free(m.dst);
    bool ack = head -> ack;
    if(ack == true) {
        cout<<"invail cmd"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,INVAILDCMD,0,0,0,"",NULL,0);
        return;
    }
    else {
        cout<<"invail cmd"<<endl;
        return;
    }
}

void THandlePool::subscribe(mess& m) {
    //cout<<"subscribe"<<endl;
    Head* head = reinterpret_cast<Head*>(m.dst);
    size_t groupId = head->groupId;
    string topic(m.dst+sizeof(Head),head->topicL);
    free(m.dst);
    Topic* T = topicMgr::get_mutable_instance().get(topic);
    if(T == NULL) {
        //no topic
        cout<<"no topic"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,NO_TOPIC,groupId,0,head->topicL,topic,NULL,0);
        return ;
    }
    int ret;
    ret = T->addGroup(groupId);
    if(ret == 0) {
        //cout<<"subsrcbe succ"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,SUBSUCC,groupId,0,head->topicL,topic,NULL,0);
        return ;
    }
    //cout<<"subsrcbed "<<endl;
    socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,SUBSUCC,groupId,0,head->topicL,topic,NULL,0);
    return ;
}
void THandlePool::deleSub(mess& m) {
    //cout<<"subscribe"<<endl;
    Head* head = reinterpret_cast<Head*>(m.dst);
    size_t groupId = head->groupId;
    string topic(m.dst+sizeof(Head),head->topicL);
    free(m.dst);
    Topic* T = topicMgr::get_mutable_instance().get(topic);
    if(T == NULL) {
        //no topic
        cout<<"no topic"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,groupId,0,head->topicL,topic,NULL,0);
        return ;
    }
    int ret;
    ret = T->subGroup(groupId);
    if(ret == 0) {
        //cout<<"subsrcbe succ"<<endl;
        socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,groupId,0,head->topicL,topic,NULL,0);
        return ;
    }
    //cout<<"subsrcbed "<<endl;
    socketService::get_mutable_instance().do_rsp(m.fd,RSP,0,OK,groupId,0,head->topicL,topic,NULL,0);
    return ;
}
void THandlePool::getList(mess& m) {
    string list;
    topicMgr::get_mutable_instance().getList(list);
    Head head;
    head.cmd = RSP;
    head.ret = OK;
    head.topicL = 0;
    head.len = sizeof(head)+list.size();
    char * dst = (char*) malloc (head.len);
    memcpy(dst,&head,sizeof(head));
    memcpy(dst+sizeof(head),list.c_str(),list.size());
    int ret = 0; 
    ret = write(m.fd,dst,head.len);
    if(ret <= 0)
        cout<<"write list fail"<<endl;
    free(dst);
}