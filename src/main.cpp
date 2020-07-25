#include "socket_service.h"
#include"singleton.h"
#include "requesthandle.h"
//#include "spdlog/spdlog.h"
//#include "spdlog/async.h"
//#include "spdlog/sinks/basic_file_sink.h"
//#include "NetDataLog.h"



int main(){
    //netDataLoger::get_mutable_instance.init("../log");
    //auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    //spdlog::set_default_logger(logger);
    //spdlog::flush_on(spdlog::level::info);
    //spdlog::flush_on(spdlog::level::err);
    //spdlog::info("start");
   
    tHandlePool::get_mutable_instance().init(3);
    socketService::get_mutable_instance().init(3);
    socketService::get_mutable_instance().mainThread();
    //spdlog::shutdown();

}

