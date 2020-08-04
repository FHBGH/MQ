#include "socket_service.h"
#include "requestque.h"
//#include "spdlog/spdlog.h"



int SocketService::init(int numWork) {
    numWorkThread = numWork;
    for(int i=0;i<numWork;i++) {
        //这需不需要锁呢
        numPerThread.push_back(0);
        int* t = (int *)malloc(2*sizeof(int));
        int ret = pipe(t);
        if(ret < 0) {
            cout<<"pipe create faill"<<endl;
            //spdlog::error("pipe create faill|ret = {}",ret);
            return -1;
        }

        pipe_.push_back(t);
        
    }
    cout<<"socketservice init succ"<<endl;
    //spdlog::info("socketservice init succ");
    return 0;
        
}
int SocketService::mainThread() {
    //开起woke线程
    for(int i = 0;i < numWorkThread; i++) {
        thread_.push_back(new thread(&SocketService::workThread,this,i));
        thread_[i]->detach();
        

        cout<<"workThread "<<i<<" create succc"<<endl;
        //spdlog::info("workThread {} createsucc!!!",i);
        //thread::id tid = this_thread::get_id();
        //this->idx[tid]=i;
    }


    socketId = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(socketId,( sockaddr*) &servaddr,sizeof(servaddr));
    if(ret<0) {
        string err="socket bind faild|return "+to_string(ret);
        cout<<err<<endl;
        //spdlog::error("socket bind faild|ret {}",ret);
        return -1;
    }
    cout<<"socket bind succ"<<endl;
    //spdlog::info("socket bind succ!!!");
    ret = listen(socketId,20);
    if(ret<0) {
        string err="socket listen faild|return "+to_string(ret);
        cout<<err<<endl;
        //spdlog::error("socket listen faild|ret {}",ret);
        return -1;
    }
    cout<<"socket listen succ"<<endl;
    //spdlog::info("socket listen succ!!!");
    evutil_make_socket_nonblocking(socketId);

    struct event_base *base = event_base_new();

    if(base == NULL) {
        cout<<"base is NULL"<<endl;
        //spdlog::error("base is NULL|ret = {}",-1);
        return -1;
    }

    struct event* listen_event;
    listen_event = event_new(base,socketId,EV_READ|EV_PERSIST,do_accept,(void*)this);
    event_add(listen_event,NULL);
    struct event* signal_event;
    signal_event = evsignal_new(base,SIGINT,signal_cb,(void*)base);
    event_add(signal_event,NULL);
    event_base_dispatch(base);
    event_free(listen_event);
    event_free(signal_event);
    event_base_free(base);
    return 0;
}

void SocketService::workThread(int id) {
    int id_ = id;
    //线程自己的事件循环
    thread::id tid = this_thread::get_id();
    this->idx[tid]=id_;
    struct event_base *base = event_base_new();
    if(base == NULL) {
        cout<<"base NULL"<<endl;
        //spdlog::error("worktread {} base is NULL",id_);
        return;
    }

    struct event* pipeEvent;
    pipeEvent = event_new(base,pipe_[id_][0],EV_READ|EV_PERSIST,do_pipe,(void*)base);
    event_add(pipeEvent,NULL);
    //struct event* signal_event;
    //signal_event = evsignal_new(base,SIGINT,signal_cb,(void*)base);
    //event_add(signal_event,NULL);
    event_base_dispatch(base);
    //event_free(pipeEvent);
    //event_free(signal_event);
    //event_base_free(base);
}
void SocketService::do_pipe(int fd,short event,void *arg){
    event_base* base = (event_base*)arg;
    evutil_socket_t fd_;
    struct sockaddr_in sin;
    socklen_t slen;
    int ret = 0; 
    ret = read(fd,&fd_,sizeof(fd_));
    if(ret < 0) {
        cout<<"do_pipe read fail"<<endl;
        return;
        //spdlog::error("do_pipe read faile|ret = {}",ret);
    }
    cout<<"do_pipe read succ"<<endl;
    //spdlog::info("do_pipe read succ");

    struct bufferevent *bev = bufferevent_socket_new(base,fd_,BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev,read_cb,NULL,error_cb,(void*)&socketService::get_mutable_instance());
    bufferevent_enable(bev,EV_READ|EV_WRITE|EV_PERSIST);
    cout<<"do pipe succ"<<endl;
    
}

