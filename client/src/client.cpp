#include  "produce.h"

int main(int argc, char **argv)
{   
    produce::get_mutable_instance().init("9.135.10.161:6000");
    produce::get_mutable_instance().create("first");
    sleep(1);
    int i=1;
    cout<< sizeof( Head)<<endl;
    string str;
    while(1) {
        //cin>>str;
        str = to_string(i++);
        
        produce::get_mutable_instance().send("first",str.c_str(),str.size(),1);
    }

}

