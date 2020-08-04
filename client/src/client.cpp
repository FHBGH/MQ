#include  "produce.h"

int main(int argc, char **argv)
{   
    produce::get_mutable_instance().init("9.135.10.161:6000");
    produce::get_mutable_instance().create("first");
    while(1) {
        string str;
        cin>>str;
        produce::get_mutable_instance().send("first",str.c_str(),str.size());
    }

}

