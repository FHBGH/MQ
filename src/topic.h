#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <shared_mutex>
using namespace std;

class Topic{
public:
    void push(char *);
    void front(size_t idx);
private:
    vector<char *> partition;
    size_t offset = 0;
};

class TopicMgr{
public:
    bool hasTopic(const string &topic) {
        bool ret;
        {
            shared_lock<shared_mutex> lock(mtx);
            
            ret=(topicList.find(topic) != topicList.end());
        }
        return ret;
    }
    void createTopic(const string &topic) {
        unique_lock<shared_mutex> lock(mtx);
        if(topicList.find(topic) == topicList.end())
            topicList[topic] = new Topic;
        
    }
    Topic* get(const string& topic) {
        shared_lock<shared_mutex> lock(mtx);
        if(topicList.find(topic) != topicList.end())
            return topicList[topic];
        return NULL;
    }
    void dele(const string& topic) {
        unique_lock<shared_mutex> lock(mtx);
        if(topicList.find(topic) == topicList.end())
            return;
        delete topicList[topic];
        topicList.erase(topic);
    }

private:
    unordered_map<string,Topic*> topicList;
    shared_mutex mtx;
};