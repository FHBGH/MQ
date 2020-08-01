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
struct messInTopic {
    int len;
    char * data;
};

#pragma pack(4)
struct Head{
    uint32_t cmd;
    uint32_t ret;
    uint32_t groupId;
    bool ack;
    uint32_t offset;
    uint32_t topicL;
};
#pragma pack()
enum CMD{
    PUSH = 0,
    PULL = 1,
    CREATE = 3, 
    DELETE = 4,
    RSP = 5,
    SUBSCRIBE
};
enum statecode{
    OK = 0,
    NO_TOPIC = 1,
    TOPICED = 2,
    FAIL = 3,
    INVAILDCMD =4,
    OFFSET_OUT
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