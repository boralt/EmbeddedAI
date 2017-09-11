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
/// \ingroup deepCopy
/// \{

/**  Validate deep copy of FactorSet 
     <br> the copy of FactorSet is done when multiple queries
     Needs to be run against the same set of data
     <br> Note that
      - FactorSet is modified during performing most of the queries
      - Deep copy of recordset can be made in a middle of multiple algorithmic operations 
        over recordset such as Elininate, Maximaize so repeated operations can be done more effectivly 
*/

int Test_DeepCopy()
{
   VarDb db;
   // Preconfigure Player model as in Test1_1
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
   printf("\n===TEST 2===\n%s\n", s.c_str());

   // Make a copy of Factorset
   FactorSet fs2 = fs;

   // Add observation that player is prepared
   VarSet vsSample(db);
   vsSample << db["prep"];
   Clause cSample(vsSample);
   cSample.SetVar(db["prep"], true);

   printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());

   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("====After apply===\n%s", s.c_str());

   VarSet vsEliminate(db);
   vsEliminate << db["injury"];
   vsEliminate << db["prep"];
   fs.EliminateVar(vsEliminate);
   s = fs.GetJson(db);
   printf("\n==After merge==\n%s\n", s.c_str());

   EXPECT_FALSE(fs.IsEmpty());
   EXPECT_NEAR(0.664, (fs.GetFactors().front()->Get(1)), 0.01);
   EXPECT_NEAR(0.136, (fs.GetFactors().front()->Get(0)), 0.01);

   s = fs2.GetJson(db);
   printf("\n==Verify copy intact==\n%s\n", s.c_str());
   auto &r2 = fs2.GetFactors();
   EXPECT_EQ(r2.size(), 3);

   // Verify that all proper factors are present
   int nMatchCount = 0;
   for (auto iter = r2.begin(); iter != r2.end(); ++iter)
   {
      if (iter->get()->GetVarSet() == VarSet(db, { db["injury"], db["prep"], db["result"] }))
      {
         nMatchCount++;
      }
      else if (iter->get()->GetVarSet() == VarSet(db, { db["injury"] }))
      {
         nMatchCount++;
      }
      else if (iter->get()->GetVarSet() == VarSet(db, { db["prep"] }))
      {
         nMatchCount++;
      }
   }
   EXPECT_EQ(nMatchCount, 3);

   return 0;
}


/// \}

