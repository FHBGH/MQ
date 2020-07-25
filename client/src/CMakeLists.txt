cmake_minimum_required(VERSION 2.8)
SET(EXECUTABLE_OUTPUT_PATH ../bin)
SET(LIBRARY_OUTPUT_DIRECTORY ../lib)
include_directories(
    .
)

file(GLOB_RECURSE SRC_LIST  *.cpp *.c )
foreach(file IN LISTS SRC_LIST)
    message(STATUS ${file})
endforeach(file)
link_directories(/usr/local/lib)
link_libraries(
    libevent.so
    libevent.a
    
)
add_executable(client ${SRC_LIST})

