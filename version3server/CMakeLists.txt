cmake_minimum_required(VERSION 2.8)

project(sharedptrBoostServer)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread -ljson")
set(jsoncpp /usr/local/lib/libjson.so)

include_directories($ENV{PATH})
link_directories($ENV{LD_LIBRARY_PATH})

# set(SOURCES main.cpp  Server.cpp Session.cpp LogicSystem.cpp MsgNode.cpp)
aux_source_directory(. SOURCESLIST)

add_executable(server ${SOURCESLIST})

target_link_libraries(server libjson.so)
