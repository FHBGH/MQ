#include "consumer.h"
#include "head.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>


void Consumer::event_cb(bufferevent *bev, short event, void *arg) {
    Consumer* th = (Consumer*) arg;
    if (event & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    }
    else if (event & BEV_EVENT_ERROR) {
        printf("Some other error.\n");
    }
    else if (event & BEV_EVENT_CONNECTED) {
        printf("Client has successfully cliented.\n");
        return;
    }

    bufferevent_free(bev);

    // free event_cmd
    // need struct as event is defined as parameter
    //struct event_base *base = (struct event_base *)arg;
    event_base_free(th->base);
}
void Consumer::server_msg_cb(bufferevent *bev, void *arg) {
    Consumer * th = (Consumer*)arg;
    char msg[1024];

    size_t len = bufferevent_read(bev, msg, sizeof(msg)-1);
    //msg[len] = '\0';

    Head * head = (Head*) msg;
    if(head->cmd == RSP) {

        if(head->ret == SUBSUCC) {
            cout<<"subcrib succ"<<endl;
            size_t topicL= head->topicL;
            string topic(msg+sizeof(head),topicL);
            th -> topicToGId[topic] = head->groupId;
            head->offset = -1;
            head -> cmd = PULL;
            msg[len] = '\n';
            bufferevent_write(bev,msg,len+1);
                
        }
        else if(head ->ret == OK){
            //cout<<"get a data"<<endl;
            size_t topicL= head->topicL;
          
            //string topic(msg+sizeof(head),topicL);
            int size =len-sizeof(Head)-topicL+1;
            char *data = (char*) malloc(size);
            memcpy(data,msg+sizeof(Head)+topicL,size - 1);
            data[size-1] = '\0';
            //cout<< data<<endl;
            //th->push(data);
            th->handle(data);
            head -> cmd = PULL;
            //cout<<"push data"<<endl;
            msg[sizeof(Head)+topicL] = '\n';
            bufferevent_write(bev,msg,sizeof(Head)+topicL+1);
            //cout<<"write data"<<endl;
        }
        else if(head->ret == OFFSET_OUT) {
            head->cmd = PULL;
            //sleep(5);
            //cout<<"offset send"<<endl;
            msg[len]='\n';
            bufferevent_write(bev,msg,len+1);
        }
        else {
            cout<<"unexcept ret  "<<head->ret<<endl;
        }
    }

}
char* Consumer::pop() {
    
    unique_lock<mutex> lock(queM);
    while(que.empty()) {
        cv1.wait(lock);
    }
    char *t =que.front();
    que.pop();
    return t;
}
void Consumer::push(char* data) {
    {
        unique_lock<mutex> lock(queM);
        que.push(data);
    }
    cv1.notify_all();
}
int Consumer::init(string mqurl) {
    thread t(&Consumer::onInit,this,mqurl);
    t.detach();  
    sleep(3);
    cout<<"init detch"<<endl;
}
void Consumer::onInit(string mqurl) {
    int idx = 0;
    for(;idx<mqurl.size();idx++) {
        if(mqurl[idx] == ':')
            break;
    }
    
    mqIp=mqurl.substr(0,idx);
    mqPort = stoi(mqurl.substr(idx+1));
    
    base = event_base_new();
    bev  = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    const char * servInetAddr = mqIp.c_str();
    int servPort = mqPort;
    char buf[1024];
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    bufferevent_socket_connect(bev, (sockaddr *)&servaddr, sizeof(servaddr));
    cout<<"conneted"<<endl;
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void*)this);
    bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);
    
    event_base_dispatch(base);
    //thread g(&Consumer::autoget,this);
    //g.detach();
    cout<<"consumer init succ"<<endl;
    return ;
}

 void  Consumer::subscrip(map<string,uint32_t> temp) {
    char buffer[1024];
    cout<<"subscrip"<<endl;
    for(auto iter = temp.begin();iter != temp.end();iter++){
        Head head;
        head.cmd = SUBSCRIBE;
        head.topicL = iter->first.size();
        head.groupId = iter->second;
        
        memcpy(buffer,&head,sizeof(head));
        memcpy(buffer+sizeof(Head),iter->first.c_str(),head.topicL);
        int ret = 0;
        //socketM.lock();
        buffer[sizeof(Head)+head.topicL] = '\n';
        ret = write(bufferevent_getfd(bev),buffer,sizeof(Head)+head.topicL+1 );
        //socketM.unlock();
        if(ret == -1) {
            cout<<"write messge fail"<<endl;
            return ;
        }
        cout<<"writ succ "<<ret<<endl;
        
    }
    
 }
 
void Consumer::setHandle(void (*fun) (void * arg)) {
    Consumer::handle = fun;
}