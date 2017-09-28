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
/// \ingroup decisionTest
/// \{


/** Initialize Facility Security system Decision Network
  The Audio Alarm can be caused by Fire or Tampering. <br>
  The system can't directly observe Alarm signal but can recive the Report
  of people leaving facility. The system canmake a decision to turn on smoke detection system 
  and evaluate smoke detection output. Secondarily based on Smoke detection and previously
  evaluated Report of people leaving facility the system can make a decision to call 
  Fire Department. 
  /// \image html "../../docs/firealarm.png" width=60%


*/
int InitDecisionTest1(VarDb &db, FactorSet &fs)
{
   db.AddVar("Tampering");
   db.AddVar("Fire");
   db.AddVar("Alarm");
   db.AddVar("Smoke");
   db.AddVar("Leaving");
   db.AddVar("Report");
   db.AddVar("CheckSmoke", VarType_Decision);
   db.AddVar("SeeSmoke");
   db.AddVar("Call", VarType_Decision);
   db.AddVar("Utility", VarType_Utility);

   // Input Tampering
   VarSet vsTampering(db);
   vsTampering << db["Tampering"];
   std::shared_ptr<Factor> fTampering = std::make_shared<Factor>(vsTampering, db["Tampering"]);
   *fTampering << 0.98F << fin ;


   // Input Fire
   VarSet vsFire(db);
   vsFire << db["Fire"];
   std::shared_ptr<Factor> fFire = std::make_shared<Factor>(vsFire, db["Fire"]);
   *fFire << 0.99F << fin;


   // Alarm
   VarSet vsAlarm(db);
   vsAlarm << db["Fire"] << db["Tampering"] << db["Alarm"];
   std::shared_ptr<Factor> fAlarm = std::make_shared<Factor>(vsAlarm, db["Alarm"]);
   *fAlarm << 0.9999F << 0.01F << 0.15F << 0.5F << 0.0001F << 0.99F << 0.85F << 0.5F;

   // People Leaving
   VarSet vsLeaving(db);
   vsLeaving << db["Alarm"] << db["Leaving"];
   std::shared_ptr<Factor> fLeaving = std::make_shared<Factor>(vsLeaving, db["Leaving"]);
   *fLeaving << 0.999F << 0.12F << 0.001F << 0.88F;

   // Report is filed
   VarSet vsReport(db);
   vsReport << db["Leaving"] << db["Report"];
   std::shared_ptr<Factor> fReport = std::make_shared<Factor>(vsReport, db["Report"]);
   *fReport << 0.99F << 0.25F << 0.01F << 0.75F;

   // Smoke
   VarSet vsSmoke(db);
   vsSmoke << db["Fire"] << db["Smoke"];
   std::shared_ptr<Factor> fSmoke = std::make_shared<Factor>(vsSmoke, db["Smoke"]);
   *fSmoke << 0.99F << 0.1F << 0.01F << 0.9F;

   //SeeSmoke
   VarSet vsSeeSmoke(db);
   vsSeeSmoke << db["Smoke"] << db["CheckSmoke"] << db["SeeSmoke"];
   std::shared_ptr<Factor> fSeeSmoke = std::make_shared<Factor>(vsSeeSmoke, db["SeeSmoke"]);
   *fSeeSmoke << 1.0 << 1.0 << 1.0 << 0. << 0. << 0. << 0. << 1.0;

   //CheckSmoke decision factor. Doesn't need probabilities
   VarSet vsCheckSmoke(db);
   vsCheckSmoke << db["Report"] << db["CheckSmoke"];
   std::shared_ptr<Factor> fCheckSmoke = std::make_shared<Factor>(vsCheckSmoke, db["CheckSmoke"]);
   fCheckSmoke->SetFactorType(VarType_Decision);

   //Call decision factor. Doesn't need probabilities
   VarSet vsCall(db);
   vsCall << db["Report"] << db["CheckSmoke"] << db["SeeSmoke"] << db["Call"];
   std::shared_ptr<Factor> fCall = std::make_shared<Factor>(vsCall, db["Call"]);
   fCall->SetFactorType(VarType_Decision);

   // Utility
   VarSet vsUtility(db);
   vsUtility << db["Fire"] << db["CheckSmoke"] << db["Call"];
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   *fUtility << 0. << -5000. << -20. << -5020. << -200. << -200. << -220. << -220.;
   fUtility->SetFactorType(VarType_Utility);

   fs.AddFactor(fTampering);
   fs.AddFactor(fFire);
   fs.AddFactor(fAlarm);
   fs.AddFactor(fLeaving);
   fs.AddFactor(fReport);

   fs.AddFactor(fSmoke);
   fs.AddFactor(fSeeSmoke);

   fs.AddFactor(fCheckSmoke);
   fs.AddFactor(fCall);

   fs.AddFactor(fUtility);


   return 0;
}

