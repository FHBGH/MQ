#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include "singleton.h"
#include "requestque.h"
using namespace std;





class THandlePool
{
private:
    /* data */
    int numThread;
    vector<thread*> thread_;
public:
  
    void init(int num);
    void workThread();
    void push(mess &m);
    void pull(mess &m);
    void create(mess &m);
    void dele(mess &m);
    void others(mess &m);
};

typedef Singleton<THandlePool> tHandlePool;


