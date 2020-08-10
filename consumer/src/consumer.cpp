#include "consumer.h"
#include "head.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>



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
int Consumer::init(string mqurl,uint32_t groupId) {
    this->groupId = groupId;
    int idx = 0;
    for(;idx<mqurl.size();idx++) {
        if(mqurl[idx] == ':')
            break;
    }
    
    mqIp=mqurl.substr(0,idx);
    mqPort = stoi(mqurl.substr(idx+1));
    const char * servInetAddr = mqIp.c_str();
    uint32_t servPort = mqPort;
    struct sockaddr_in servaddr;
    socketId = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    if (connect(socketId, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        cout<<"connect fail"<<endl;
        return -1;
    }
    struct timeval      tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    //setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    cout<<"consumer init succ"<<endl;
    return 0;
}

int  Consumer::subscrip(string topic) {
    char buffer[1024];
    Head head;
    head.cmd = SUBSCRIBE;
    head.topicL = topic.size();
    head.groupId = groupId;
    head.len = sizeof(Head)+head.topicL+1;
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(Head),topic.c_str(),head.topicL);
    buffer[sizeof(Head)+head.topicL] = '\n';
    
    int ret = 0;
    ret = write(socketId,buffer,head.len);
    if(ret == -1) {
        cout<<"send subscrip mess fail"<<endl;
        return -1;
    }
    cout<<"subscrip mess send succ "<<endl;
    
    ret = read(socketId,buffer,1024);
    if( ret <= 0){
        cout<<"no subscrip succ rsp "<<endl;
        return -1;
    }
    Head * h = (Head*) buffer;
    if(h->cmd == RSP) {

        if(h->ret == SUBSUCC) {
            cout<<"subcrib succ"<<endl;
            return 0;      
        }
        else {
            cout<<"no expect ret or no topic"<<endl;
            return -1;
        }
    }
    else {
        cout<<"no expect rsp cmd"<<endl;
        return -1;
    }
    return 0;
    
 }

int Consumer::get(const string topic,char* message,int& len,uint64_t& seq) {
    char buffer[1024];
    Head head;
    head.cmd = PULL;
    head.topicL = topic.size();
    head.groupId = groupId;
    head.offset = seq;
    head.len = sizeof(Head)+head.topicL+1;
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(Head),topic.c_str(),head.topicL);
    buffer[sizeof(Head)+head.topicL] = '\n';
    
    int ret = 0;
    ret = write(socketId,buffer,head.len);
    if(ret == -1) {
        cout<<"send req fail"<<endl;
        return -1;
    }
    //cout<<"subscrip mess send succ "<<endl;
    
    ret = read(socketId,buffer,1024);
    if( ret <= 0){
        cout<<"no recv rsp "<<endl;
        return -1;
    }
    Head * h = (Head*) buffer;
    if(h->cmd != RSP) {
        cout<<" unexpect cmd "<<endl;
        return -1;
    }
    if(h->ret == OK) {
        size_t topicL= h->topicL;
        len =ret-sizeof(Head)-topicL;
        memcpy(message,buffer+sizeof(Head)+topicL,len);
        seq = h->offset;
        return 0;
    }
    else if(h->ret == OFFSET_OUT) {
        len = 0;
        seq = h->offset;
        return 0;
    }
    else {
        cout<<" unexcept   ret  or no topic"<<endl;
        return -1;
    }
}
int Consumer::deleSub(const string topic) {
    char buffer[1024];
    Head head;
    head.cmd = DELESUB;
    head.topicL = topic.size();
    head.groupId = groupId;
    head.len = sizeof(Head)+head.topicL+1;
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(Head),topic.c_str(),head.topicL);
    buffer[sizeof(Head)+head.topicL] = '\n';
    
    int ret = 0;
    ret = write(socketId,buffer,head.len);
    if(ret == -1) {
        cout<<"send req fail"<<endl;
        return -1;
    }
    //cout<<"subscrip mess send succ "<<endl;
    
    ret = read(socketId,buffer,1024);
    if( ret <= 0){
        cout<<"no recv rsp "<<endl;
        return -1;
    }
    Head * h = (Head*) buffer;
    if(h->cmd != RSP) {
        cout<<" unexpect cmd "<<endl;
        return -1;
    }
    if(h->ret == OK) {
        cout<<"delesub succ"<<endl;
        return 0;
    }
    else {
        cout<<" unexcept   ret  or no topic"<<endl;
        return -1;
    }
}