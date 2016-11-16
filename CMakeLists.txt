cmake_minimum_required(VERSION 3.3)
project(chatTCP_wincli)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES skel.h etcp.h sofuncs.c wincompat.c miscfuncs.c main.cpp)
add_executable(chatTCP_wincli ${SOURCE_FILES})
target_link_libraries(chatTCP_wincli ws2_32)