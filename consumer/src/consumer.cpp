#include "consumer.h"
#include "requestque.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>

int Consumer::init(string mqurl) {
    int idx = 0;
    for(;idx<mqurl.size();idx++) {
        if(mqurl[idx] == ':')
            break;
    }
    mqIp=mqurl.substr(0,idx);
    mqPort = stoi(mqurl.substr(idx+1));
    
    const char * servInetAddr = mqIp.c_str();
    int servPort = mqPort;
    char buf[1024];
    struct sockaddr_in servaddr;



    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    if (connect(socketFd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        cout<<"connect err consumer init fail"<<endl;
        return -1;
    }
    cout<<"consumer init succ"<<endl;
    thread g(&Consumer::autoget,this);
    g.detach();
    return 0;
       
    
}

int Consumer::get(size_t groupId_,string topic_){
        
        char buffer[1024];
        Head head;
        head.cmd = PULL;
        head.topicL = topic_.size();
        head.groupId = groupId_;
        memcpy(buffer,&head,sizeof(head));
        memcpy(buffer+sizeof(Head),topic_.c_str(),head.topicL);
        int ret = 0;
        //socketM.lock();
        ret = write(socketFd,buffer,sizeof(Head)+head.topicL);
        //socketM.unlock();
        if(ret == -1) {
            cout<<"write messge fail"<<endl;
            return -1;
        }
        //cout<<"write message succ"<<endl;
        //socketM.lock();
        ret = read(socketFd,buffer,1024);
        buffer[ret]='\0';
        //socketM.unlock();
        if(ret == -1){
            cout<<"read message fail"<<endl;
            return -1;
        }
        if(ret == 0) {
            cout<<"read a null"<<endl;
            return -1;
        }
        //cout<<"read message succ"<<endl;
        Head * h = (Head*)buffer;
        if(h->cmd != RSP||h->ret != OK) {
            //cout<<"message is err or no mess"<<endl;
           
            return -1;
        }
        
        char* t = (char*)malloc(ret-sizeof(Head)+1);
        //cout<<"data length"<<ret-sizeof(Head)<<endl;
        memcpy(t,buffer+sizeof(Head),ret-sizeof(Head)+1);
        cout<<t<<endl;
        //sleep(1000);
        free(t);
        //cout<<t<<endl;
        //{
        //    unique_lock<mutex> lock(queM);

        //    que.push(t);

        //}
        //cv1.notify_all();
        return 0;

}
void Consumer::event_handle_zk(const string& path,const string& new_value) {
    cout<<"huijiaohanshu"<<endl;
    string topic(path,4);
    size_t offset = stoi(new_value);
    consumer::get_mutable_instance().tTOM.lock();
    consumer::get_mutable_instance().tTO.push(pair<string,size_t>(topic,offset));
    consumer::get_mutable_instance().tTOM.unlock();
    consumer::get_mutable_instance().cv.notify_all();
    string value;
    zookeeper::get_mutable_instance().watch(topic,Consumer::event_handle_zk,value);
}
 
 
 
 
 void  Consumer::subscrip(size_t groupId_,string topic_) {
    char buffer[1024];
    Head head;
    head.cmd = SUBSCRIBE;
    head.topicL = topic_.size();
    head.groupId = groupId_;
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(Head),topic_.c_str(),head.topicL);
    int ret = 0;
    //socketM.lock();
    ret = write(socketFd,buffer,sizeof(Head)+head.topicL);
    //socketM.unlock();
    if(ret == -1) {
        cout<<"write messge fail"<<endl;
        return ;
    }
    //cout<<"write message succ"<<endl;
    //socketM.lock();
    ret = read(socketFd,buffer,1024);
    //socketM.unlock();
    if(ret == -1){
        cout<<"read message fail"<<endl;
        return ;
    }
    if(ret == 0) {
        cout<<"read a null"<<endl;
        return ;
    }
    //cout<<"read message succ"<<endl;
    Head * h = (Head*)buffer;
    if(h->cmd != RSP||h->ret != OK) {
        cout<<"subscrip fail"<<endl;
        return ;
    }
    cout<<"subscrip succc"<<endl;
    unique_lock<mutex> lock(subM);
    groupId.push_back(groupId_);
    topic.push_back(topic_);
 }
 
void Consumer::autoget() {
    while(1){
        unique_lock<mutex> lock(subM);
        for(int i=0;i< topic.size();i++){
            get(groupId[i],topic[i]);
        }
    }  
}