/** Build decision matrix and calculate two consequtive decisions
    - Activate Smoke Decisions
    - Cone Fire Department

    Two different situations are analyzed:
     - Report of People leaving the facity 
     - Smoke is detected
*/
int DecisionTest1() {
   VarDb db;
   FactorSet fs(db);
   InitDecisionTest1(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Decision test1  ==\n%s\n", s.c_str());

   std::shared_ptr<DecisionBuilderHelper> dh = fs.BuildDecision();
   if (dh)
      s = dh->GetJson(db);
   else
      return -1;

   printf("\n==Decision test1  Result ==\n%s\n", s.c_str());


   // ClauseValue res1 = dh->GetDecisions(Clause(VarSet({ db["Report"] }), false));
   // more compact method
   ClauseValue res1 = dh->GetDecisions(Clause(db, { { db["Report"] , false } }));

   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_FALSE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-17.58, res1.GetVal(), 0.01);

   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"] }), true));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-5.01, res1.GetVal(), 0.01);


   // Reported but don't See smoke
   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"], db["SeeSmoke"] }), 1));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_FALSE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-3.68, res1.GetVal(), 0.01);


   // Reported and see smoke
   res1 = dh->GetDecisions(Clause(VarSet(db, { db["Report"], db["SeeSmoke"] }), 3));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_TRUE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-1.33, res1.GetVal(), 0.01);

   return 0;
}


