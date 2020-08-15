#include  "produce.h"
#include <vector>
#include <time.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional> 
#include <boost/thread/thread.hpp>
using namespace std;
char* genRandomString(int length) ;
void workThread(std::string topic ,bool ack,int  len ) ;
void test(std::string topic ,bool ack , int threadNum ,int len ) ;
void create(std::string topic,bool ack = true);
void send(std::string topic,bool ack = false );
void dele(std::string topic,bool ack);
int getList(std::string& list);
mutex coutM;
std::vector<float> count_;
mutex countM;
boost::shared_mutex mtx;
int cT = 0;
int tNum = 0;
int main(int argc, char **argv)
{ 
    std::string topic = "first";
    std::string temp = "false";
    std::string mode = "send";
    std::string threadNum = "1";   
    std::string len = "10";
    bool ack = false;
    if(argc < 2) {
        std::cout<<"usage: "<<argv[0]<<" create/test/send/delete/list topic [true/false] threadNum len"<<endl;
        return 0;
    }

    for(int i = 0;i < argc;i++) {
        if(i == 1)
            mode= std::string(argv[i]);
        else if(i == 2)
            topic = std::string(argv[i]);
        else if(i == 3)
            temp = std::string(argv[i]);
        else if(i == 4)
            threadNum = std::string(argv[i]);
        else if(i == 5)
            len = std::string(argv[i]);
    }
    if(temp == "true")
        ack = true;

    if(mode == "create") {
        create(topic,true);
        return 0;
    }
    if(mode == "test") {
        test(topic,ack,stoi(threadNum),stoi(len));
        return 0;
    }
    if(mode == "send") {
        send(topic,ack); 
        return 0;
    }
    if(mode == "delete") {
        dele(topic,ack);
        return 0;
    }
    if(mode == "list") {
        std::string list;
        int ret;
        ret = getList(list);
        if(ret != 0)
            return 0;
        cout<<"topic list : "<<list<<endl;
        return 0;
    }
    cout<<"invaild mode! ---usage: "<<argv[0]<<"create/test/send topic [true/false] threadNum len"<<endl;
    return 0;
}
void create(std::string topic,bool ack ) {
    Produce p;
    p.init("9.135.10.161:6000");
    p.create(topic,ack);
}

void send(std::string topic,bool ack ) {
    Produce p;
    p.init("9.135.10.161:6000");
    std::string str;
    while(1) {
        cin>>str;
        p.send(topic,str.c_str(),str.size(),ack);
    }
    
}
void dele(string topic,bool ack) {
    Produce p;
    p.init("9.135.10.161:6000");
    p.dele(topic,ack);
}
int getList(string& list) {
    Produce p;
    p.init("9.135.10.161:6000");
    return p.getList(list);
}

void test(string topic ,bool ack , int threadNum ,int len ) {
    tNum = threadNum;
    vector<thread*> vec(threadNum);
    for(int i = 0;i < threadNum; i++) {
        vec[i] =new thread(workThread,topic,ack,len);
    }
    for(int i = 0 ;i < threadNum; i++){
        vec[i]->join();
    }
    for(int i = 0 ;i < threadNum; i++){
        delete vec[i];
    }
    float sum = 0;
    for(int i = 0;i < threadNum; i++) {
        sum += count_[i];
    }
    cout<<"平均每个线程的发送速率（threadNum = "<<threadNum<<" , len = " <<len<<" ）： "<<sum/threadNum<<"个/ms"<<endl;

}

void workThread(string topic ,bool ack,int  len ) {
    Produce p1;
    p1.init("9.135.10.161:6000");
    sleep(1);
    int i = 0;
    char * str = genRandomString(len);
    int temp = 0;
    if(str == NULL)
        return;
    
    {
        boost::unique_lock<boost::shared_mutex> lock(mtx);
        temp = ++cT;
    }
    
    while(temp < tNum){
        boost::shared_lock<boost::shared_mutex> lock(mtx);
        temp = cT;
    }
    
    std::chrono::milliseconds begin = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
    while(i++<10000) {
        p1.send(topic,str,len,ack);
    }
    std::chrono::milliseconds end = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
    free(str);
    float speed = 10000.0/(end.count()-begin.count());
    countM.lock();
    count_.push_back(speed);
    countM.unlock();
    coutM.lock();
    cout<<"thread "<<std::this_thread::get_id()<<" speed "<< speed <<"个/ms"<<endl;
    coutM.unlock();
}
char* genRandomString(int length)  
{  
    int flag, i;  
    char* string;  
    srand((unsigned) time(NULL ));  
    if ((string = (char*) malloc(length)) == NULL )  
    {  
        printf("Malloc failed!flag:14\n");  
        return NULL ;  
    }  
  
    for (i = 0; i < length; i++)  
    {  
        flag = rand() % 3;  
        switch (flag)  
        {  
            case 0:  
                string[i] = 'A' + rand() % 26;  
                break;  
            case 1:  
                string[i] = 'a' + rand() % 26;  
                break;  
            case 2:  
                string[i] = '0' + rand() % 10;  
                break;  
            default:  
                string[i] = 'x';  
                break;  
        }  
    }  
    //string[length] = '\0';
    return string;  
} 
