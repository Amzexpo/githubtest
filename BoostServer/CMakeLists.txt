cmake_minimum_required(VERSION 2.8)

project(BoostServer)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories($ENV{PATH})
link_directories($ENV{LD_LIBRARY_PATH})


add_executable(server "main.cpp" "asio.h" "asio.cpp" main.cpp asio.h asio.cpp)
