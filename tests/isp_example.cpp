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


/// \file 
/// \ingroup exampleIsp  
/// \{


using namespace bayeslib;

/// Initialize of ISP  model
/// \image html "../../docs/isp.png" width=60%

int InitIspExample(VarDb &db, FactorSet &fs)
{
   // define variables
   db.AddVar("Sched");   // Scheduled maintainance
   db.AddVar("Maint");   // Maintainance in progress 
   db.AddVar("Weather"); // Bad Rain
   db.AddVar("Alert");   // Weather alert
   db.AddVar("Cong");    // Internet congestion
   db.AddVar("Drop");    // Drop packets
   db.AddVar("Damage");  // internet access failure, require roll a track
   db.AddVar("Outage");  // Temporary Internet outage
   db.AddVar("Call");    // Service Call received

                         // Create factors

                         // Create Factor Sched which is just a probability that Maintainance
                         // scheduled at any given time
   VarSet vsSched(db);
   vsSched << db["Sched"];
   std::shared_ptr<Factor> fSched = std::make_shared<Factor>(vsSched, vsSched);
   fSched->AddInstance(0, 0.96F);  // not scheduled 96% of the time
   fSched->AddInstance(1, 0.04F);  // scheduled 4 % of the time

                                   // Create Factor with bad Weather probability
                                   // Another method of initialization
   std::shared_ptr<Factor> fWeather(new Factor(db, { db["Weather"] }, { db["Weather"] }));
   (*fWeather) << 0.995F << fin;  // 99.5% of the time weather is not bad
                                  //fWeather->AddInstance(1, 0.005);    * not needed as fin manipulator above does that

                                  // Create Factorwith probabilities of internet congestion 
   std::shared_ptr<Factor> fCong(new Factor(db, { db["Cong"] }, { db["Cong"] }));
   (*fCong) << 0.98F << fin;  // 98% of timethere is no congestion


                              // Create factor with conditional probability of schedule was set for this maintainance 
   std::shared_ptr<Factor> fMaint(new Factor(db, { db["Sched"], db["Maint"] }, { db["Maint"] }));
   (*fMaint) << 0.99F << 0.4F << fin;  // 99% no unscheduled maintiannace, 40% sheduled maintainance is not performed

                                       // Create factor with conditional probabilities of Alert based on Weather Condition  
   std::shared_ptr<Factor> fAlert(new Factor(db, { db["Weather"], db["Alert"] }, { db["Alert"] }));
   (*fAlert) << 0.998F << 0.3F << fin;  // 99.8% no bad weather will not alert, 30% bad weather will not cause alert

                                        // Create factor with conditional probabilities of packet drop based on Internet Congestion
   std::shared_ptr<Factor> fDrop(new Factor(db, { db["Cong"], db["Drop"] }, { db["Drop"] }));
   (*fDrop) << 0.995F << 0.4F << fin;  // 99.5% no congestion pings will notdrop, 40% congestion packet will not drop

                                       // probability of damage based on weather condition and maintainace 
   std::shared_ptr<Factor> fDamage(new Factor(db, { db["Weather"], db["Maint"], db["Damage"] }, { db["Damage"] }));
   (*fDamage) << 0.997F << 0.94F << 0.96F << 0.84F << fin;

   // probability of outage based on weather, maintainance and congestion
   std::shared_ptr<Factor> fOutage(new Factor(db, { db["Weather"], db["Maint"], db["Cong"], db["Outage"] }, { db["Outage"] }));
   (*fOutage) << 0.99F << 0.92F << 0.9F << 0.88F << 0.8F << 0.82F << 0.85F << 0.78F << fin;

   // probability of service call based on damage and outage
   std::shared_ptr<Factor> fCall(new Factor(db, { db["Damage"], db["Outage"], db["Call"] }, { db["Call"] }));
   (*fCall) << 0.99F << 0.6F << 0.8F << 0.6F << fin;

   // add all asemled factor to factorset

   fs.AddFactor(fMaint);
   fs.AddFactor(fWeather);
   fs.AddFactor(fCong);
   fs.AddFactor(fSched);
   fs.AddFactor(fAlert);
   fs.AddFactor(fDrop);
   fs.AddFactor(fDamage);
   fs.AddFactor(fOutage);
   fs.AddFactor(fDrop);
   fs.AddFactor(fCall);

   return 0;
}

