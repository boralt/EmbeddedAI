cmake_minimum_required(VERSION 3.6)
project(test1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(../src ../libs/json ../googletest/include)

set(SOURCE_FILES test1.cpp basic_query.cpp decision_test.cpp electric_circuit_diag.cpp factorset_deep_copy.cpp
        isp_example.cpp json_factor_factory.cpp json_factory.cpp large_test.cpp test_basic_solve.cpp )

set(INSTALL_DIR bin/tests)

add_executable(test1 ${SOURCE_FILES})
target_link_libraries(test1 bayes gtest)

#install(TARGETS test1 RUNTIME DESTINATION ${INSTALL_DIR} )
