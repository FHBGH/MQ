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
    head.len = sizeof(Head)+head.topicL+1;
    memcpy(sendline,&head,sizeof(Head));
    memcpy(sendline+sizeof(Head),topic.c_str(),head.topicL);
    sendline[sizeof(Head)+head.topicL] = '\n';
    int n = 0;
    if(ack == true) {
        int ret = 0;
        ret = write(socketId,sendline,head.len);
        if(ret == 0) {
            cout<<"connnect closed"<<endl;
            exit(-1);
        }
        int cnt = 5;
        while( (n = read(socketId,recvline,1024))<0  ) {
            if(cnt == 0){
                cout<<"exit"<<endl;
                exit(-1);
            }
            write(socketId,sendline,head.len);
            cnt--;
        }
        if(n == 0) {
            cout<<"connect closed"<<endl;
            exit(-1);
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
        write(socketId,sendline,head.len);
        cout<<"send create cmd succ"<<endl;
        return 0;
    }
    return 0;
}
int Produce::send(string topic,const char* data,size_t len,bool ack ) {
    char sendline[1024], recvline[1024];
    Head head;
    head.cmd = PUSH;
    head.ack = ack;
    head.topicL = topic.size();
    head.len = sizeof(Head)+head.topicL+len+1;
    memcpy(sendline,&head,sizeof(head));
    memcpy(sendline+sizeof(head),topic.c_str(),topic.size());
    memcpy(sendline+sizeof(head)+head.topicL,data,len);
    sendline[sizeof(Head)+head.topicL+len] = '\n';
    
    int n = 0;
    if(ack == true) {
        int ret = 0;
        ret = write(socketId,sendline,head.len);
        if(ret == 0) {
            cout<<"connnect closed"<<endl;
            exit(-1);
        }
        int cnt = 5;
        while( (n = read(socketId,recvline,1024))<0  ) {
            if(cnt == 0){
                cout<<"exit"<<endl;
                exit(-1);
            }
            write(socketId,sendline,head.len);
            cnt--;
        }
        if(n == 0) {
            cout<<"connect closed"<<endl;
            exit(-1);
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
        int ret = 0;
        while( write(socketId,sendline,head.len) != head.len) {

        }
        return 0;
    }
    return 0;



}
int Produce::dele(string topic,bool ack) {
    char sendline[1024], recvline[1024];
    Head head;
    head.cmd=DELETE;
    head.ack=ack;
    head.topicL=topic.size();
    head.len = sizeof(Head)+head.topicL+1;
    memcpy(sendline,&head,sizeof(Head));
    memcpy(sendline+sizeof(Head),topic.c_str(),head.topicL);
    sendline[sizeof(Head)+head.topicL] = '\n';
    int n = 0;
    if(ack == true) {
        int ret = 0;
        ret = write(socketId,sendline,head.len);
        if(ret == 0) {
            cout<<"connnect closed"<<endl;
            exit(-1);
        }
        int cnt = 5;
        while( (n = read(socketId,recvline,1024))<0  ) {
            if(cnt == 0){
                cout<<"exit"<<endl;
                exit(-1);
            }
            write(socketId,sendline,head.len);
            cnt--;
        }
        if(n == 0) {
            cout<<"connect closed"<<endl;
            exit(-1);
        }
        Head* rsp =(Head*) recvline;
        if(rsp->cmd==RSP){
            if(rsp->ret == OK) {
                cout<< "delete topic succ : "<<topic<<endl;
                return 0;
            }  
            else if(rsp->ret == FAIL) {
                cout<<"delete fail"<<endl;
                return -1;
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
}
int Produce::getList(string& list) {
    char sendline[1024], recvline[1024];
    Head head;
    head.cmd=GETLIST;
    head.len = sizeof(Head)+1;
    memcpy(sendline,&head,sizeof(Head));
    sendline[sizeof(Head)] = '\n';

    int ret = 0;
    ret = write(socketId,sendline,head.len);
    if(ret == 0) {
        cout<<"connnect closed"<<endl;
        exit(-1);
    }
       
    ret = read(socketId,recvline,1024);
           
    if(ret <= 0) {
        cout<<"connect closed"<<endl;
        exit(-1);
    }
    Head* rsp =(Head*) recvline;
    if(rsp->cmd==RSP){
        if(rsp->ret == OK) {
            list = string(recvline+sizeof(Head),rsp->len - sizeof(Head));
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
