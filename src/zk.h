#include "zk_cpp.h"
#include "iostream"
#include "singleton.h"
using namespace std;
using namespace utility;
class ZooKeeper :public zk_cpp{
public:
    int init(std::string url) {
        zoo_rc ret;
        ret = connect(url);
        if(ret != z_ok) {
            std::cout<<error_string(ret)<<std::endl;
            return -1;
        }
        std::cout<<"zookeeper connect succ"<<std::endl;
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
};
typedef Singleton<ZooKeeper> zookeeper;