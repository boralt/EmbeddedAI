cmake_minimum_required(VERSION 3.6)
project(airt)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES main.cpp)
#add_executable(airt ${SOURCE_FILES})

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(googletest)
add_subdirectory(cgi)
add_subdirectory(dvntest)

