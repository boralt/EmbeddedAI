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
/// \ingroup basicquery
/// \{


/**
    Create FactorSet for the model of the following world
    - The grass can be wet due to to Sprinkler or Rain
    - Sprinkler frequency depends on Season (Winter)
    - Umbrelas are observed mor frequently if it is Raining

    The system will perform some inferences based on subset of observations
    \image html "../../docs/probbasic.png" width=60%

*/
int CreateRainTest(VarDb &db, FactorSet &fs)
{
   db.AddVar("A");   // Winter
   db.AddVar("B");   // Sprinkler
   db.AddVar("C");   // Rain
   db.AddVar("D");   // Wet grass
   db.AddVar("E");   // umbreala

   // Winter
   VarSet vsA(db);
   vsA << db["A"];
   std::shared_ptr<Factor> fA = std::make_shared<Factor>(vsA);
   fA->AddInstance(0, 0.4F);
   fA->AddInstance(1, 0.6F);

   // Sprinkler
   VarSet vsB(db);
   vsB << db["A"] << db["B"];
   std::shared_ptr<Factor> fB = std::make_shared<Factor>(vsB, db["B"]);
   Factor::FactorLoader flB(fB);
   flB << 0.25F << 0.8F << 0.75F << 0.2F;

   // Rain
   VarSet vsC(db);
   vsC << db["A"] << db["C"];
   std::shared_ptr<Factor> fC = std::make_shared<Factor>(vsC, db["C"]);
   Factor::FactorLoader flC(fC);
   flC << 0.9F << 0.2F << 0.1F << 0.8F;

   // Wet grass
   VarSet vsD(db);
   vsD << db["B"];
   vsD << db["C"];
   vsD << db["D"];
   std::shared_ptr<Factor> fD = std::make_shared<Factor>(vsD, db["D"]);
   Factor::FactorLoader flD(fD);
   flD << 1.0F << 0.1F << 0.2F << 0.05F << 0.0F << 0.9F << 0.8F << 0.95F;

   // see umbrela
   VarSet vsE(db);
   vsE << db["C"] << db["E"];
   std::shared_ptr<Factor> fE = std::make_shared<Factor>(vsE, db["E"]);
   Factor::FactorLoader flE(fE);
   flE << 1.F << 0.3F << 0.F << 0.7F;

   fs.AddFactor(fA);
   fs.AddFactor(fB);
   fs.AddFactor(fC);
   fs.AddFactor(fD);
   fs.AddFactor(fE);
   return 0;
}

/** This test observes 
    - Season  (Winter = TRUE)
    - Sprinkler (Sprinkler = OFF)
    The code will infer the  probabilities of all combinations of
    - Wet Grass
    - Umbrelas
*/
int TestRain()
{
   VarDb db;
   FactorSet fs(db);
   CreateRainTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==RAIN TEST==\n%s\n", s.c_str());

   // Observing Winter and Sprinkler off
   VarSet vsSample(db);
   vsSample << db["A"] << db["B"];
   Clause cSample(vsSample);
   cSample.SetVar(db["A"], true);
   cSample.SetVar(db["B"], false);

   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("\n =After apply =\n%s\n", s.c_str());


   // eliminate winter, sprinkler, rain 
   VarSet vsEliminate(db);
   vsEliminate << db["A"] << db["B"] << db["C"];
   fs.EliminateVar(vsEliminate);
   std::shared_ptr<Factor> res1 = fs.Merge();
   s = res1->GetJson(db);
   printf("\n==After Eliminate All but D E ==\n%s\n", s.c_str());

   EXPECT_NEAR(0.12, res1->Get(0), 0.1);
   EXPECT_NEAR(0.05, res1->Get(1), 0.1);
   EXPECT_NEAR(0.09, res1->Get(2), 0.1);
   EXPECT_NEAR(0.21, res1->Get(3), 0.1);
   return 0;
}

/** This test is same as in TestRain1() except
    PruneEdges optimization is invoked
*/
int TestRain2()
{
   VarDb db;
   FactorSet fs(db);
   CreateRainTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==RAIN TEST WITH EDGE Pruning==\n%s\n", s.c_str());


   VarSet vsSample(db);
   vsSample << db["A"] << db["B"];
   Clause cSample(vsSample);
   cSample.SetVar(db["A"], true);
   cSample.SetVar(db["B"], false);

   fs.PruneEdges(cSample);
   s = fs.GetJson(db);
   printf("\n =After Prune Edges =\n%s\n", s.c_str());


   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("\n =After apply =\n%s\n", s.c_str());



   VarSet vsEliminate(db);
   vsEliminate << db["A"] << db["B"] << db["C"];
   fs.EliminateVar(vsEliminate);
   s = fs.GetJson(db);
   printf("\n==After Eliminate All but D E ==\n%s\n", s.c_str());

   std::shared_ptr<Factor> res1 = fs.Merge();
   s = res1->GetJson(db);
   printf("\n==After Last Merge ==\n%s\n", s.c_str());

   EXPECT_NEAR(0.12, res1->Get(0), 0.1);
   EXPECT_NEAR(0.05, res1->Get(2), 0.1);
   EXPECT_NEAR(0.09, res1->Get(1), 0.1);
   EXPECT_NEAR(0.21, res1->Get(3), 0.1);
   return 0;
}

// \}