/** Initialize Facility Security system Decision Network
  The Audio Alarm can be caused by Fire or Tampering. <br>
  The system can't directly observe Alarm signal but can recive the Report
  of people leaving facility. The system canmake a decision to turn on smoke detection system
  and evaluate smoke detection output. Secondarily based on Smoke detection and previously
  evaluated Report of people leaving facility the system can make a decision to call
  Fire Department.
  /// \image html "../../docs/firealarm.png" width=60%


*/
int InitDecisionTest2(VarDb &db, FactorSet &fs)
{
   db.AddVar("Tampering");
   db.AddVar("Fire");
   db.AddVar("Alarm");
   db.AddVar("Smoke");
   db.AddVar("Leaving", {"None", "Walking", "Running"});
   db.AddVar("Report");
   db.AddVar("CheckSmoke", VarType_Decision);
   db.AddVar("SeeSmoke");
   db.AddVar("Call", VarType_Decision);
   db.AddVar("Utility", VarType_Utility);

   // Input Tampering
   VarSet vsTampering(db);
   vsTampering << db["Tampering"];
   std::shared_ptr<Factor> fTampering = std::make_shared<Factor>(vsTampering, db["Tampering"]);
   *fTampering << 0.98F << fin ;


   // Input Fire
   VarSet vsFire(db);
   vsFire << db["Fire"];
   std::shared_ptr<Factor> fFire = std::make_shared<Factor>(vsFire, db["Fire"]);
   *fFire << 0.99F << fin;


   // Alarm
   VarSet vsAlarm(db);
   vsAlarm << db["Fire"] << db["Tampering"] << db["Alarm"];
   std::shared_ptr<Factor> fAlarm = std::make_shared<Factor>(vsAlarm, db["Alarm"]);
   *fAlarm << 0.9999F << 0.01F << 0.15F << 0.5F << 0.0001F << 0.99F << 0.85F << 0.5F;

   // People Leaving
   VarSet vsLeaving(db);
   vsLeaving << db["Alarm"] << db["Smoke"] << db["Leaving"];
   std::shared_ptr<Factor> fLeaving = std::make_shared<Factor>(vsLeaving, db["Leaving"]);
   *fLeaving << 0.88F << 0.12F   /* Alarm = 0/1 No Smoke, No Leaving   */
             << 0.5F   << 0.1F    /* Alarm 0/1 Smoke, No Leaving  */
             << 0.1F   << 0.6F    /* Alarm 0/1 No Smoke, Leaving Walking  */
             << 0.4F   << 0.4F    /* Alarm 0/1   Smoke, Leaving Walking  */
             << fin;

   // Report is filed
   VarSet vsReport(db);
   vsReport << db["Leaving"] << db["Report"];
   std::shared_ptr<Factor> fReport = std::make_shared<Factor>(vsReport, db["Report"]);
   *fReport << 0.99F << 0.7 <<  0.1F << fin;

   // Smoke
   VarSet vsSmoke(db);
   vsSmoke << db["Fire"] << db["Smoke"];
   std::shared_ptr<Factor> fSmoke = std::make_shared<Factor>(vsSmoke, db["Smoke"]);
   *fSmoke << 0.99F << 0.1F << 0.01F << 0.9F;

   //SeeSmoke
   VarSet vsSeeSmoke(db);
   vsSeeSmoke << db["Smoke"] << db["CheckSmoke"] << db["SeeSmoke"];
   std::shared_ptr<Factor> fSeeSmoke = std::make_shared<Factor>(vsSeeSmoke, db["SeeSmoke"]);
   *fSeeSmoke << 1.0 << 1.0 << 1.0 << 0. << 0. << 0. << 0. << 1.0;

   //CheckSmoke decision factor. Doesn't need probabilities
   VarSet vsCheckSmoke(db);
   vsCheckSmoke << db["Report"] << db["CheckSmoke"];
   std::shared_ptr<Factor> fCheckSmoke = std::make_shared<Factor>(vsCheckSmoke, db["CheckSmoke"]);
   fCheckSmoke->SetFactorType(VarType_Decision);

   //Call decision factor. Doesn't need probabilities
   VarSet vsCall(db);
   vsCall << db["Report"] << db["CheckSmoke"] << db["SeeSmoke"] << db["Call"];
   std::shared_ptr<Factor> fCall = std::make_shared<Factor>(vsCall, db["Call"]);
   fCall->SetFactorType(VarType_Decision);

   // Utility
   VarSet vsUtility(db);
   vsUtility << db["Fire"] << db["CheckSmoke"] << db["Call"];
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   *fUtility << 0. << -5000. << -20. << -5020. << -200. << -200. << -220. << -220.;
   fUtility->SetFactorType(VarType_Utility);

   fs.AddFactor(fTampering);
   fs.AddFactor(fFire);
   fs.AddFactor(fAlarm);
   fs.AddFactor(fLeaving);
   fs.AddFactor(fReport);

   fs.AddFactor(fSmoke);
   fs.AddFactor(fSeeSmoke);

   fs.AddFactor(fCheckSmoke);
   fs.AddFactor(fCall);

   fs.AddFactor(fUtility);


   return 0;
}

