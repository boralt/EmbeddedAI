/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/


#include "Factories.h"
#include <json/json.h>

using namespace bayeslib;

static std::string createErrorJson(const char *err)
{
   std::string res = "{\"error\":\"";
   res += err;
   res += "\"}";
   return res;
}

std::string
SessionEntry::RunCommand(std::string sOp)
{
   Json::Value v;
   Json::Reader r;
   if (!r.parse(sOp, v))
   {
      return createErrorJson("parse error");
   }

   VarDb *pVarDb= 0;
   FactorSet *pFs = 0;
   std::string op;

   if (v.isMember("VarDb"))
   {
      pVarDb = VarDbFactory::Create(v["VarDb"]);
   }

   if (!pVarDb)
   {
      return createErrorJson("No Vars");
   }

   VarSet opVarSet(*pVarDb);
   Clause opClause(*pVarDb);


   for (Json::Value::iterator it = v.begin();
      it != v.end(); ++it)
   {
      Json::Value &vf = *it;
      if (it.name() == "VarDb")
      {
         // already done
      }
      else if (it.name() == "FactorSet")
      {
         pFs = FactorSetFactory::Create(*pVarDb, *it);
      }
      else if (it.name() == "QueryVarSet")
      {
         opVarSet = VarSetFactory::Create(*pVarDb, *it);
      }
      else if (it.name() == "SampleClause")
      {
         opClause = ClauseFactory::Create(*pVarDb, *it);
      }
      else if (it.name() == "op")
      {
         op = it->asString();
      }
   }

   if (op.empty())
   {
      return createErrorJson("No operation");
   }

   if (op == "MPE")
   {
      pFs->PruneEdges(opClause);
      pFs->ApplyClause(opClause);
      VarSet vsEliminate = pFs->GetVarSet()->Substract(opClause.GetVarSet());
      pFs->MaximizeVar(vsEliminate);
      std::shared_ptr<Factor> res1 = pFs->Merge();

      std::string sMpeVal = res1->GetJson(*pVarDb);

      VarSet vsMpe = res1->GetExtendedVarSet();
      InstanceId instanceMpe = res1->GetExtendedClause(0);
      Clause clMpe(vsMpe, instanceMpe);
      std::string sMpeClause = clMpe.GetJson(*pVarDb);

      // implement with string buffer more efficient
      std::string sRes = "{";
      sRes += "\"mpe\":";
      sRes += sMpeVal;
      sRes += ",\"clause\":";
      sRes += sMpeClause;
      sRes += "}";
      return sRes;
   }

   if (op == "MAP")
   {
      VarSet vsEliminate = pFs->GetVarSet()->Substract(opVarSet);

      // varset to prune networks
      VarSet vsPrune = opVarSet.Disjuction(opClause.GetVarSet());
      pFs->PruneVars(vsPrune);
      pFs->PruneEdges(opClause.GetVarSet());
      pFs->ApplyClause(opClause);
      pFs->EliminateVar(vsEliminate);
      pFs->MaximizeVar(opVarSet);
      std::shared_ptr<Factor> res1 = pFs->Merge();

      std::string sMpeVal = res1->GetJson(*pVarDb);

      VarSet vsMpe = res1->GetExtendedVarSet();
      InstanceId instanceMpe = res1->GetExtendedClause(0);
      Clause clMpe(vsMpe, instanceMpe);
      std::string sMpeClause = clMpe.GetJson(*pVarDb);

      // implement with string buffer more efficient
      std::string sRes = "{";
      sRes += "\"map\":";
      sRes += sMpeVal;
      sRes += ",\"clause\":";
      sRes += sMpeClause;
      sRes += "}";
      return sRes;
   }
    return createErrorJson("Unsupported operation");

}
