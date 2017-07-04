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

/// \file
/// \ingroup jsonFactory
/// \{


/** Test construction of  Factor tables from Json
*/

int Test_JsonFactor()
{
   VarDb db;


   const char def[] =
      "{\"vars\":[\"injury\", \"prep\", \"res\"], \"vals\":[0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8]}";
   Json::Value v;
   Json::Reader r;
   if (!r.parse(def, v))
   {
      printf("\n=== TEST 4 -- Failed\n %s", r.getFormattedErrorMessages().c_str());
      return -1;
   }

   std::shared_ptr<Factor> f = FactorFactory::Create(db, v);
   FactorSet fs(db);
   fs.AddFactor(f);

   std::string s = fs.GetJson(db);
   printf("\n===TEST 4===\n%s\n", s.c_str());
   EXPECT_TRUE(0.1F == f->Get(0) && 0.2F == f->Get(1) && 0.3F == f->Get(2) && 0.4F == f->Get(3) && 0.5F == f->Get(4) && 0.6F == f->Get(5) && 0.7F == f->Get(6) && 0.8F == f->Get(7));

   return 0;
}

