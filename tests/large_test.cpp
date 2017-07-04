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

/** \file
 \ingroup largeModel
 \{

 Large describe the network shown on picture below. <br>
 Traffic is sent over links 1,2,3,4. Inside the network traffic on each link 
 is split into 2 sublink. On the Endpoint Node the packet drops are measured
 and attributed to each of 8 sublinks 1_1, 1_2, 2_1... 4_2. Also accumulated 
 over medium interval time traffic drop is calculated for the same 8 sublinks.
 Traffic dropped can be caused by congestion on links 1..4, any of 8 sublinks or
 Endpoint Node itself.
 /// \image html "../../docs/netdiag.png" width=60%

*/




/**  Create Model of Carrier Network 
     packet drops are measured for each sublink
     The system can sample instantenues and accumulated rate of packet drop
     digitized into two regions -- hi drop and low drop
     /// \image html "../../docs/netgraph.png" width=60%
*/

int InitLargeTest(VarDb &db, FactorSet &fs)
{
   char sz[10];

   // 4 links links with 2 sublinks each 
   for (int nLink = 1; nLink <= 4; nLink++)
   {
      // conjested link

      snprintf(sz, 10, "cjl%d", nLink);
      db.AddVar(sz);

      // conjested sublink
      snprintf(sz, 10, "cjl%d_1", nLink);
      db.AddVar(sz);

      snprintf(sz, 10, "cjl%d_2", nLink);
      db.AddVar(sz);

      // packet drops

      // hi drop sublink1
      snprintf(sz, 10, "drhi%d_1", nLink);
      db.AddVar(sz);

      // hi drop sublink 2
      snprintf(sz, 10, "drhi%d_2", nLink);
      db.AddVar(sz);

      // lo drop sublink 1
      snprintf(sz, 10, "drlo%d_1", nLink);
      db.AddVar(sz);

      // lo drop sublink 2
      snprintf(sz, 10, "drlo%d_2", nLink);
      db.AddVar(sz);


      // acc hi drop sublink1
      snprintf(sz, 10, "drhia%d_1", nLink);
      db.AddVar(sz);

      // acc hi drop sublink 2
      snprintf(sz, 10, "drhia%d_2", nLink);
      db.AddVar(sz);

      // acc lo drop sublink 1
      snprintf(sz, 10, "drloa%d_1", nLink);
      db.AddVar(sz);

      // acc lo drop sublink 2
      snprintf(sz, 10, "drloa%d_2", nLink);
      db.AddVar(sz);
   }
   // conjested endpoint
   db.AddVar("cjE");

   for (int nLink = 1; nLink <= 4; nLink++)
   {
      // toplevel varaibles.
      VarSet vsConjL;
      // conjested link
      snprintf(sz, 10, "cjl%d", nLink);
      vsConjL << db[sz];
      std::shared_ptr<Factor> fConjL = std::make_shared<Factor>(vsConjL, db[sz]);
      fConjL->AddInstance(0, 0.95F);
      fConjL->AddInstance(1, 0.05F);

      VarSet vsConjSL1;
      // conjested sub link 1 
      snprintf(sz, 10, "cjl%d_1", nLink);
      vsConjSL1 << db[sz];
      std::shared_ptr<Factor> fConjSL1 = std::make_shared<Factor>(vsConjSL1, db[sz]);
      fConjSL1->AddInstance(0, 0.9F);
      fConjSL1->AddInstance(1, 0.1F);

      VarSet vsConjSL2;
      // conjested sub link 2 
      snprintf(sz, 10, "cjl%d_2", nLink);
      vsConjSL2 << db[sz];
      std::shared_ptr<Factor> fConjSL2 = std::make_shared<Factor>(vsConjSL2, db[sz]);
      fConjSL2->AddInstance(0, 0.9F);
      fConjSL2->AddInstance(1, 0.1F);

      // second level variables

      VarSet vsDropLo1;
      // low drop link 1
      VarId headVar;
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropLo1 << db[sz];
      snprintf(sz, 10, "cjl%d_1", nLink);
      vsDropLo1 << db[sz];
      vsDropLo1 << db["cjE"];
      snprintf(sz, 10, "drlo%d_1", nLink);
      headVar = db[sz];
      vsDropLo1 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropLo1 = std::make_shared<Factor>(vsDropLo1, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropLo1(fDropLo1);
      flDropLo1 << 0.99F << 0.7F << 0.7F << 0.6F << 0.7F << 0.6F << 0.6F << 0.55F;
      flDropLo1 << 0.01F << 0.3F << 0.3F << 0.4F << 0.3F << 0.4F << 0.4F << 0.45F;
      /////

      VarSet vsDropLo2;
      // low drop link 2
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropLo2 << db[sz];
      snprintf(sz, 10, "cjl%d_2", nLink);
      vsDropLo2 << db[sz];
      vsDropLo2 << db["cjE"];
      snprintf(sz, 10, "drlo%d_2", nLink);
      headVar = db[sz];
      vsDropLo2 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropLo2 = std::make_shared<Factor>(vsDropLo2, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropLo2(fDropLo2);
      flDropLo2 << 0.99F << 0.7F << 0.7F << 0.6F << 0.7F << 0.6F << 0.6F << 0.55F;
      flDropLo2 << 0.01F << 0.3F << 0.3F << 0.4F << 0.3F << 0.4F << 0.4F << 0.45F;

      //HHHHHHHHHHHHHHH
      VarSet vsDropHi1;
      // low drop link 1
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropHi1 << db[sz];
      snprintf(sz, 10, "cjl%d_1", nLink);
      vsDropHi1 << db[sz];
      vsDropHi1 << db["cjE"];
      snprintf(sz, 10, "drhi%d_1", nLink);
      headVar = db[sz];
      vsDropHi1 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropHi1 = std::make_shared<Factor>(vsDropHi1, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropHi1(fDropHi1);
      flDropHi1 << 0.998F << 0.9F << 0.9F << 0.8F << 0.9F << 0.7F << 0.7F << 0.65F;
      flDropHi1 << 0.002F << 0.1F << 0.1F << 0.2F << 0.1F << 0.3F << 0.3F << 0.35F;
      /////

      VarSet vsDropHi2;
      // low drop link 2
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropHi2 << db[sz];
      snprintf(sz, 10, "cjl%d_2", nLink);
      vsDropHi2 << db[sz];
      vsDropHi2 << db["cjE"];
      snprintf(sz, 10, "drhi%d_2", nLink);
      headVar = db[sz];
      vsDropHi2 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropHi2 = std::make_shared<Factor>(vsDropHi2, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropHi2(fDropHi2);

      flDropHi2 << 0.998F << 0.9F << 0.9F << 0.8F << 0.9F << 0.7F << 0.7F << 0.65F;
      flDropHi2 << 0.002F << 0.1F << 0.1F << 0.2F << 0.1F << 0.3F << 0.3F << 0.35F;

      ////////////////////////////////////////////////////
      // ACCUMULATED drop VARS

      VarSet vsDropLoA1;
      // low drop link 1
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropLoA1 << db[sz];
      snprintf(sz, 10, "cjl%d_1", nLink);
      vsDropLoA1 << db[sz];
      vsDropLoA1 << db["cjE"];
      snprintf(sz, 10, "drloa%d_1", nLink);
      headVar = db[sz];
      vsDropLoA1 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropLoA1 = std::make_shared<Factor>(vsDropLoA1, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropLoA1(fDropLoA1);
      flDropLoA1 << 0.95F << 0.5F << 0.5F << 0.4F << 0.4F << 0.4F << 0.4F << 0.30F;
      flDropLoA1 << 0.05F << 0.5F << 0.5F << 0.6F << 0.6F << 0.6F << 0.6F << 0.70F;
      /////

      VarSet vsDropLoA2;
      // low drop link 2
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropLoA2 << db[sz];
      snprintf(sz, 10, "cjl%d_2", nLink);
      vsDropLoA2 << db[sz];
      vsDropLoA2 << db["cjE"];
      snprintf(sz, 10, "drloa%d_2", nLink);
      headVar = db[sz];
      vsDropLoA2 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropLoA2 = std::make_shared<Factor>(vsDropLoA2, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropLoA2(fDropLoA2);
      flDropLoA2 << 0.95F << 0.5F << 0.5F << 0.4F << 0.4F << 0.4F << 0.4F << 0.30F;
      flDropLoA2 << 0.05F << 0.5F << 0.5F << 0.6F << 0.6F << 0.6F << 0.6F << 0.70F;

      //HHHHHHHHHHHHHHH
      VarSet vsDropHiA1;
      // low drop link 1
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropHiA1 << db[sz];
      snprintf(sz, 10, "cjl%d_1", nLink);
      vsDropHiA1 << db[sz];
      vsDropHiA1 << db["cjE"];
      snprintf(sz, 10, "drhia%d_1", nLink);
      headVar = db[sz];
      vsDropHiA1 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropHiA1 = std::make_shared<Factor>(vsDropHiA1, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropHiA1(fDropHiA1);
      flDropHiA1 << 0.999F << 0.8F << 0.7F << 0.7F << 0.7F << 0.6F << 0.7F << 0.55F;
      flDropHiA1 << 0.001F << 0.2F << 0.3F << 0.3F << 0.3F << 0.4F << 0.3F << 0.45F;
      /////

      VarSet vsDropHiA2;
      // low drop link 2
      snprintf(sz, 10, "cjl%d", nLink);
      vsDropHiA2 << db[sz];
      snprintf(sz, 10, "cjl%d_2", nLink);
      vsDropHiA2 << db[sz];
      vsDropHiA2 << db["cjE"];
      snprintf(sz, 10, "drhia%d_2", nLink);
      headVar = db[sz];
      vsDropHiA2 << headVar;

      // construct factor
      std::shared_ptr<Factor> fDropHiA2 = std::make_shared<Factor>(vsDropHiA2, headVar);
      // load factor using factor loader
      Factor::FactorLoader flDropHiA2(fDropHiA2);

      flDropHiA2 << 0.999F << 0.8F << 0.7F << 0.7F << 0.7F << 0.6F << 0.7F << 0.55F;
      flDropHiA2 << 0.001F << 0.2F << 0.3F << 0.3F << 0.3F << 0.4F << 0.3F << 0.45F;

      // add all Factors to the factorset
      fs.AddFactor(fConjL);
      fs.AddFactor(fConjSL1);
      fs.AddFactor(fConjSL2);
      fs.AddFactor(fDropLo1);
      fs.AddFactor(fDropLo2);
      fs.AddFactor(fDropHi1);
      fs.AddFactor(fDropHi2);
      fs.AddFactor(fDropLoA1);
      fs.AddFactor(fDropLoA2);
      fs.AddFactor(fDropHiA1);
      fs.AddFactor(fDropHiA2);
   }

   VarSet vsConjE;
   // conjested link
   vsConjE << db["cjE"];
   std::shared_ptr<Factor> fConjE = std::make_shared<Factor>(vsConjE, db["cjE"]);
   fConjE->AddInstance(0, 0.92F);
   fConjE->AddInstance(1, 0.08F);
   fs.AddFactor(fConjE);


   return 0;
}


/**  Evaluate  information about packet drops on on certain segents of network 
     infer most likely congested segments on the network
*/
int LargeTest1()
{
   VarDb db;
   FactorSet fs(db);
   InitLargeTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Large test with edge Pruning==\n%s\n", s.c_str());


   VarSet vsSample, vsSolve;
   vsSolve << db["cjl1_1"] << db["cjl1_2"] << db["cjl1"];
   vsSolve << db["cjl2_1"] << db["cjl2_2"] << db["cjl2"];
   vsSolve << db["cjl3_1"] << db["cjl3_2"] << db["cjl3"];
   vsSolve << db["cjl4_1"] << db["cjl4_2"] << db["cjl4"];
   vsSolve << db["cjE"];

   VarSet varDbSet = db.GetVarSet();



   vsSample = db.GetVarSet().Substract(vsSolve);

   s = vsSample.GetJson(db);
   printf("\n   == Test Sample Varset ==\n%s\n", s.c_str());



   Clause cSample(vsSample);

#if 0
   cSample.SetVar(db["drhi3_1"], true);
   cSample.SetVar(db["drlo3_1"], true);
   cSample.SetVar(db["drhi3_2"], false);
   cSample.SetVar(db["drlo3_2"], true);
   cSample.SetVar(db["drhia3_1"], false);
   cSample.SetVar(db["drloa3_1"], true);
   cSample.SetVar(db["drhia3_2"], true);
   cSample.SetVar(db["drhia3_2"], false);
#endif

   cSample.SetVar(db["drhi3_1"], true);
   cSample.SetVar(db["drlo3_1"], true);
   cSample.SetVar(db["drhi3_2"], false);
   cSample.SetVar(db["drlo3_2"], false);
   cSample.SetVar(db["drhia3_1"], false);
   cSample.SetVar(db["drloa3_1"], true);
   cSample.SetVar(db["drhia3_2"], false);
   cSample.SetVar(db["drhia3_2"], false);


   s = cSample.GetJson(db);
   printf("\n   == Test Sample Clause ==\n%s\n", s.c_str());

   fs.PruneEdges(cSample);
   // s = fs.GetJson();
   // printf("\n =After Prune Edges =\n%s\n", s.c_str());


   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("\n =After apply =\n%s\n", s.c_str());

   fs.MaximizeVar(varDbSet);

   s = fs.GetJson(db);
   printf("\n =After MAX up =\n%s\n", s.c_str());


   std::shared_ptr<Factor> res1 = fs.Merge();
   s = res1->GetJson(db);
   printf("\n==After MPE calculation ==\n%s\n", s.c_str());

   VarSet vsMpe = res1->GetExtendedVarSet();
   InstanceId instanceMpe = res1->GetExtendedClause(0);
   Clause clMpe(vsMpe, instanceMpe);
   s = clMpe.GetJson(db);
   printf("==MPE clause ==\n%s\n", s.c_str());

   for (VarId vid = vsMpe.GetFirst(); vid != 0; vid = vsMpe.GetNext(vid))
   {
      if (vid == db["drlo3_1"] ||
         vid == db["drhi3_1"] ||
         vid == db["drloa3_1"] || vid == db["cjl3_1"])
      {
         EXPECT_TRUE(clMpe[vid]);
      }
      else
      {
         EXPECT_FALSE(clMpe[vid]);
      }
   }
   EXPECT_NEAR(0.00024, res1->Get(0), 0.00001);


   return 0;
}

/**
   Same test as LargeTest1() but performs algorithm optimization
   that dramatically reduces duration of calculations
  
*/

int LargeTest2()
{
   VarDb db;
   FactorSet fs(db);
   InitLargeTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Large test2  with edge Pruning==\n%s\n", s.c_str());


   VarSet vsSample, vsSolve;
   vsSolve << db["cjl1_1"] << db["cjl1_2"] << db["cjl1"];
   vsSolve << db["cjl2_1"] << db["cjl2_2"] << db["cjl2"];
   vsSolve << db["cjl3_1"] << db["cjl3_2"] << db["cjl3"];
   vsSolve << db["cjl4_1"] << db["cjl4_2"] << db["cjl4"];
   vsSolve << db["cjE"];

   VarSet varDbSet = db.GetVarSet();



   vsSample = db.GetVarSet().Substract(vsSolve);

   s = vsSample.GetJson(db);
   printf("\n   == Test Sample Varset ==\n%s\n", s.c_str());



   Clause cSample(vsSample);

#if 0
   cSample.SetVar(db["drhi3_1"], true);
   cSample.SetVar(db["drlo3_1"], true);
   cSample.SetVar(db["drhi3_2"], false);
   cSample.SetVar(db["drlo3_2"], true);
   cSample.SetVar(db["drhia3_1"], false);
   cSample.SetVar(db["drloa3_1"], true);
   cSample.SetVar(db["drhia3_2"], true);
   cSample.SetVar(db["drhia3_2"], false);
#endif

   cSample.SetVar(db["drhi3_1"], true);
   cSample.SetVar(db["drlo3_1"], true);
   cSample.SetVar(db["drhi3_2"], false);
   cSample.SetVar(db["drlo3_2"], false);
   cSample.SetVar(db["drhia3_1"], false);
   cSample.SetVar(db["drloa3_1"], true);
   cSample.SetVar(db["drhia3_2"], false);
   cSample.SetVar(db["drhia3_2"], false);


   s = cSample.GetJson(db);
   printf("\n   == Test Sample Clause ==\n%s\n", s.c_str());

   fs.PruneEdges(cSample);
   // s = fs.GetJson();
   // printf("\n =After Prune Edges =\n%s\n", s.c_str());


   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   //s = fs.GetJson(db);
   //printf("\n =After apply =\n%s\n", s.c_str());

   s = varDbSet.GetJson(db);
   printf("\n==== Initial VS%s", s.c_str());


   InteractionGraph ig(&fs);
   VarSet optVs = ig.GetElimOrder();
   s = optVs.GetJson(db);
   printf("\n==== Updated VS%s", s.c_str());

   fs.MaximizeVar(optVs);


   s = fs.GetJson(db);
   printf("\n =After MAX up =\n%s\n", s.c_str());


   std::shared_ptr<Factor> res1 = fs.Merge();
   s = res1->GetJson(db);
   printf("\n==After MPE calculation ==\n%s\n", s.c_str());

   VarSet vsMpe = res1->GetExtendedVarSet();
   InstanceId instanceMpe = res1->GetExtendedClause(0);
   Clause clMpe(vsMpe, instanceMpe);
   s = clMpe.GetJson(db);
   printf("==MPE clause ==\n%s\n", s.c_str());

   for (VarId vid = vsMpe.GetFirst(); vid != 0; vid = vsMpe.GetNext(vid))
   {
      if (vid == db["drlo3_1"] ||
         vid == db["drhi3_1"] ||
         vid == db["drloa3_1"] || vid == db["cjl3_1"])
      {
         EXPECT_TRUE(clMpe[vid]);
      }
      else
      {
         EXPECT_FALSE(clMpe[vid]);
      }
   }
   EXPECT_NEAR(0.00024, res1->Get(0), 0.00001);

   return 0;
}
