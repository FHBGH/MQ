#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "singleton.h"
#include <stdint.h>
#include <event2/event.h>
#include "head.h"
using namespace std;

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