#include "consumer.h"


int main() {
    consumer::get_mutable_instance().init("9.135.10.161:6000");
    //thread s(&Consumer::subscrip,&consumer::get_mutable_instance(),"first",true);
    consumer::get_mutable_instance().subscrip(1,"first");
    while(1) {
        char* t;
          //cout<<"qushuju"<<endl;
            unique_lock<mutex> lock(consumer::get_mutable_instance().queM);
            //while(consumer::get_mutable_instance().que.empty()) {
            //    consumer::get_mutable_instance().cv1.wait(lock);
           // }
           if(consumer::get_mutable_instance().que.empty())
                continue;
            t = consumer::get_mutable_instance().que.front();
            consumer::get_mutable_instance().que.pop();
        
        cout<<t<<endl;
        free(t);

    }
}
