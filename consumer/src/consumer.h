#pragma once
#include <string>
#include <map>
#include <queue>
#include <mutex>
#include "singleton.h"
#include <thread>
#include "head.h"
#include <condition_variable>
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <iostream>
using namespace std;


class Consumer{
public:
    int init(string mqurl);
    void onInit(string mqurl);
    void subscrip(map<string,uint32_t> temp);
    void push(char*);
    char* pop();
    static void event_cb(bufferevent *bev, short event, void *arg);
    static void server_msg_cb(bufferevent *bev, void *arg);
    void setHandle(void (*fun) (void * arg));
private:
    string mqIp;
    int mqPort = 0;
    condition_variable cv1;
    queue<char*> que;
    mutex queM;
    bufferevent *bev;
    event_base *base;
    map<string,uint32_t> topicToGId;
    void (* handle) (void * arg);
    
    
};
typedef Singleton<Consumer> consumer;