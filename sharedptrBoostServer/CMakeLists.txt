cmake_minimum_required(VERSION 2.8)

project(sharedptrBoostServer)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories($ENV{PATH})
link_directories($ENV{LD_LIBRARY_PATH})

set(SOURCES main.cpp  Server.cpp Session.cpp)


# add_executable(server "main.cpp" "Server.h" "Session.h" "Server.cpp" "Session.cpp" main.cpp Server.h Session.h Server.cpp Session.cpp)
add_executable(server ${SOURCES})
target_link_directories(server PRIVATE ${PROJECT_SOURCE_DIR})
