#include "topic.h"
#include "zk.h"

void Topic::push(messInTopic m) {
    boost::unique_lock<boost::shared_mutex> lock(mtx);
    partition.push_back(m);
    offset++;
    //zookeeper::get_mutable_instance().set(name,offset);
}
messInTopic  Topic::front(size_t groupId,size_t &idx) {
    boost::shared_lock<boost::shared_mutex> lock(mtx);
    messInTopic t;
    t.len = 0;
    boost::shared_lock<boost::shared_mutex> lock1(group);
    unique_lock<mutex> lock2(*(groupMutex[groupId]));
    //groupMutex[groupId]->lock();
    if(groupOffset[groupId]>=offset)
        return t;
    idx = groupOffset[groupId]++;
    //groupMutex[groupId]->unlock();
    return partition[idx];
}
void Topic::upOffset(size_t groupId,size_t idx) {
    boost::shared_lock<boost::shared_mutex> lock(group);
    unique_lock<mutex> lock1(*(groupMutex[groupId]));
    //groupMutex[groupId]->lock();
    groupOffset[groupId] = idx;
    //groupMutex[groupId]->unlock();
}

bool Topic::hasGroup(uint32_t groupId) {
    unique_lock<boost::shared_mutex> lock(group);
    if(groupOffset.find(groupId) == groupOffset.end())
        return false;
    return true;
}
int Topic::addGroup(uint32_t groupId) {
    unique_lock<boost::shared_mutex> lock(group);
    if(groupOffset.find(groupId) != groupOffset.end())
        return -1;
    groupOffset[groupId] = 0;

    groupMutex[groupId] = new mutex;
    return 0;
    
}

bool TopicMgr::hasTopic(const string &topic){
    bool ret;
    {
        boost::shared_lock<boost::shared_mutex> lock(mtx);
        
        ret=(topicList.find(topic) != topicList.end());
    }
    return ret;
}
int TopicMgr::createTopic(const string &topic) {
    boost::unique_lock<boost::shared_mutex> lock(mtx);
    if(topicList.find(topic) == topicList.end())
    {   int ret = 0;
        //ret = zookeeper::get_mutable_instance().create("/"+topic,"0");
        //if(ret != 0)
        //    return -1;
        topicList[topic] = new Topic(topic);
        cout<<"topiclist add topic succ"<<endl;
        return 0; 
    }
    return -2;
}
Topic* TopicMgr::get(const string& topic) {
    boost::shared_lock<boost::shared_mutex> lock(mtx);
    if(topicList.find(topic) != topicList.end())
        return topicList[topic];
    return NULL;
}
int TopicMgr::dele(const string& topic) {
    boost::unique_lock<boost::shared_mutex> lock(mtx);
    if(topicList.find(topic) == topicList.end())
        return -2;
    int ret = 0; 
    //ret = zookeeper::get_mutable_instance().dele(topic);
    //if(ret == -1)
    //    return -1;
    delete topicList[topic];
    topicList.erase(topic);
    return 0;
}