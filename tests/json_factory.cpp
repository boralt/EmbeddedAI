/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/

#include <factor.h>
#include <Factories.h>
#include <json/json.h>
#include <fstream>
#include <gtest/gtest.h>


using namespace bayeslib;


/**
 \ingroup jsonFactory
  /{
*/


const char *szJsonInput = R"( {
   "VarDb": ["J", "I", "Y", "X", "O"],
      "FactorSet" : [
   {
      "vars": ["I"],
         "head" : ["I"],
         "vals" : [0.5, 0.5]
   },
   {
      "vars": ["J"],
      "head" : ["J"],
      "vals" : [0.5, 0.5]
   },
   {
      "vars": ["J", "Y"],
      "head" : ["Y"],
      "vals" : [0.01, 0.99, 0.99, 0.01]
   },
   {
      "vars": ["I", "J", "X"],
      "head" : ["X"],
      "vals" : [0.95, 0.95, 0.95, 0.05, 0.05, 0.05, 0.05, 0.95]
   },
   {
      "vars": ["X", "Y", "O"],
      "head" : ["O"],
      "vals" : [0.98, 0.02, 0.02, 0.02, 0.02, 0.98, 0.98, 0.98]
   }
      ],
      "QueryVarSet": ["I", "J"],
         "SampleClause" : {
         "varset": ["O"],
            "values" : [1]
      },
         "op" : "MAP"
})";

#if 0

std::string ReadJsonFile(const char *pFileName)
{
   std::stringstream sstr;
   std::ifstream in(pFileName);
   sstr << in.rdbuf();
   return sstr.str();
}


int TestJson(const char *pFileName)
{
   std::string s = ReadJsonFile(pFileName);
   s = SessionEntry::RunCommand(s);
   printf("==XML result ==\n%s\n", s.c_str());
   return 0;
}

#endif


/// Create FactorSet and run specified high level EmbeddedAI command
int TestJson()
{
   std::string s = szJsonInput;
   s = SessionEntry::RunCommand(s);
   printf("==XML result ==\n%s\n", s.c_str());
   return 0;
}

// \}

