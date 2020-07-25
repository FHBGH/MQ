#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include "singleton.h"
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

};

typedef Singleton<THandlePool> tHandlePool;


