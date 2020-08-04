#include "consumer.h"

#include<unistd.h>

void handle(void * arg) {
    char* t =(char*) arg;
    cout<<t<<endl; 
    free (t);
}

int main() {
    consumer::get_mutable_instance().init("9.135.10.161:6000");
    //thread s(&Consumer::subscrip,&consumer::get_mutable_instance(),"first",true);
    consumer::get_mutable_instance().setHandle(&handle);

    consumer::get_mutable_instance().subscrip({{"first",1}}  );
    


    while(1) {
        char* t = consumer::get_mutable_instance().pop();
        cout<<t<<endl;
        free(t);
    }
    sleep(10000);
    //system("pause");
}
