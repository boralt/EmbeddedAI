cmake_minimum_required(VERSION 3.8)
project(dvntest)


set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(../src ../libs/json ../googletest/include)


set(SOURCE_FILES
    dvn_tree.cpp
    dvn_tree1.cpp
    dvntest.cpp)

set(INSTALL_DIR bin/dvntest)

add_executable(dvntest ${SOURCE_FILES})

target_link_libraries(dvntest bayes gtest)
