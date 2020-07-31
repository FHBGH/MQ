#include "consumer.h"
#include "requestque.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>

int Consumer::init(string mqurl,string zkurl) {
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
    int ret = 0;
    ret = zookeeper::get_mutable_instance().init(zkurl);
    if(ret == -1) { 
        cout<<"zookeeper init err consumer init fail"<<endl;
        return -1;
    }
    cout<<"consumer init succ"<<endl;
    thread g(&Consumer::autoget,this);
    g.detach();
    return 0;
       
    
}

int Consumer::get(string topic,size_t offset){
        
        char buffer[1024];
        Head head;
        head.cmd = PULL;
        head.offset = offset;
        head.topicL = topic.size();
        memcpy(buffer,&head,sizeof(head));
        memcpy(buffer+sizeof(Head),topic.c_str(),head.topicL);
        int ret = 0;
        //socketM.lock();
        ret = write(socketFd,buffer,sizeof(Head)+head.topicL);
        //socketM.unlock();
        if(ret == -1) {
            cout<<"write messge fail"<<endl;
            return -1;
        }
        cout<<"write message succ"<<endl;
        //socketM.lock();
        ret = read(socketFd,buffer,1024);
        buffer[ret] = '\0';
        //socketM.unlock();
        if(ret == -1){
            cout<<"read message fail"<<endl;
            return -1;
        }
        if(ret == 0) {
            cout<<"read a null"<<endl;
            return -1;
        }
        cout<<"read message succ"<<endl;
        Head * h = (Head*)buffer;
        if(h->cmd != RSP||h->ret != OK) {
            cout<<"message is err"<<endl;
            return -1;
        }
        
        char* t = (char*)malloc(ret-sizeof(Head)+1);
        //cout<<"data length"<<ret-sizeof(Head)<<endl;
        memcpy(t,buffer+sizeof(Head),ret-sizeof(Head)+1);
        //cout<<t<<endl;
        {
            unique_lock<mutex> lock(queM);

            que.push(t);

        }
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
 void  Consumer::subscrip(string topic,bool from_begin) {
    int ret = 0;
    ret = zookeeper::get_mutable_instance().exist(topic);
    if(ret == -1)
        return ;
    size_t offset;
    ret = zookeeper::get_mutable_instance().get(topic,offset);
    if(ret == -1)
    {
        cout<<"get fail"<<endl;
        return ;
    }
        
    cout<<"offset"<<offset<<endl;
    //topicToOffset[topic] = offset ;
    if(from_begin) {
        int i = 0;
        while(i < offset) {
            int r = 0;
            r = get(topic,i);
            if(r == -1)
                return ;
            i++;
            topicToOffset[topic] = i;

        }
        string value;
        zookeeper::get_mutable_instance().watch(topic,Consumer::event_handle_zk,value);
        return ;
    }
    topicToOffset[topic] = offset ;
    string value;
    zookeeper::get_mutable_instance().watch(topic,Consumer::event_handle_zk,value);
    return ;
 }
 void Consumer::autoget() {
     while(1){
        pair<string,size_t> task;
        {
            unique_lock<mutex> lock(tTOM);
            while(tTO.empty())
                cv.wait(lock);
            task = tTO.front();
            tTO.pop();
        }
        for(int i = topicToOffset[task.first];i<task.second;) {
            int ret = 0;
            ret = get(task.first,i);
            if(ret == -1){ 
                cout<< "get fail"<<endl;
                break;
            }
                
            i++;
            topicToOffset[task.first] =  i;

        }
    }
 }