void SocketService::do_accept(evutil_socket_t listener,short event,void* arg) {
    SocketService* base=(SocketService *)arg;
    evutil_socket_t fd;
    struct sockaddr_in sin;
    socklen_t slen;
    fd = accept(listener,(struct sockaddr *)&sin,&slen);
    if(fd < 0) {
        cout<<"accept erro"<<endl;
        //spdlog::error("accept erro|fd = {}",fd);
        return;
    }
    if(fd > FD_SETSIZE) {
        cout<<"fd>FD_SETSIZE"<<endl;
        //spdlog::error("fd>FD_SETSIZE|fd = {}",fd);
        return;
    }
    cout<<"accept fd "<<fd<<endl;
    //spdlog::info("accept fd = {}",fd);
    int idx = 0;
    int min_= INT_MAX;
    for(int i = 0;i < base->numWorkThread;i++) {
        base->numPerThreadM.lock();
        if(base->numPerThread[i] < min_){
            min_ =base-> numPerThread[i];
            idx = i;
        }
        base->numPerThreadM.unlock();
    
    }

    socketService::get_mutable_instance().addHasFd(fd);
    

    int ret = write(base->pipe_[idx][1],&fd,sizeof(evutil_socket_t));
    if( ret < 0)
        cout<<"write pipe fail "<<endl;
        //spdlog::error("write pipe fail|ret = {}",ret);
    cout<< "write pipe succ"<<endl;
    //spdlog::info("write pipe succ");
    base->numPerThreadM.lock();
    base->numPerThread[idx]++;
    base->numPerThreadM.unlock();
    
}

void SocketService::error_cb(struct bufferevent *bev, short event, void *arg) { 
    evutil_socket_t fd = bufferevent_getfd(bev); 
    printf("fd = %u, ", fd); 
   // spdlog::info("fd = {}",fd);
    if (event & BEV_EVENT_TIMEOUT) { 
        printf("Timed out\n"); //if bufferevent_set_timeouts() called 
        //spdlog::info("Timeid out");
    } 
    else if (event & BEV_EVENT_EOF) { 
        printf("connection closed\n");
        //spdlog::info("connection closed");
    } 
    else if (event & BEV_EVENT_ERROR) { 
        printf("some other error\n"); 
        //spdlog::info("some other error");
    } 
    SocketService* th = (SocketService*)arg;
    th->deleHasFd(fd);
    bufferevent_free(bev); 
    
    thread::id tid = this_thread::get_id();
    th->numPerThreadM.lock();
    (th->numPerThread[th->idx[tid]])--;
    th->numPerThreadM.unlock();


} 
void SocketService::read_cb(struct bufferevent *bev, void *arg) { 

    int n; 
    //SocketService* th = (SocketService*)arg;
    evutil_socket_t fd = bufferevent_getfd(bev); 
    //这里要去实现将请求写入请求队列
    char* dst = (char*)malloc(1024);
    cout<<"reading"<<endl;
    //spdlog::info("reading");
    if(n = bufferevent_read(bev, dst, 1024), n > 0) { 
        char *t = (char*)malloc(n);
        memcpy(t,dst,n);
        struct mess temp;
        temp.fd = fd;
        temp.len = n;
        temp.dst = t;
        //push 
        requestQue::get_mutable_instance().pushReq(temp);
       // spdlog::info("stroe succ");
        //cout<< "stroe succ"<<endl;
        //bufferevent_write(bev,  "store succ" , 10); 
    } 
    char temp[1];
    if(n = bufferevent_read(bev, temp , 1),n !=0 ) {
        bufferevent_write(bev,  "data biger" , 10); 
        //spdlog::info("data biger");
        cout<<"data biger"<<endl;
    }
} 

void  SocketService::signal_cb(evutil_socket_t sig,short event,void* arg) {
    struct event_base *base = (struct event_base *)arg;
    struct timeval delay = {2,0};

    cout<<"catch a interrupt signal;exiting cleanly in two seconds"<<endl;
    //spdlog::info("catch a interrupt signal;exiting cleanly in two seconds");
    event_base_loopexit(base,&delay);
}

void  SocketService::addHasFd(evutil_socket_t fd) {
    
    boost::unique_lock<boost::shared_mutex> lock(fdM);
    hasfd[fd]=new mutex; 
    
}
void SocketService::deleHasFd(evutil_socket_t fd) {
    boost::unique_lock<boost::shared_mutex> lock(fdM);
    delete hasfd[fd];
    hasfd.erase(fd);
}
mutex* SocketService::getHasFd(evutil_socket_t fd) {
    boost::shared_lock<boost::shared_mutex> lock(fdM);
    if(hasfd.count(fd) == 0)
        return NULL;
    return hasfd[fd];
}


int SocketService::do_rsp(evutil_socket_t fd,CMD cmd,bool ack,uint32_t ret,uint32_t  groupId,uint32_t offset,uint32_t lenT,string topic ,char* data,uint32_t lenD){
    char buffer[1024];
    Head head;
    head.cmd = cmd;
    head.ack = ack;
    head.ret = ret;
    head.groupId = groupId;
    head.offset = offset;
    head.topicL = lenT;
    int len= sizeof(head)+lenT+lenD;
    if(len > 1024) {
        cout<<"rsp len > 1024"<<endl;
        return  -1;
    }
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(head),topic.c_str(),lenT);
    memcpy(buffer+sizeof(head)+lenT,data,lenD);
    int n = 0;
    mutex * t =  getHasFd(fd);
    if(t){
        t->lock();
        n = write(fd,(void*) buffer,len) ;
        t->unlock();
        if( n == len) {
        
            return 0;
        }
        if( n < 0 ) {
            
            return -1;
        }
        if( n < len) {
            
            return -2;
        }
        return -3;
    }

    return -4;

}
//////////////////////////////////////resquestque////////////////////////////////////
