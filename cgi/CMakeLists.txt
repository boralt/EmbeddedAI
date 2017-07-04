cmake_minimum_required(VERSION 3.6)
project(cgi)

set(CMAKE_CXX_STANDARD 11)
include_directories(../src ../libs/json)
set(INSTALL_DIR bin/cgi)


set(SOURCE_FILES main.cpp)
add_executable(cgi ${SOURCE_FILES})

target_link_libraries(cgi bayes)