/// Init ISP AI decision network
/// \image html "../../docs/isp_decision.png" width=60%
int InitIspDecisionExample(VarDb &db, FactorSet &fs)
{
   int res = InitIspExample(db, fs);

   if (res)
      return res;

   db.AddVar("CustomerLoss"); // Customer loss probabilities
   db.AddVar("SendCrew", VarType_Decision);     // Decision variable
   db.AddVar("Utility", VarType_Utility);      // Utility node to truck expense of sending crew and loss of unsatisfied customers

   // Customer loss node
   std::shared_ptr<Factor> fCustomerLoss = std::shared_ptr<Factor>(new Factor(db, { db["Call"], db["Damage"], db["SendCrew"], db["CustomerLoss"] }, { db["CustomerLoss"] }));
   *fCustomerLoss << 0.999F << 0.995F << 0.97F << 0.7F << 0.999F << 0.998F << 0.997F << 0.985F << fin;


   std::shared_ptr<Factor> fSendCrew = std::shared_ptr<Factor>(new Factor(db, { db["Sched"], db["Alert"],db["Drop"], db["Call"], db["SendCrew"] }, { db["SendCrew"] }));
   fSendCrew->SetFactorType(VarType_Decision);

   // Utility
   VarSet vsUtility(db);
   vsUtility << db["Weather"] << db["Maint"] << db["SendCrew"]  << db["CustomerLoss"];
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   *fUtility << 0. << 0. << 0. << 0. << -300. << -400. << -500. << -600. << -5000. << -5000. << -5000. << -5000. << -5300. << -5400. << -5500. << -5600.;
   fUtility->SetFactorType(VarType_Utility);

   fs.AddFactor(fCustomerLoss);
   fs.AddFactor(fSendCrew);
   fs.AddFactor(fUtility);
   return 0;
}

/** Inject sample with 
    - Scheduled Maintainace = TRUE
    - Packet Drop  = FALSE
    - Weather Alert = TRUE
    - Customer Call = TRUE
    
    Find a combination of all variables that most likely explains this sample

    Repeat same test with 
    - Scheduled Maintainace = TRUE
    - Packet Drop  = FALSE
    - Weather Alert = FALSE
    - Customer Call = TRUE

*/

int IspTest1()
{

   VarDb db;
   FactorSet fsInitial(db);
   InitIspExample(db, fsInitial);


   std::string s = fsInitial.GetJson(db);
   printf("\n==ISP test with edge Pruning==\n%s\n", s.c_str());

   // observed sample
#if 0
   // long way to enter sampling data
   VarSet vsSample;
   vsSample << db["Sched"] << db["Alert"] << db["Drop"] << db["Call"];
   Clause cSample(vsSample);

   cSample.SetVar(db["Sched"], true);
   cSample.SetVar(db["Drop"], false);
   cSample.SetVar(db["Alert"], true);
#endif

   {
      FactorSet fs = fsInitial;
      Clause cSample(db, {
         { db["Sched"], true },{ db["Drop"], false },
         { db["Alert"], true },{ db["Call"],true }
      });

      printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());

      fs.PruneEdges(cSample);
      s = fs.GetJson(db);
      printf("\n =After Prune Edges =\n%s\n", s.c_str());

      fs.ApplyClause(cSample);
      s = fs.GetJson(db);
      printf("\n =After apply =\n%s\n", s.c_str());

      // This is MPE case -- calculate maximum 
      // probablity explanation of sample 
      fs.MaximizeVar(db.GetVarSet());
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
   }
#if 0
   EXPECT_FALSE(clMpe.GetVar(db["X"]));
   EXPECT_TRUE(clMpe.GetVar(db["J"]));
   EXPECT_FALSE(clMpe.GetVar(db["I"]));
   EXPECT_NEAR(res1->Get(0), 0.23, 0.005);
#endif

   {
      FactorSet fs = fsInitial;

      // another run wit the same model but sample is different
      Clause cSample(db, { { db["Sched"], true },{ db["Drop"], false },
      { db["Alert"], false },{ db["Call"], true } });
      printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());

      fs.PruneEdges(cSample);
      s = fs.GetJson(db);
      printf("\n =After Prune Edges =\n%s\n", s.c_str());

      fs.ApplyClause(cSample);
      s = fs.GetJson(db);
      printf("\n =After apply =\n%s\n", s.c_str());

      // This is MPE case -- calculate maximum 
      // probablity explanation of sample 
      fs.MaximizeVar(db.GetVarSet());
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
   }

   return 0;

}


/** Inject sample with
   - Scheduled Maintainace = TRUE
   - Packet Drop  = FALSE
   - Weather Alert = TRUE
   - Customer Call = TRUE
  <br> Find most likely combination of variables Damage and Outage
 <br>Repeat same test with
   - Scheduled Maintainace = TRUE
   - Packet Drop  = FALSE
   - Weather Alert = FALSE
   - Customer Call = TRUE

*/

