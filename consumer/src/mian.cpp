#include "consumer.h"
#include<unistd.h>
#include <time.h>
#include <thread>
#include <chrono>
mutex coutM;
vector<float> sum;
mutex sumM;
void consumer(string url , uint32_t groupId  ,string topic);
void workThread(string url , uint32_t groupId  ,string topic);
void test(string url , uint32_t groupId  ,string topic,int threadNum,string mode);
void sub(string url ,string topic ,uint32_t groupId);
int getList(string& list);
int main(int argc,char** argv) {
    if(argc < 2) {
        cout<<"usage: "<<argv[0]<<" mode groupId topic threadNum     (ps:每个消费者只能订阅只能用同一个身份订阅主题一次，fail: 1 topic1 2 topic1)"<<endl;
        return -1;
    }
    string mode(argv[1]);
    if(mode == "list") {
        string list;
        int ret;
        ret = getList(list);
        if(ret != 0)
            return 0;
        cout<<"topic list : "<<list<<endl;
        return 0;
    }
    string topic(argv[3]);
    uint32_t groupId = atoi( argv[2]);
    
    int threadNum = 1; 
    if(argc >= 5) 
        threadNum = atoi(argv[4]);
    string url ="9.135.10.161:6000";
    if(mode == "sub") {
        sub(url,topic ,groupId);
        return 0;
    }
    test(url,groupId,topic,threadNum,mode);
    return 0;
}
void sub(string url ,string topic,uint32_t groupId) {
    Consumer c1;
    int ret = 0;
    ret = c1.init(url,groupId);
    if(ret < 0)
        return;
    ret = c1.subscrip(topic);
    if(ret < 0)
        return;
    return ;
}
int getList(string& list) {
    Consumer c;
    c.init("9.135.10.161:6000",0);
    return c.getList(list);
}
void test(string url , uint32_t groupId  ,string topic,int threadNum,string mode) {
    //cout<<"test"<<endl;
    vector<thread*> vec(threadNum);
    if(mode == "test") {
        cout<<"loading ........ "<<endl;
        for(int i = 0;i < threadNum; i++) {
            //cout<<"thread start"<<endl;
            vec[i] =new thread(workThread,url,groupId,topic);
        }
    }
    else if(mode == "recv")
        for(int i = 0;i < threadNum; i++) {
            vec[i] =new thread(consumer,url,groupId,topic);
        }
    else {
        cout<< "erro mode"<<endl;
        return;
    }

    for(int i = 0 ;i < threadNum; i++){
        vec[i]->join();
    }
    for(int i = 0 ;i < threadNum; i++){
        delete vec[i];
    }
    float summ = 0;
    for(int i = 0 ;i < threadNum; i++) {
        summ+=sum[i];
        cout<<"thread "<<i<<" speed "<< sum[i] <<"个/ms"<<endl;
    }
    cout<<"平均每个线程的接收速率（threadNum = "<<threadNum<<" ）： "<<summ/threadNum<<"个/ms"<<endl;
}

void workThread(string url , uint32_t groupId  ,string topic) {
    Consumer c1;
    int ret = 0;
    ret = c1.init(url,groupId);
    if(ret < 0)
        return;
    //ret = c1.subscrip(topic);
    //if(ret < 0)
    //    return;
    uint64_t seq = -1;
    char buffer[1024];
    int len = 1024;
    int count = 0;
    std::chrono::milliseconds begin; 
    while(1) {
        ret = c1.get(topic,buffer,len,seq);
        if(ret < 0)
            return;
        if(len != 0) {
            count++; 
            if(count == 1) {
                begin = std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::system_clock::now().time_since_epoch()
                );
            }
            else if(count == 51) {
                std::chrono::milliseconds end = std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::system_clock::now().time_since_epoch()
                ) ;
                float speed = float(count-1)/(end.count()-begin.count());
                sumM.lock();
                sum.push_back(speed);
                sumM.unlock();
                return;
            }
        }
        else if(len == 0 && count > 10) {
            
            return;
        }
    }
}
void consumer(string url , uint32_t groupId  ,string topic) {
    Consumer c1;
    int ret = 0;
    ret = c1.init(url,groupId);
    if(ret < 0)
        return;
    ret = c1.subscrip(topic);
    if(ret < 0)
        return;
    uint64_t seq = -1;
    char buffer[1024];
    int len = 1024;
    while(1) {
        ret = c1.get(topic,buffer,len,seq);
        if(ret < 0)
            return;
        if(len != 0) {
            buffer[len] = '\0';
            coutM.lock();
            cout<<buffer<<endl; 
            coutM.unlock();
        }
    }
}