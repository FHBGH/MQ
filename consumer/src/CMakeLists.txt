cmake_minimum_required(VERSION 2.8)
SET(EXECUTABLE_OUTPUT_PATH ../bin)
SET(LIBRARY_OUTPUT_DIRECTORY ../lib)
include_directories(
    .
    /usr/src/kernels/3.10.107-1-tlinux2_kvm_guest-0049/include/config/pci/
    /data/hongbin/MQ/zookeeper/include
    #/data/hongbin/MQ/spdlog/include/
)

file(GLOB_RECURSE SRC_LIST  *.cpp *.c )
foreach(file IN LISTS SRC_LIST)
    message(STATUS ${file})
endforeach(file)
link_directories(
    /usr/local/lib
    /data/hongbin/MQ/zookeeper/lib
    #/data/hongbin/MQ/spdlog/lib
    )
link_libraries(
    libevent.so
    libevent.a
    #libspdlog.a
    libzookeeper_mt.a
    libboost_thread.so
    libboost_system.so
   
)
add_executable(consumer ${SRC_LIST})

