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

/** \function
   Some basic tests
  - Add Variables to domain database
  - Construct VarSets using Variables
  - Construct Factors using Varsets
  - Populate Factors with probability values

 This file uses tiny model of describing athlete performance
 depending on preparation level and previous injury
 \image html ./docs/palyer.png

**/
int Test1_1()
{
   VarDb db;

   db.AddVar("injury");
   db.AddVar("prep");
   db.AddVar("result");

   // Player plays injury free 80% of the time
   VarSet vs1(db);
   vs1 << db["injury"];
   std::shared_ptr<Factor> fInjury = std::make_shared<Factor>(vs1);
   fInjury->AddInstance(0, 0.8F);
   fInjury->AddInstance(1, 0.2F);

   // Player prepaired for game 90% of the time
   VarSet vs2(db);
   vs2 << db["prep"];
   std::shared_ptr<Factor> fPrep = std::make_shared<Factor>(vs2);
   fPrep->AddInstance(0, 0.1F);
   fPrep->AddInstance(1, 0.9F);

   // Probalities of win based on injury and preparations
   VarSet vs3(db);
   vs3 << db["injury"];
   vs3 << db["prep"];
   vs3 << db["result"];
   std::shared_ptr<Factor> fRes = std::make_shared<Factor>(vs3);
   Factor::FactorLoader fl(fRes);
   fl << 0.4F << 0.9F << 0.1F << 0.8F << 0.6F << 0.1F << 0.9F << 0.2F;

   FactorSet fs(db);
   fs.AddFactor(fInjury);
   fs.AddFactor(fPrep);
   fs.AddFactor(fRes);

   std::string s = fs.GetJson(db);
   printf("\n=== TEST 1 ===\n%s\n", s.c_str());

   // calculate overall probability of Win for this player 
   VarSet vsEliminate(db);
   vsEliminate << db["injury"] << db["prep"];
   fs.EliminateVar(vsEliminate);
   s = fs.GetJson(db);
   printf("==After merge==\n%s\n", s.c_str());

   EXPECT_FALSE(fs.IsEmpty());
   EXPECT_NEAR(0.73, (fs.GetFactors().front()->Get(1)), 0.01);
   EXPECT_NEAR(0.27, (fs.GetFactors().front()->Get(0)), 0.01);

   return 0;
}


int Test1_3()
{
   VarDb db;

   db.AddVar("injury");
   db.AddVar("prep");
   db.AddVar("result");

   VarSet vs1(db);
   vs1 << db["injury"];
   std::shared_ptr<Factor> fInjury = std::make_shared<Factor>(vs1);
   fInjury->AddInstance(0, 0.9F);
   fInjury->AddInstance(1, 0.1F);

   VarSet vs2(db);
   vs2 << db["prep"];
   std::shared_ptr<Factor> fPrep = std::make_shared<Factor>(vs2);
   fPrep->AddInstance(0, 0.2F);
   fPrep->AddInstance(1, 0.8F);

   VarSet vs3(db);
   vs3 << db["injury"];
   vs3 << db["prep"];
   vs3 << db["result"];
   std::shared_ptr<Factor> fRes = std::make_shared<Factor>(vs3);
   Factor::FactorLoader fl(fRes);
   fl << 0.4F << 0.9F << 0.1F << 0.8F << 0.6F << 0.1F << 0.9F << 0.2F;

   FactorSet fs(db);
   fs.AddFactor(fInjury);
   fs.AddFactor(fPrep);
   fs.AddFactor(fRes);

   std::string s = fs.GetJson(db);
   printf("\n===TEST 3===\n%s\n", s.c_str());

   // observation result is false (player lost)
   VarSet vsSample(db);
   vsSample << db["result"];
   Clause cSample(vsSample);
   cSample.SetVar(db["result"], false);

   printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());


   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("==After apply==\n%s", s.c_str());

   FactorSet fs2 = fs;      // copy of factorset 

   VarSet vsEliminateInjury(db);
   vsEliminateInjury << db["result"] << db["injury"];
   fs.EliminateVar(vsEliminateInjury);
   std::shared_ptr<Factor> res1 = fs.Merge()->Normalize();
   s = res1->GetJson(db);
   printf("\n==After Eliminate Injury==\n%s\n", s.c_str());

   VarSet vsEliminatePrep(db);
   vsEliminatePrep << db["prep"] << db["result"];
   fs2.EliminateVar(vsEliminatePrep);
   std::shared_ptr<Factor> res2 = fs2.Merge()->Normalize();

   s = res2->GetJson(db);
   printf("\n==After Eliminate Prep==\n%s\n", s.c_str());

   EXPECT_NEAR(0.64, res2->Get(0), 0.01);
   EXPECT_NEAR(0.36, res2->Get(1), 0.01);

   return 0;
}


/// \}
