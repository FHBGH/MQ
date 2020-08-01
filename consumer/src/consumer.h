#pragma once
#include <string>
#include <map>
#include <queue>
#include <mutex>
#include "zk.h"
#include "singleton.h"
#include <thread>
#include "requestque.h"
#include <condition_variable>
using namespace std;


class Consumer{
public:
    int init(string mqurl);
    int get(size_t groupId_,string topic_);
    static void event_handle_zk(const string& path,const string& new_value);
    void subscrip(size_t groupId_,string topic_);
    void autoget();
    condition_variable cv1;
    queue<char*> que;
    mutex queM;
    queue<pair<string,size_t>>  tTO;
    mutex tTOM; 
    condition_variable cv;
private:
    string mqIp;
    int mqPort = 0;
    string zkurl;
    int socketFd = 0;
    map<string,int> topicToOffset ;
    vector<size_t> groupId;
    vector<string> topic;
    mutex subM;
    mutex socketM;
    
    
};
typedef Singleton<Consumer> consumer;