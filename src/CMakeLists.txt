cmake_minimum_required(VERSION 2.8)
SET(EXECUTABLE_OUTPUT_PATH ../bin)
SET(LIBRARY_OUTPUT_DIRECTORY ../lib)
include_directories(
    .
    /usr/src/kernels/3.10.107-1-tlinux2_kvm_guest-0049/include/config/pci/
    #../spdlog/include/
)

file(GLOB_RECURSE SRC_LIST  *.cpp *.c )
foreach(file IN LISTS SRC_LIST)
    message(STATUS ${file})
endforeach(file)
link_directories(
    /usr/local/lib
    #../spdlog/lib
    )
link_libraries(
    libevent.so
    libevent.a
    #libspdlog.a
)
add_executable(mq ${SRC_LIST})

