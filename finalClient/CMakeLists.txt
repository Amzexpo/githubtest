cmake_minimum_required(VERSION 2.8)

project(testclient)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories($ENV{PATH})
link_directories($ENV{LD_LIBRARY_PATH})

set(SOURCES client.cpp)


# add_executable(server "main.cpp" "Server.h" "Session.h" "Server.cpp" "Session.cpp" main.cpp Server.h Session.h Server.cpp Session.cpp)
add_executable(client ${SOURCES})
target_link_libraries(client libjson.so)
