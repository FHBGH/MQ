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
    messInTopic front(size_t idx);
private:
    vector<messInTopic> partition;
    size_t offset = 0;
    boost::shared_mutex mtx;
    const string name;
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