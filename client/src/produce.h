#pragma once
#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> /*gethostbyname function */
#include "head.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "singleton.h"
using namespace std;

class Produce {
public:
    ~Produce(){close(socketId);}
    int init(string url);
    int create(string topic,bool ack = false);
    int send(string topic,const char* data,size_t len,bool ack = false);
private:
    int socketId;
};
typedef Singleton<Produce> produce;