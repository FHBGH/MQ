
#include "requestque.h"
void RequestQue::getReq(struct mess& m) {
    {
        unique_lock<mutex> lock(queM);
        while(que.empty())
            cv.wait(lock);
        m = std::move(que.front());
        que.pop();
    }
}
void RequestQue::pushReq(struct mess& m) {
    {
        unique_lock<mutex> lock(queM);
        que.push(m);
    }
    cv.notify_all();
    
}