#include "produce.h"

int Produce::init(string url) {
    int idx = 0;
    for(;idx < url.size();idx++) {
        if(url[idx] == ':')
            break;
    }
    
    string IP = url.substr(0,idx);
    uint32_t Port = stoi(url.substr(idx+1));
    const char * servInetAddr = IP.c_str();
    uint32_t servPort = Port;
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
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    cout<<"welcome to echoclient"<<endl;
    return 0;
}
int Produce::create(string topic,bool ack) {
    char sendline[1024], recvline[1024];
    Head head;
    head.cmd=CREATE;
    head.ack=ack;
    head.topicL=topic.size();
    memcpy(sendline,&head,sizeof(Head));
    memcpy(sendline+sizeof(Head),topic.c_str(),head.topicL);
    int n = 0;
    if(ack == true) {
        write(socketId,sendline,sizeof(Head)+head.topicL);
        int cnt = 5;
        while( (n = read(socketId,recvline,1024))<0  ) {
            if(cnt == 0){
                cout<<"exit"<<endl;
                exit(-1);
            }
            write(socketId,sendline,sizeof(Head)+head.topicL);
            cnt--;
        }
        Head* rsp =(Head*) recvline;
        if(rsp->cmd==RSP){
            if(rsp->ret == OK) {
                cout<< "create topic succ : "<<topic<<endl;
                return 0;
            }  
            else if(rsp->ret == TOPICED) {
                cout<<"create topiced"<<endl;
                return 0;
            }   
            else
                cout<<"unhnwon ret"<<endl;
            return -1;
        } 
        else
        {
            cout<<"unexcept cmd"<<endl;
            return -1 ;
        }
        
    }
    else {
        write(socketId,sendline,sizeof(Head)+head.topicL);
        cout<<"send create cmd succ"<<endl;
        return 0;
    }
    return 0;
}
int Produce::send(string topic,const char* data,size_t len,bool ack ) {
    cout<< "send"<<endl;
    char sendline[1024], recvline[1024];
    Head head;
    head.cmd = PUSH;
    head.ack = ack;
    head.topicL = topic.size();
    memcpy(sendline,&head,sizeof(head));
    memcpy(sendline+sizeof(head),topic.c_str(),topic.size());
    memcpy(sendline+sizeof(head)+head.topicL,data,len);
    int n = 0;
    if(ack == true) {
        write(socketId,sendline,sizeof(Head)+head.topicL+len);
        int cnt = 5;
        while( (n = read(socketId,recvline,1024))<0  ) {
            if(cnt == 0){
                cout<<"exit"<<endl;
                exit(-1);
            }
            write(socketId,sendline,sizeof(Head)+head.topicL+len);
            cnt--;
        }
        Head* rsp =(Head*) recvline;
        if(rsp->cmd==RSP){
            if(rsp->ret == OK) {
                cout<< "send messge succ : "<<topic<<endl;
                return 0;
            }  
            else if(rsp->ret == NO_TOPIC) {
                cout<<"no topic"<<endl;
                return 0;
            }   
            else
                cout<<"unhnwon ret"<<endl;
            return -1;
        } 
        else
        {
            cout<<"unexcept cmd"<<endl;
            return -1 ;
        }
        
    }
    else {
        write(socketId,sendline,sizeof(Head)+head.topicL+len);
        //cout<<"send create cmd succ"<<endl;
        return 0;
    }
    return 0;



}