/** Build decision matrix and calculate two consequtive decisions
    - Activate Smoke Decisions
    - Cone Fire Department

    Two different situations are analyzed:
     - Report of People leaving the facity
     - Smoke is detected
*/
int DecisionTest2() {
   VarDb db;
   FactorSet fs(db);
   InitDecisionTest2(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Decision test2  ==\n%s\n", s.c_str());

   std::shared_ptr<DecisionBuilderHelper> dh = fs.BuildDecision();
   if (dh)
      s = dh->GetJson(db);
   else
      return -1;

   printf("\n==Decision test1  Result ==\n%s\n", s.c_str());


   // ClauseValue res1 = dh->GetDecisions(Clause(VarSet({ db["Report"] }), false));
   // more compact method
   ClauseValue res1 = dh->GetDecisions(Clause(db, { { db["Report"] , false } }));

   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_FALSE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-22.5, res1.GetVal(), 0.01);

   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"] }), true));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-4.97, res1.GetVal(), 0.01);


   // Reported but don't See smoke
   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"], db["SeeSmoke"] }), 1));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_FALSE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-3.37, res1.GetVal(), 0.01);


   // Reported and see smoke
   res1 = dh->GetDecisions(Clause(VarSet(db, { db["Report"], db["SeeSmoke"] }), 3));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_TRUE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-1.59, res1.GetVal(), 0.01);

   return 0;
}




/** Initialize Facility Security system Decision Network
  The Audio Alarm can be caused by Fire or Tampering. <br>
  The system can't directly observe Alarm signal but can recive the Report
  of people leaving facility. The system canmake a decision to turn on smoke detection system
  and evaluate smoke detection output. Secondarily based on Smoke detection and previously
  evaluated Report of people leaving facility the system can make a decision to call
  Fire Department.
  /// \image html "../../docs/firealarm.png" width=60%


*/
int InitDecisionTest3(VarDb &db, FactorSet &fs)
{
   db.AddVar("Tampering");
   db.AddVar("Fire");
   db.AddVar("Explosion");
   db.AddVar("Alarm");
   db.AddVar("Smoke");
   db.AddVar("Leaving", {"None", "Walking", "Running"});
   db.AddVar("Report");
   db.AddVar("CheckSmoke", VarType_Decision);
   db.AddVar("SeeSmoke");
   db.AddVar("Call", {"None", "Fire", "NSA"}, VarType_Decision);
   db.AddVar("Utility", VarType_Utility);

   // Input Tampering
   VarSet vsTampering(db);
   vsTampering << db["Tampering"];
   std::shared_ptr<Factor> fTampering = std::make_shared<Factor>(vsTampering, db["Tampering"]);
   *fTampering << 0.98F << fin ;


   // Input Fire
   VarSet vsFire(db);
   vsFire << db["Fire"];
   std::shared_ptr<Factor> fFire = std::make_shared<Factor>(vsFire, db["Fire"]);
   *fFire << 0.99F << fin;

   // Input Explosion
   VarSet vsExplosion(db);
   vsFire << db["Explosion"];
   std::shared_ptr<Factor> fExplosion = std::make_shared<Factor>(vsFire, db["Fire"]);
   *fFire << 0.999F << fin;


   // Alarm
   VarSet vsAlarm(db);
   vsAlarm << db["Fire"] << db["Tampering"] << db["Explosion"] << db["Alarm"];
   std::shared_ptr<Factor> fAlarm = std::make_shared<Factor>(vsAlarm, db["Alarm"]);
   *fAlarm << 0.9999F << 0.01F << 0.15F << 0.5F << 0.01F << 0.005F << 0.008 << 0.002F << fin;

   // People Leaving
   VarSet vsLeaving(db);
   vsLeaving << db["Alarm"] << db["Smoke"] << db["Explosion"] << db["Leaving"];
   std::shared_ptr<Factor> fLeaving = std::make_shared<Factor>(vsLeaving, db["Leaving"]);
   *fLeaving << 0.92F << 0.25F  << 0.3F  << 0.15F << 0.8F << 0.1F << 0.2F  << 0.1F
             << 0.07F << 0.7F   << 0.5F  <<  0.7F << 0.1F << 0.7  << 0.4F  << 0.3F
             << fin;

   // Report is filed
   VarSet vsReport(db);
   vsReport << db["Leaving"] << db["Report"];
   std::shared_ptr<Factor> fReport = std::make_shared<Factor>(vsReport, db["Report"]);
   *fReport << 0.99F << 0.7 <<  0.1F << fin;

   // Smoke
   VarSet vsSmoke(db);
   vsSmoke << db["Fire"] << db["Smoke"];
   std::shared_ptr<Factor> fSmoke = std::make_shared<Factor>(vsSmoke, db["Smoke"]);
   *fSmoke << 0.99F << 0.1F << 0.01F << 0.9F;

   //SeeSmoke
   VarSet vsSeeSmoke(db);
   vsSeeSmoke << db["Smoke"] << db["CheckSmoke"] << db["SeeSmoke"];
   std::shared_ptr<Factor> fSeeSmoke = std::make_shared<Factor>(vsSeeSmoke, db["SeeSmoke"]);
   *fSeeSmoke << 1.0 << 1.0 << 1.0 << 0. << 0. << 0. << 0. << 1.0;

   //CheckSmoke decision factor. Doesn't need probabilities
   VarSet vsCheckSmoke(db);
   vsCheckSmoke << db["Report"] << db["CheckSmoke"];
   std::shared_ptr<Factor> fCheckSmoke = std::make_shared<Factor>(vsCheckSmoke, db["CheckSmoke"]);
   fCheckSmoke->SetFactorType(VarType_Decision);

   //Call decision factor. Doesn't need probabilities
   VarSet vsCall(db);
   vsCall << db["Report"] << db["CheckSmoke"] << db["SeeSmoke"] << db["Call"];
   std::shared_ptr<Factor> fCall = std::make_shared<Factor>(vsCall, db["Call"]);
   fCall->SetFactorType(VarType_Decision);

   // Utility
   VarSet vsUtility(db);
   vsUtility << db["Fire"] << db["Explosion"] <<  db["CheckSmoke"] << db["Call"];
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   *fUtility << 0. << -5000. << -20000. << -21000. << -20. << -5020. << -20020. << -21200.
             << -200. << -200. << -10000. << -10200. << -220. << -5220. << -10200. << -10200.
             << -2000. << -4200. << -400. << -600. << -2020. << -2020. << -420. << -620. ;



   fUtility->SetFactorType(VarType_Utility);

   fs.AddFactor(fTampering);
   fs.AddFactor(fFire);
   fs.AddFactor(fAlarm);
   fs.AddFactor(fLeaving);
   fs.AddFactor(fReport);

   fs.AddFactor(fSmoke);
   fs.AddFactor(fSeeSmoke);

   fs.AddFactor(fCheckSmoke);
   fs.AddFactor(fCall);

   fs.AddFactor(fUtility);


   return 0;
}


