#pragma once
#include<iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include "singleton.h"
#include <memory.h>
#include <event2/event.h> 
#include <event2/bufferevent.h> 
#include <map>
#include <thread>
#include <queue>
#include <limits.h>
#include <signal.h>
#include <mutex>
#include <condition_variable>
using namespace std;
#define PORT 6000



class Processor{
public:
    Processor(int fd ) {
        connedId = fd;
        th = new thread(process,connedId);
    }
    ~ Processor () {
        delete th;
    }

private:
static void process(int arg){
        cout<<"daole"<<"  fd"<<arg<<endl;
        while(1){};
        cout<<"process end"<<endl;
    }
    int connedId;
    thread *th;
};


class SocketService {
public:
    
    int init(int numWork);
    int mainThread();
    void workThread(int id);
    //map<int,Processor*> processThread;
    //通过这个来找线程对应的pipe_、numPerThread
    map<thread::id ,int> idx;
    int numWorkThread;
    mutex  numPerThreadM;
    vector<int> numPerThread;
    vector<int *> pipe_;
    vector<thread *> thread_;
private:
    static void do_accept(evutil_socket_t listener,short event,void *arg);
    static void error_cb(struct bufferevent *bev, short event, void *arg); 
    static void read_cb(struct bufferevent *bev, void *arg);
    static void do_pipe(int fd,short event,void *arg);
    static void signal_cb(evutil_socket_t sig,short event,void* arg);

    int socketId;
};
typedef Singleton<SocketService> socketService;