int IspTest2()
{

   VarDb db;
   FactorSet fsInitial(db);
   InitIspExample(db, fsInitial);


   std::string s = fsInitial.GetJson(db);
   printf("\n==ISP MAP TEST. Find Most likely values of some variables==\n%s\n", s.c_str());

   // observed sample

   {
      //Run 1
      FactorSet fs = fsInitial;
      Clause cSample(db, {
         { db["Sched"], true },{ db["Drop"], false },
         { db["Alert"], true },{ db["Call"],true }
      });

      printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());

      // Evaluate most likely combination
      VarSet vsMap(db, { db["Damage"], db["Outage"] });
      // all other variables can be eliminated
      VarSet vsEliminate = fs.GetVarSet()->Substract(vsMap);
      // find if any variables that can be pruned 
      VarSet vsPrune = vsMap.Disjuction(cSample.GetVarSet());
      fs.PruneVars(vsPrune);
      fs.PruneEdges(cSample.GetVarSet());
      fs.ApplyClause(cSample);
      fs.EliminateVar(vsEliminate);
      fs.MaximizeVar(vsMap);
      std::shared_ptr<Factor> res1 = fs.Merge();
      VarSet vsMapRes = res1->GetExtendedVarSet();
      InstanceId instanceMap = res1->GetExtendedClause(0);
      Clause clMap(vsMapRes, instanceMap);
      s = clMap.GetJson(db);
      printf("==MAP clause ==\n%s\n", s.c_str());
   }

   {
      FactorSet fs = fsInitial;

      Clause cSample(db, {
         { db["Sched"], true },{ db["Drop"], false },
         { db["Alert"], false },{ db["Call"],true }
      });

      printf("\n== Sample = %s\n", cSample.GetJson(db).c_str());

      // Evaluate most likely combination
      VarSet vsMap(db, { db["Damage"] , db["Outage"] });
      // all other variables can be eliminated
      VarSet vsEliminate = fs.GetVarSet()->Substract(vsMap);
      // find if any variables that can be pruned 
      VarSet vsPrune = vsMap.Disjuction(cSample.GetVarSet());
      fs.PruneVars(vsPrune);
      fs.PruneEdges(cSample.GetVarSet());
      fs.ApplyClause(cSample);
      fs.EliminateVar(vsEliminate);
      fs.MaximizeVar(vsMap);
      std::shared_ptr<Factor> res1 = fs.Merge();
      VarSet vsMapRes = res1->GetExtendedVarSet();
      InstanceId instanceMap = res1->GetExtendedClause(0);
      Clause clMap(vsMapRes, instanceMap);
      s = clMap.GetJson(db);
      printf("==MAP clause ==\n%s\n", s.c_str());

   }

   return 0;

}

/** Calculate decision Matrix for <b> SendCrew </b> decision
    based on visible variables.
    Calculate SendCrew value forsample
    - Scheduled Maintainace = FALSE
    - Packet Drop  = FALSE
    - Weather Alert = FALSE
    - Customer Call = TRUE
*/
int IspDecisionExample()
{
   VarDb db;
   FactorSet fs(db);
   InitIspDecisionExample(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==ISP Decision  ==\n%s\n", s.c_str());

   std::shared_ptr<DecisionBuilderHelper> dh = fs.BuildDecision();
   if (dh)
      s = dh->GetJson(db);
   else
      return -1;

   printf("\n==Isp Decision Result ==\n%s\n", s.c_str());

   if (dh->GetSize() != 1)
      return -1;

   ClauseValue res1 = dh->GetDecisions(Clause(db, {
      { db["Call"] , true },
      { db["Alert"] , true },
      { db["Sched"], false },
      { db["Drop"],false }
   }));

   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["SendCrew"]);
   EXPECT_TRUE(res1[db["SendCrew"]]);
   EXPECT_NEAR(-0.081, res1.GetVal(), 0.01);



   ClauseValue res2 = dh->GetDecisions( Clause(db, { 
                                       { db["Call"] , true }, 
                                       { db["Alert"] , false }, 
                                       { db["Sched"], false }, 
                                       {db["Drop"],false} 
                                                }));

   EXPECT_EQ(res2.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res2.GetVarSet().GetFirst(), db["SendCrew"]);
   EXPECT_FALSE(res2[db["SendCrew"]]);
   EXPECT_NEAR(-2.3, res2.GetVal(), 0.01);


   return 0;
}

/// \}


