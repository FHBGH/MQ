#include "socket_service.h"
#include "requestque.h"
#include <event2/buffer.h>

int SocketService::init(int numWork) {
    numWorkThread = numWork;
    for(int i = 0;i < numWork;i++) {
        //这需不需要锁呢
        numPerThread.push_back(0);
        int* t = (int *)malloc(2*sizeof(int));
        int ret = pipe(t);
        if(ret < 0) {
            cout<<"pipe create faill"<<endl;
            return -1;
        }

        pipe_.push_back(t);
        
    }
    cout<<"socketservice init succ"<<endl;
    return 0;
        
}
int SocketService::mainThread() {
    //开起woke线程
    for(int i = 0;i < numWorkThread; i++) {
        thread_.push_back(new thread(&SocketService::workThread,this,i));
        thread_[i]->detach();
        

        cout<<"workThread "<<i<<" create succc"<<endl;
    }


    socketId = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(socketId,( sockaddr*) &servaddr,sizeof(servaddr));
    if(ret < 0) {
        cout<<"socket bind faild|return "<<endl;
        return -1;
    }
    cout<<"socket bind succ"<<endl;
    ret = listen(socketId,20);
    if(ret < 0) {
        cout<<"socket listen faild|return "<<endl;
        return -1;
    }
    cout<<"socket listen succ"<<endl;
    evutil_make_socket_nonblocking(socketId);

    struct event_base *base = event_base_new();

    if(base == NULL) {
        cout<<"base is NULL"<<endl;
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
    this->idx[tid] = id_;
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
void SocketService::do_pipe(int fd,short event,void *arg) {
    event_base* base = (event_base*)arg;
    evutil_socket_t fd_;
    struct sockaddr_in sin;
    socklen_t slen;
    int ret = 0; 
    ret = read(fd,&fd_,sizeof(fd_));
    if(ret < 0) {
        cout<<"do_pipe read fail"<<endl;
        return;
    }
    cout<<"do_pipe read succ"<<endl;
    struct bufferevent *bev = bufferevent_socket_new(base,fd_,BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev,read_cb,NULL,error_cb,(void*)&socketService::get_mutable_instance());
    bufferevent_enable(bev,EV_READ|EV_WRITE|EV_PERSIST);
    cout<<"do pipe succ"<<endl;
    
}

void SocketService::do_accept(evutil_socket_t listener,short event,void* arg) {
    SocketService* base = (SocketService *)arg;
    evutil_socket_t fd;
    struct sockaddr_in sin;
    socklen_t slen;
    fd = accept(listener,(struct sockaddr *)&sin,&slen);
    if(fd < 0) {
        cout<<"accept erro"<<endl;
        return;
    }
    if(fd > FD_SETSIZE) {
        cout<<"fd>FD_SETSIZE"<<endl;
        return;
    }
    cout<<"accept fd "<<fd<<endl;
    int idx = 0;
    int min_ = INT_MAX;
    for(int i = 0;i < base->numWorkThread;i++) {
        base->numPerThreadM.lock();
        if(base->numPerThread[i] < min_){
            min_ = base-> numPerThread[i];
            idx = i;
        }
        base->numPerThreadM.unlock();
    
    }

    base->addHasFd(fd);
    {
        boost::unique_lock<boost::shared_mutex> lock(base->bufferM);
        base->buffer[fd].offset = 0 ;
        base->buffer[fd].len = 0;
    }
    int ret = write(base->pipe_[idx][1],&fd,sizeof(evutil_socket_t));
    if( ret < 0)
        cout<<"write pipe fail "<<endl;
    cout<< "write pipe succ"<<endl;
    base->numPerThreadM.lock();
    base->numPerThread[idx]++;
    base->numPerThreadM.unlock();
    
}

void SocketService::error_cb(struct bufferevent *bev, short event, void *arg) { 
    evutil_socket_t fd = bufferevent_getfd(bev); 
    printf("fd = %u, ", fd); 
    if (event & BEV_EVENT_TIMEOUT) { 
        printf("Timed out\n"); //if bufferevent_set_timeouts() called 
    } 
    else if (event & BEV_EVENT_EOF) { 
        printf("connection closed\n");
    } 
    else if (event & BEV_EVENT_ERROR) { 
        printf("some other error\n"); 
    } 
    SocketService* th = (SocketService*)arg;
    th->deleHasFd(fd);
    bufferevent_free(bev); 
    {
        boost::unique_lock<boost::shared_mutex> lock(th->bufferM);
        th->buffer.erase(fd);
    }
    thread::id tid = this_thread::get_id();
    th->numPerThreadM.lock();
    (th->numPerThread[th->idx[tid]])--;
    th->numPerThreadM.unlock();


} 
void SocketService::read_cb(struct bufferevent *bev, void *arg) { 

    int n; 
    SocketService* th = (SocketService*)arg;
    evutil_socket_t fd = bufferevent_getfd(bev); 
    char buffer[1024];
    Buffer * buf;
     {
        boost::shared_lock<boost::shared_mutex> lock(th->bufferM);
        buf = &(th->buffer[fd]);
    }
    while(1){
        int ret;
        ret = bufferevent_read(bev,buffer, 1024);
        if(ret <= 0)
            break;

        if(buf->offset == 0 ) {
            Head* head = (Head*) buffer;
            if(ret == head->len) {
                //cout<<"inque1"<<endl;
                if( inputQue(fd,buffer,ret) !=0 ) {
                    th->release(bev);
                    return;
                }
                continue;
            }
        }  
        for(int  i = 0;i < ret; ) { 
            if(buf->offset == 0) {
                //如果剩下的自己不够包头长度
                if( ret-i < sizeof(uint32_t)) {
                    memcpy(buf->buf,buffer + i,ret - i);
                    buf->offset = ret - i;
                    break;  
                }
                Head * head = (Head*)(buffer + i);
                if(i + head->len <= ret ) {
                    if( inputQue(fd,buffer + i,head->len) !=0 ) {
                        th->release(bev);
                        return;
                    }     
                    i+=head->len;
                    continue;
                }
                else {
                    memcpy(buf->buf,buffer + i,ret - i);
                    buf->offset = ret - i;
                    //cout<<"offset"<<buf->offset<<"headlen"<<head->len<<endl;
                    buf->len = head->len;
                    //cout<<"buflen"<<buf->len<<endl;
                    break;
                }


            }
            if(buf->offset<sizeof(uint32_t)) {
                if(ret-i+buf->offset < sizeof(uint32_t)) {
                    memcpy(buf->buf + buf->offset,buffer + i,ret - i);
                    buf->offset += ret - i;
                    break;
                }
                memcpy(buf->buf + buf->offset,buffer + i,sizeof(uint32_t) - buf->offset);
                i += sizeof(uint32_t) - buf->offset;
                buf->offset = sizeof(uint32_t);
                Head* head = (Head*)buf->buf;
                buf->len = head->len;
            }
            
            //cout<<"buflen"<<buf->len<<"bufoffset"<<buf->offset<<endl;
            /*
            if(buffer[i] != '\n'|| buf->offset < buf->len -1) {
                buf -> buf[buf->offset] = buffer[i];
                buf->offset++;
                i++;
                continue; 
            }*/
            int need = buf->len - buf->offset;
            if(need <= ret) {
                memcpy(buf->buf + buf->offset,buffer + i,need);
                buf->offset += need;
                i += need;
            } 
            else {
                 memcpy(buf->buf + buf->offset,buffer + i,ret);
                 buf->offset += ret ;
                 i += ret;
                 break;
            }
            struct mess temp;
            temp.fd = fd;
            temp.len = buf->offset - 1;
            if(buf->buf[temp.len] != '\n') {
                th->release(bev);
                return;
            }
               
            char* dst = (char*) malloc( buf->offset - 1);
            memcpy(dst,buf->buf,buf->offset - 1);
            temp.dst = dst;
            buf->offset = 0;
            buf->len = 0;
             
            //cout<<"reading "<<temp.len<<" bytes "<<head->cmd<<endl;
            requestQue::get_mutable_instance().pushReq(temp);
            //i++;
                
        }
        
    }
} 

int SocketService::inputQue(evutil_socket_t fd,char* buffer , int len) { 
    struct mess temp;
    temp.fd = fd;
    // 去掉 ‘\n'
    temp.len = len - 1;
    if(buffer[len - 1] != '\n')
        return -1;
    char* dst = (char*) malloc(len - 1);
    memcpy(dst,buffer,len - 1);
    temp.dst = dst;
    //Head *head =(Head*) dst;
    //
    //cout<<"reading "<<temp.len<<" bytes "<<head->cmd<<endl;
    requestQue::get_mutable_instance().pushReq(temp);
    return 0;
}
void SocketService::release(struct bufferevent *bev) {
    evutil_socket_t fd = bufferevent_getfd(bev); 
    deleHasFd(fd);
    bufferevent_free(bev); 
    {
        boost::unique_lock<boost::shared_mutex> lock(bufferM);
        buffer.erase(fd);
    }
    thread::id tid = this_thread::get_id();
    numPerThreadM.lock();
    (numPerThread[idx[tid]])--;
    numPerThreadM.unlock();
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
    hasfd.insert(fd);
    
}
void SocketService::deleHasFd(evutil_socket_t fd) {
    boost::unique_lock<boost::shared_mutex> lock(fdM);
    hasfd.erase(fd);
}
bool SocketService::getHasFd(evutil_socket_t fd) {
    boost::shared_lock<boost::shared_mutex> lock(fdM);
    if(hasfd.find(fd) == hasfd.end())
        return NULL;
    return true;
}


int SocketService::do_rsp(evutil_socket_t fd,CMD cmd,bool ack,uint32_t ret,uint32_t  groupId,uint32_t offset,uint32_t lenT,string topic ,shared_ptr<char> data,uint32_t lenD){
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
        //cout<<"rsp len > 1024"<<endl;
        return  -1;
    }
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(head),topic.c_str(),lenT);
    memcpy(buffer+sizeof(head)+lenT,data.get(),lenD);
    int n = 0;
    bool t =  getHasFd(fd);
    if(t){
        //t->lock();
        n = write(fd,(void*) buffer,len) ;
        //->unlock();
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