/** Build decision matrix and calculate two consequtive decisions
    - Activate Smoke Decisions
    - Cone Fire Department

    Two different situations are analyzed:
     - Report of People leaving the facity
     - Smoke is detected
*/
int DecisionTest3() {
   VarDb db;
   FactorSet fs(db);
   InitDecisionTest3(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Decision test3  ==\n%s\n", s.c_str());

   std::shared_ptr<DecisionBuilderHelper> dh = fs.BuildDecision();
   if (dh)
      s = dh->GetJson(db);
   else
      return -1;

   printf("\n==Decision test3  Result ==\n%s\n", s.c_str());


   // ClauseValue res1 = dh->GetDecisions(Clause(VarSet({ db["Report"] }), false));
   // more compact method
   ClauseValue res1 = dh->GetDecisions(Clause(db, { { db["Report"] , false } }));

   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_FALSE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-22.5, res1.GetVal(), 0.01);

   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"] }), true));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckSmoke"]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-4.97, res1.GetVal(), 0.01);


   // Reported but don't See smoke
   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Report"], db["SeeSmoke"] }), 1));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_FALSE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-3.37, res1.GetVal(), 0.01);


   // Reported and see smoke
   res1 = dh->GetDecisions(Clause(VarSet(db, { db["Report"], db["SeeSmoke"] }), 3));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckSmoke"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Call"]));
   EXPECT_TRUE(res1[db["Call"]]);
   EXPECT_TRUE(res1[db["CheckSmoke"]]);
   EXPECT_NEAR(-1.59, res1.GetVal(), 0.01);

   return 0;
}




/// \}


