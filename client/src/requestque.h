#pragma once
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
struct Head{
    uint32_t cmd;
    bool ack;
    uint32_t ret;
    uint32_t offset;
    uint32_t topicL;
};
enum CMD{
    PUSH = 0,
    PULL = 1,
    CREATE = 3, 
    DELETE = 4,
    RSP = 5
};
enum statecode{
    OK = 0,
    NO_TOPIC = 1,
    TOPICED = 2,
    FAIL = 3,
    INVAILDCMD =4,
    OFFSET_OUT
};

