cmake_minimum_required(VERSION 2.8)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(EXECUTABLE_OUTPUT_PATH ../bin)
SET(LIBRARY_OUTPUT_DIRECTORY ../lib)
include_directories(
    .
    /usr/src/kernels/3.10.107-1-tlinux2_kvm_guest-0049/include/config/pci/
)

file(GLOB_RECURSE SRC_LIST  *.cpp *.c )
foreach(file IN LISTS SRC_LIST)
    message(STATUS ${file})
endforeach(file)
link_directories(/usr/local/lib)
link_libraries(
    libevent.so
    libevent.a
    libboost_thread.so
    libboost_system.so
    
)
add_executable(client ${SRC_LIST})

