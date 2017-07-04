/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/


// #include "stdafx.h"
#include <factor.h>
#include <Factories.h>
#include <json/json.h>
#include <fstream>
#include <gtest/gtest.h>

/// @file

/**
   @defgroup examples EmbeddedAI tests
   @brief Entry point for EmbeddedAi tests and code samples
   @{

*/

/** @defgroup basicquery Basic Queries  
 @brief simple example queries to small models
*/

/**  @defgroup exampleCircuit Digital Circuit Example
 @brief example diagnostic digital circuit
*/

/** @defgroup decisionTest Test Decision Algorithm
  @brief Test Bayesian Decision Engine. Multiple sequential decisions example
*/

/** @defgroup deepCopy FactorSet deepy copy 
  @brief Sample of runing multiple queries against the same model
*/

/** @defgroup exampleIsp Isp Examples
   @brief Sample code from project Wiki Walkthroughs
*/

/** @defgroup jsonFactory Json Factory Example
   @brief Test Json Interface into EmbeddedAI system
*/

/** @defgroup largeModel Test Quering Large Models
   @brief Test regular and optimized operations on Large model
*/

/** @} */


using namespace bayeslib;


int Test1_1();
int Test_DeepCopy();
int Test1_3();
int Test_JsonFactor();
int TestRain();
int TestRain2();
int TestCircuit1();
int TestCircuit2();
int TestJson();
int LargeTest1();
int LargeTest2();
int DecisionTest1();

int IspTest1();
int IspTest2();
int IspDecisionExample();


TEST(BASIC, TEST1_1)
{
    EXPECT_EQ(0, Test1_1());
}

TEST(BASIC, Test_DeepCopy)
{
   EXPECT_EQ(0, Test_DeepCopy());
}


TEST(BASIC, Test1_3)
{
   EXPECT_EQ(0, Test1_3());
}

TEST(BASIC, Test_JsonFactor)
{
   EXPECT_EQ(0, Test_JsonFactor());
}

TEST(BASIC, TestRain)
{
   EXPECT_EQ(0, TestRain());
}

TEST(BASIC, TestRain2)
{
   EXPECT_EQ(0, TestRain2());
}

TEST(BASIC, TestCircuit1)
{
   EXPECT_EQ(0, TestCircuit1());
}

TEST(BASIC, TestCircuit2)
{
   EXPECT_EQ(0, TestCircuit2());
}

#if 0
TEST(BASIC, TestJson)
{
   EXPECT_EQ(0, TestJson("..\\tests\\TestFile1.json"));
}
#endif

TEST(BASIC, TestJson)
{
   EXPECT_EQ(0, TestJson());
}



TEST(BASIC, DecisionTest1)
{
   EXPECT_EQ(0, DecisionTest1());
}


TEST(BASIC, LargeTest1)
{
   EXPECT_EQ(0, LargeTest1());
}


TEST(BASIC, LargeTest2)
{
    EXPECT_EQ(0, LargeTest2());
}


TEST(EXAMPLE, IspTest1)
{
   EXPECT_EQ(0, IspTest1());
}

TEST(EXAMPLE, IspTest2)
{
   EXPECT_EQ(0, IspTest2());
}


TEST(EXAMPLE, IspDecisionExample)
{
   EXPECT_EQ(0, IspDecisionExample());
}


int main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   int res = RUN_ALL_TESTS();
   getchar();
   return res;
}


#if 0
int main(int argc, char **argv)
{
   if (argc > 1)
   {
      TestJson(argv[1]);
      getchar();
      return 0;
   }

   Test1_1();
 
   Test1_2();
   Test1_3();
   Test1_4();

  
   TestRain();
   TestRain2();
   TestCircuit1();
   TestCircuit2();

   getchar();

}
#endif

















