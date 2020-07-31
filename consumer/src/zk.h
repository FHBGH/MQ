#include "zk_cpp.h"
#include "iostream"
#include "singleton.h"
using namespace std;
using namespace utility;
class ZooKeeper :public zk_cpp{
public:
    int init(std::string url,bool only_connect = true) {
        zoo_rc ret;
        ret = connect(url);
        if(ret != z_ok) {
            std::cout<<error_string(ret)<<std::endl;
            return -1;
        }
        std::cout<<"zookeeper connect succ"<<std::endl;
        if(only_connect == 1) 
            return 0;
        if( create("","0") == 0 )
            return  0;
        return -1;
    }
    int create(string path,string value) {
        zoo_rc ret =  z_ok;
        vector<zoo_acl_t> acl;
        acl.push_back(create_world_acl(zoo_perm_all));
        path = "/mq"+path;
        ret = create_persistent_node(path.c_str(),value,acl);
        if( ret == z_ok) {
            cout<<"create topic succ "<< path<<endl;
            return 0;
        }
        else {
            cout<<"create topic fail :"<<error_string(ret)<<endl;
            return -1;
        }
    }
    int set(string path,size_t offset){
        path="/mq/"+path;
        string value =to_string(offset);
        zoo_rc ret = set_node(path.c_str(),value,-1);
        if( ret == z_ok ) {
            cout<<"updata offset succ"<<endl;
            return 0;
        }
        cout<<"updata offset fail : "<<error_string(ret)<<endl;
        return -1;
    }

    int dele(string path) {
        path = "/mq/"+path;
        zoo_rc ret = delete_node(path.c_str(),-1);
        if(ret == z_ok) {
            cout<<"dele topic succ : "<<path<<endl;
            return 0;
        }
        cout<<"dele topic fail : "<<error_string(ret)<<endl;
        return -1;
    }
    int watch(string path,const data_change_event_handler_t& handler, std::string& value) {
        path="/mq/"+path;
        zoo_rc ret =  watch_data_change(path.c_str(),handler,&value);
        if(ret == z_ok) {
            cout<<"watch "<<path<<" succ"<<endl;
            return 0;
        }
        cout<<"watch topic fail : "<<error_string(ret)<<endl;
        return -1;
    }
    int exist(string path) {
        path = "/mq/" + path;
        zoo_rc ret = exists_node(path.c_str(), nullptr, true);
        if(ret == z_ok)
            return 0;
        cout<<error_string(ret)<<endl;
        return -1;
    }
    int get(string path,size_t& offset) {
        path = "/mq/" + path;
        string value;
        zoo_rc ret = get_node(path.c_str(), value, nullptr, true);
        if(ret != z_ok) {
            cout<<"get fail"<<error_string(ret)<<endl;
            return -1;
        }
        offset = stoi(value);
        return 0;

    }
};
typedef Singleton<ZooKeeper> zookeeper;