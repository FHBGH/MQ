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
    int init(string mqurl,uint32_t groupId);
    int subscrip(string topic);
    void push(char*);
    char* pop();
    int get(const string topic,char* message,int& len,uint64_t& seq);
    int deleSub(const string topic);
private:
    string mqIp;
    int mqPort = 0;
    condition_variable cv1;
    queue<char*> que;
    mutex queM;
    int socketId;
    uint32_t groupId;
    
};
//typedef Singleton<Consumer> consumer;