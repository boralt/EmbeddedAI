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







static ValueType GetUserUtility(ValueType v) {
   if (v < 1.0F) {
      return -8.F;
   } else if (v < 2.0F) {
      return -6.0F;
   } else if (v < 3.0F) {
      return -4.0F;
   } else if (v < 4.0F) {
      return -2.0F;
   } else if (v < 5.0F) {
      return 0.0F;
   } else if (v < 6.0F) {
      return 0.0F;
   } else if (v < 7.0F) {
      return -2.0F;
   } else if (v < 8.0F) {
      return -4.0F;
   } else if (v < 9.0F) {
      return -6.0F;
   } else {
      return -8.F;
   }
}


int InitDecisionTest3(VarDb &db, FactorSet &fs)
{
   // Gaussian 1 = 0.028 + 0.049 + 0.082 + 0.11 + 0.14  +  0.166 + 0.14 + 0.11 + 0.082 + 0.049 + 0.028;
   std::vector<float> arGaussian = { 0.028F, 0.049F, 0.082F, 0.118F, 0.14F,  0.166F, 0.14F, 0.118F, 0.082F, 0.049F, 0.028F };


   db.AddVar("Signal", {"0","1","2","3","4","5","6","7","8","9"});
   //db.AddVar("Noice", {"0","0.2","0.4","0.6","0.8","1.0", "1.2", "1.4","1.6","1.8"});
   db.AddVar("Sensor1", {"0","1","2","3","4","5","6","7","8","9"});
   db.AddVar("CheckExtra", VarType_Decision);
   db.AddVar("Sensor2", {"0","1","2","3","4","5","6","7","8","9"});
   db.AddVar("Act", {"None", "Cool", "Heat"}, VarType_Decision);
   db.AddVar("Utility", VarType_Utility);

   // Signal Distribution
   VarSet vsSignal(db);
   vsSignal << db["Signal"];
   std::shared_ptr<Factor> fSignal = std::make_shared<Factor>(vsSignal, db["Signal"]);
   Factor::FactorLoader flSignal(fSignal);
   for( auto f : arGaussian)
   {
      flSignal << f;
   }

   /*
   // Input Noice is gaussian 0-1 / sigma 0-0.1
   VarSet vsNoice(db);
   vsNoice << db["Noice"];
   std::shared_ptr<Factor> fNoice = std::make_shared<Factor>(vsNoice, db["Noice"]);
   Factor::FactorLoader flNoice(fNoice);
   for( auto f : arGaussian)
   {
      flNoice << f;
   }
   */

   // Sensor1 readout
   VarSet vsSensor1(db);
   vsSensor1 << db["Signal"] << db["Sensor1"];
   std::shared_ptr<Factor> fSensor1 = std::make_shared<Factor>(vsSensor1, db["Sensor1"]);

   Factor::FactorLoader flSensor1(fSensor1);
   for(int nOffs = 0;  nOffs < 10; nOffs++)
   {

      for(int nSignal=0; nSignal < 10; nSignal++)
      {
         ValueType v = 0;
         int nDelta = abs(nSignal-nOffs);
         if (nDelta < 5)
            v = arGaussian[nDelta+5];
         flSensor1 << v ;
      }
   }

   // Sensor2 readout
   VarSet vsSensor2(db);
   vsSensor2 << db["Signal"] <<  db["CheckExtra"] << db["Sensor2"];
   std::shared_ptr<Factor> fSensor2 = std::make_shared<Factor>(vsSensor2, db["Sensor2"]);
   Factor::FactorLoader flSensor2(fSensor2);

   for (int nOffs = 0; nOffs < 10; nOffs++)
   {

      for(int ch = 0; ch < 2; ch++)
      {
         for (int nSignal = 0; nSignal < 10; nSignal++)
         {
            ValueType v = 0;
            int nDelta = abs(nSignal-nOffs);
            if (nDelta < 5)
               v = arGaussian[nDelta+5];
            if (ch == 0)   // if no Extra check, assume sensor is parked at zero
            {
               v = 0.1;
            }
            flSensor2 << v ;
         }
      }
   }


   //CheckExtra decision factor. Doesn't need probabilities
   VarSet vsCheckExtra(db);
   vsCheckExtra << db["Sensor1"] << db["CheckExtra"];
   std::shared_ptr<Factor> fCheckExtra = std::make_shared<Factor>(vsCheckExtra, db["CheckExtra"]);
   fCheckExtra->SetFactorType(VarType_Decision);

   // Act decision factor. Doesn't nned probabilities
   VarSet vsAct(db);
   vsAct << db["Sensor1"] << db["Sensor2"] << db["CheckExtra"] << db["Act"];
   std::shared_ptr<Factor> fAct = std::make_shared<Factor>(vsAct, db["Act"]);
   fCheckExtra->SetFactorType(VarType_Decision);


   VarSet vsUtility(db);
   vsUtility << db["Signal"] <<  db["CheckExtra"] << db["Act"];
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   Factor::FactorLoader flUtility(fUtility);

   for( VarState valAct = 0; valAct < 3; valAct++)
   {
      for (VarState valExtra = 0; valExtra < 2; valExtra++)
      {
         for (int valSignal = 0; valSignal < 10; valSignal++)
         {
            ValueType valUtility = GetUserUtility(valSignal);
            ValueType expectedSignal = valSignal;
            if(valAct == 1)
            {
               expectedSignal -= 2.0;
            }
            else if (valAct == 2)
            {
               expectedSignal += 2.0;
            }

            valUtility += GetUserUtility(expectedSignal);


            if(valAct == 1)
            {
               valUtility -= 1.5F;    // cooling is expensive
            }
            else if(valAct == 2)
            {
               valUtility -= 1.0F;    // heating is less expensive
            }

            if (valExtra)
            {
               valUtility -= 3.0F;  // obtaining second reading is expensive
            }
            flUtility << valUtility;
         }
      }
   }

   fUtility->SetFactorType(VarType_Utility);

   fs.AddFactor(fSensor1);
   fs.AddFactor(fSensor2);
   fs.AddFactor(fCheckExtra);
   fs.AddFactor(fAct);
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
   ClauseValue res1 = dh->GetDecisions(Clause(db, { { db["Sensor1"] , 3 } }));

   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckExtra"]);
   EXPECT_FALSE(res1[db["CheckExtra"]]);
   EXPECT_NEAR(-5.23, res1.GetVal(), 0.01);

   res1 = dh->GetDecisions(Clause(db, { { db["Sensor1"] , 8 } }));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 1);
   EXPECT_EQ(res1.GetVarSet().GetFirst(), db["CheckExtra"]);
   EXPECT_TRUE(res1[db["CheckExtra"]]);
   EXPECT_NEAR(-5.09, res1.GetVal(), 0.01);



   res1 = dh->GetDecisions(Clause(VarSet(db,{ db["Sensor1"], db["Sensor2"] }), 3*10+4));
   EXPECT_EQ(res1.GetVarSet().GetSize(), 2);
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["CheckExtra"]));
   EXPECT_TRUE(res1.GetVarSet().HasVar(db["Act"]));
   EXPECT_FALSE(res1[db["CheckExtra"]]);
   EXPECT_EQ(res1[db["Act"]], 0);
   EXPECT_NEAR(-0.498, res1.GetVal(), 0.01);



   return 0;
}




/// \}


