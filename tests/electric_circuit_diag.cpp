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
 \ingroup exampleCircuit 
 \{
  This example describes Beysian Network that encodes the logic 
  of digital circuit as well as failure probability which allowes it
  to make inferences about state of logical components and values of
  un-observed signals.
  \image html "../../docs/circuit.png"
*/

/**  Initialize model for circuit troubleshooting
     Probabilities of component outages is described
     by probabilities assigned to erroneus outputs
     \image html "../../docs/circuitgraph.png" wudth=60%
*/

int InitCircuitTest(VarDb &db, FactorSet &fs)
{
   /*   Build the troubleshooting Beysian network for circuit

   J --- (NOT) ---Y--|
   |              |
   |-- |          | (OR) ---O--
   | (AND)-X--|
   |          |
   I -----|

   */


   db.AddVar("J");
   db.AddVar("I");
   db.AddVar("Y");
   db.AddVar("X");
   db.AddVar("O");

   // Input I
   VarSet vsI(db);
   vsI << db["I"];
   std::shared_ptr<Factor> fI = std::make_shared<Factor>(vsI, db["I"]);
   fI->AddInstance(0, 0.5F);
   fI->AddInstance(1, 0.5F);

   // Input J
   VarSet vsJ(db);
   vsJ << db["J"];
   std::shared_ptr<Factor> fJ = std::make_shared<Factor>(vsJ, db["J"]);
   fJ->AddInstance(0, 0.5F);
   fJ->AddInstance(1, 0.5F);


   // Output/Input Y
   VarSet vsY(db);
   vsY << db["J"] << db["Y"];
   std::shared_ptr<Factor> fY = std::make_shared<Factor>(vsY, db["Y"]);
   Factor::FactorLoader flY(fY);
   flY << 0.01F << 0.99F << 0.99F << 0.01F;

   // Output/Input X
   VarSet vsX(db);
   vsX << db["I"] << db["J"] << db["X"];
   std::shared_ptr<Factor> fX = std::make_shared<Factor>(vsX, db["X"]);
   Factor::FactorLoader flX(fX);
   flX << 0.95F << 0.95F << 0.95F << 0.05F << 0.05F << 0.05F << 0.05F << 0.95F;

   // Output O
   VarSet vsO(db);
   vsO << db["X"] << db["Y"] << db["O"];
   std::shared_ptr<Factor> fO = std::make_shared<Factor>(vsO, db["O"]);
   Factor::FactorLoader flO(fO);
   flO << 0.98F << 0.02F << 0.02F << 0.02F << 0.02F << 0.98F << 0.98F << 0.98F;

   fs.AddFactor(fI);
   fs.AddFactor(fJ);
   fs.AddFactor(fX);
   fs.AddFactor(fY);
   fs.AddFactor(fO);

   return 0;
}

/** Apply input J=1 to the circuit.
    Input I is unknown. Measured output O is 0.
    Find most likely combination of variables in the model
*/

int TestCircuit1()
{
   VarDb db;
   FactorSet fs(db);
   InitCircuitTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Circuit test with edge Pruning==\n%s\n", s.c_str());


   VarSet vsSample(db);
   vsSample << db["J"] << db["O"];
   Clause cSample(vsSample);

   // observed sample. 
   cSample.SetVar(db["J"], true);
   cSample.SetVar(db["O"], false);


   fs.PruneEdges(cSample);
   s = fs.GetJson(db);
   printf("\n =After Prune Edges =\n%s\n", s.c_str());


   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("\n =After apply =\n%s\n", s.c_str());

   VarSet vsEliminate(db);
   vsEliminate << db["J"] << db["I"] << db["X"] << db["Y"] << db["O"];
   fs.MaximizeVar(vsEliminate);

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

   // Should point to I&J -> X  circuit failure 
   EXPECT_FALSE(clMpe.GetVar(db["X"]));
   EXPECT_TRUE(clMpe.GetVar(db["J"]));
   EXPECT_FALSE(clMpe.GetVar(db["I"]));
   EXPECT_NEAR(res1->Get(0), 0.23, 0.005);

   return 0;
}


/** Inputs I and J  are unknown. Measured output O is 1.
   Find most likely combination of Inputs I and J
*/
int TestCircuit2()
{
   VarDb db;
   FactorSet fs(db);
   InitCircuitTest(db, fs);

   std::string s = fs.GetJson(db);
   printf("\n==Circuit test MAP with NEtworl Pruning==\n%s\n", s.c_str());


   VarSet vsSample(db);
   vsSample << db["O"];
   Clause cSample(vsSample);
   cSample.SetVar(db["O"], true);

   // solve for this variables
   VarSet vsMap(db);
   vsMap << db["I"] << db["J"];

   VarSet vsEliminate = fs.GetVarSet()->Substract(vsMap);

   // varset to prune networks
   VarSet vsPrune = vsMap.Disjuction(vsSample);

   fs.PruneVars(vsPrune);
   s = fs.GetJson(db);
   printf("\n =After Prune Vars =\n%s\n", s.c_str());

   fs.PruneEdges(vsSample);
   s = fs.GetJson(db);
   printf("\n =After Prune Edges =\n%s\n", s.c_str());


   //printf("\n== Sample = %s\n", cSample.GetJson().c_str());
   fs.ApplyClause(cSample);
   s = fs.GetJson(db);
   printf("\n =After apply =\n%s\n", s.c_str());

   fs.EliminateVar(vsEliminate);
   s = fs.GetJson(db);
   printf("\n =After eliminate =\n%s\n", s.c_str());

   fs.MaximizeVar(vsMap);

   s = fs.GetJson(db);
   printf("\n =After MAX up =\n%s\n", s.c_str());

   std::shared_ptr<Factor> res1 = fs.Merge();
   s = res1->GetJson(db);
   printf("\n==After MAP calculation ==\n%s\n", s.c_str());

   VarSet vsMapRes = res1->GetExtendedVarSet();
   InstanceId instanceMap = res1->GetExtendedClause(0);
   Clause clMap(vsMapRes, instanceMap);
   s = clMap.GetJson(db);
   printf("==MAP clause ==\n%s\n", s.c_str());

   EXPECT_TRUE(clMap.GetVar(db["I"]));
   EXPECT_FALSE(clMap.GetVar(db["J"]));
   EXPECT_NEAR(0.24, res1->Get(0), 0.01);

   return 0;
}


/// \}

