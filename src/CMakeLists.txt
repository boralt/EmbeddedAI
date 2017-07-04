cmake_minimum_required(VERSION 2.8.4)
project(airt)

set(LIB_NAME bayes)

include_directories(../libs/json)

set(SOURCE_FILES
        Factor.cpp
        FactorFactory.cpp
        FactorMergeHelper.cpp
        FactorSet.cpp
        Var.cpp
        VarDb.cpp
        VarSet.cpp
        Clause.cpp
        FactorFactory.cpp
        VarSetFactory.cpp
        ClauseFactory.cpp
        FactorSetFactory.cpp
        SessionEntry.cpp
        VarDbFactory.cpp
        DecisionBuilderHelper.cpp
        DecisionFunction.cpp
        ../libs/json/jsoncpp.cpp
        InteractionGraph.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(EXECUTABLE_OUTPUT_PATH ../dist/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ../dist/${CMAKE_BUILD_TYPE})

add_library(bayes STATIC ${SOURCE_FILES})
