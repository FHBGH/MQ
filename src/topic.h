#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/thread/thread.hpp>
#include "requestque.h"
#include "singleton.h"
using namespace std;

class Topic{
public:
    Topic(const string & topic):name(topic){}
    void push(messInTopic m);
    messInTopic front(size_t groupId,size_t &idx);
    bool hasGroup(uint32_t groupId);
    int addGroup(uint32_t groupId);
    void upOffset(size_t groupId,size_t idx);
private:
    //vector<messInTopic> partition;
    //size_t offset = 0;
    //boost::shared_mutex mtx;
    const string name;
    map<uint32_t,size_t> groupOffset;
    map<uint32_t,mutex*> offsetMutex;
    map<uint32_t,mutex*> queMutex;
    map<uint32_t,condition_variable*> groupCv;
    map<uint32_t,vector<messInTopic>> groupQue;
    map<uint32_t,size_t> groupSize;
    boost::shared_mutex group;
};

class TopicMgr{
public:
    bool hasTopic(const string &topic);
    int createTopic(const string &topic);
    Topic* get(const string& topic);
    int dele(const string& topic);

private:
    unordered_map<string,Topic*> topicList;
    boost::shared_mutex mtx;
};
typedef Singleton<TopicMgr> topicMgr;