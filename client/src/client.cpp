#include  "produce.h"
#include <vector>
#include <time.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional> 
using namespace std;
char* genRandomString(int length) ;
void workThread(string topic ,bool ack,int  len ) ;
void test(string topic ,bool ack , int threadNum ,int len ) ;
void create(string topic,bool ack = true);
void send(string topic,bool ack = false );
mutex coutM;
vector<float> count;
mutex countM;
int main(int argc, char **argv)
{   
    /*string topic = "first";
    string temp = "false";
    bool ack = false;
    if(argc < 2) {
        cout<<"usage: "<<argv[0]<<" topic [true/false]"<<endl;
        return 0;
    }
    for(int i = 0;i < argc;i++) {
        if(i == 1)
            topic= string(argv[i]);
        if(i == 2)
            temp = string(argv[i]);
    }
    if(temp == "true")
        ack = true;
    produce::get_mutable_instance().init("9.135.10.161:6000");
    produce::get_mutable_instance().create(topic,true);
    sleep(1);
    int i=1;
    string str;
    while(0) {
        //cin>>str;
        str = to_string(i++);
        
        produce::get_mutable_instance().send(topic,str.c_str(),str.size(),ack);
    }*/

//////////////////////////test//////////////////////////////////////
    string topic = "first";
    string temp = "false";
    string mode = "send";
    string threadNum = "1";   
    string len = "10";
    bool ack = false;
    if(argc < 3) {
        cout<<"usage: "<<argv[0]<<"create/test/send topic [true/false] threadNum len"<<endl;
        return 0;
    }

    for(int i = 0;i < argc;i++) {
        if(i == 1)
            mode= string(argv[i]);
        else if(i == 2)
            topic = string(argv[i]);
        else if(i == 3)
            temp = string(argv[i]);
        else if(i == 4)
            threadNum = string(argv[i]);
        else if(i == 5)
            len = string(argv[i]);
    }
    if(temp == "true")
        ack = true;

/*
    enum MODE {
        CREATE,
        TEST,
        SEND
    };
    bind(MODE::CREATE , "create");
    bind(MODE::TEST , "test");
    bind(MODE::SEND , "send");
    int nKey = Manager::keyFromString(mode);

    switch(nKey){
        case MODE::CREATE : create(topic,true);
        case MODE::TEST :test(topic,ack,stoi(threadNum),stoi(len));break;
        case MODE::SEND :send(topic,ack);
        default : cout<<"invaild mode! ---usage: "<<argv[0]<<"create/test/send topic [true/false] threadNum len"<<endl;
    }
*/  
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
    cout<<"invaild mode! ---usage: "<<argv[0]<<"create/test/send topic [true/false] threadNum len"<<endl;
    return 0;
}
void create(string topic,bool ack ) {
    Produce p;
    p.init("9.135.10.161:6000");
    p.create(topic,ack);
}

void send(string topic,bool ack ) {
    Produce p;
    p.init("9.135.10.161:6000");
    string str;
    while(1) {
        cin>>str;
        p.send(topic,str.c_str(),str.size(),ack);
    }
    
}

void test(string topic ,bool ack , int threadNum ,int len ) {

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
        sum += count[i];
    }
    cout<<"平均每个线程的发送速率（threadNum = "<<threadNum<<" , len = " <<len<<" ）： "<<sum/threadNum<<"个/ms"<<endl;

}

void workThread(string topic ,bool ack,int  len ) {
    Produce p1;
    p1.init("9.135.10.161:6000");
    //p1.create(topic , ack);
    sleep(1);
    int i = 0;
    char * str = genRandomString(len);
    std::chrono::milliseconds begin = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
    while(i++<5000) {
        p1.send(topic,str,len,ack);
    }
    std::chrono::milliseconds end = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
    free(str);
    float speed = 5000.0/(end.count()-begin.count());
    countM.lock();
    count.push_back(speed);
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
  
    for (i = 0; i < length+1; i++)  
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
    string[length] = '\0';
    return string;  
} 
