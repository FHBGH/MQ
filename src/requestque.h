#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "singleton.h"
#include <stdint.h>
#include <event2/event.h>
using namespace std;

struct mess{
    // int id; 线程id
    //
    evutil_socket_t  fd;
    
    //消息的长度
    int len;

    //malloc 分配的内存指针存放消息
    char* dst;
};
struct Head{
    uint32_t cmd;
    uint32_t topic;
};
enum messtype{
    PUSH = 0,
    PULL = 1
};
class RequestQue {
private:
    queue<struct mess> que;
    mutex queM;
    condition_variable cv;
    
public:
    void getReq(struct mess& m);
    void pushReq(struct mess& m);
    //只能在加锁的时候使用
    bool empty(){
        return que.empty();
    }

};
typedef Singleton<RequestQue> requestQue;