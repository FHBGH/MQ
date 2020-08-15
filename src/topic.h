#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/thread/thread.hpp>
#include "requestque.h"
#include "singleton.h"
#include "shm.h"
using namespace std;


class Topic{
public:
    Topic(const string & topic):name(topic) {}
    ~Topic() {
        for(auto iter = offsetMutex.begin();iter != offsetMutex.end();iter++) {
            delete iter->second;
        }
        for(auto iter = queMutex.begin();iter != queMutex.end();iter++ )
            delete iter->second;
        for(auto iter = groupCv.begin();iter != groupCv.end();iter++ )
            delete iter->second;
        for(auto iter = groupQue.begin();iter != groupQue.end();iter++ ) {
            while(!iter->second.empty())
            {
                //free(iter->second.front().data);
                iter->second.pop();
            }
        }
        for(auto iter = groupCache.begin();iter != groupCache.end();iter++) {
            //for(auto iter1 = iter->second.begin();iter1 != iter->second.end();iter1++)
                //free(iter1->second.data);
                iter->second.clear();    
        }
    }
    void push(messInTopic m);
    messInTopic front(size_t groupId,uint64_t offset,uint64_t &idx);
    bool hasGroup(uint32_t groupId);
    int addGroup(uint32_t groupId);
    void upOffset(size_t groupId,size_t idx);
    int subGroup(uint32_t groupId);
private:
    //vector<messInTopic> partition;
    //size_t offset = 0;
    //boost::shared_mutex mtx;
    const string name;
    map<uint32_t,uint64_t> groupOffset;
    map<uint32_t,mutex*> offsetMutex;
    map<uint32_t,mutex*> queMutex;
    map<uint32_t,condition_variable*> groupCv;
    map<uint32_t,queue<messInTopic>> groupQue;
    map<uint32_t,map<uint64_t,messInTopic>> groupCache;
    map<uint32_t,int> groupSub;
    //map<uint32_t,size_t> groupSize;
    boost::shared_mutex group;
};

class TopicMgr{
public:
    
    bool hasTopic(const string &topic);
    int createTopic(const string &topic);
    Topic* get(const string& topic);
    int dele(const string& topic);
    int getList(string& string);

private:
    unordered_map<string,Topic*> topicList;
    boost::shared_mutex mtx;

};
typedef Singleton<TopicMgr> topicMgr;