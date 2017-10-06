/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/

#include "factor.h"
#include "json/json.h"

using namespace bayeslib;

DecisionFunction::DecisionFunction(const VarSet &vset) :
	Factor(vset)
{


}


DecisionFunction::DecisionFunction(std::shared_ptr<Factor> f, VarId decisionNode) :
	Factor(*f.get()), mDecisionNode(decisionNode)
{


}


VarState
DecisionFunction::GetDecision(InstanceId id)
{
   if(mExtendedVarSet.GetSize() == 1)
   {
      // Shortcut case
      return (VarState) mExtendedClauseVector[id];
   }
   else
   {
      return mExtendedVarSet.FetchVarState(mDecisionNode, id);
   }
}

std::string 
DecisionFunction::GetJson(const VarDb &db) const
{
   std::string s;
   s = "{varset:";
   s += mSet.GetJson(db);
   s += ",decisionVar:";
   s += db[mDecisionNode];
   s += ",vals:[ ";

   for(InstanceId id = 0; id < mFactorSize; id++)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%d", (int) mExtendedClauseVector[id]);
      s += sz;
      s += ",";
   }   
   s.erase(s.length()-1);
   s += "], utility:[";
   for (InstanceId id = 0; id < mFactorSize; id++)
   {
	   char sz[20];
	   snprintf(sz, sizeof(sz), "%f", mValues[id]);
	   s += sz;
	   s += ",";
   }
   s += "]}";
   return s;
}

std::string 
DecisionFunction::GetType() const
{
	return "DecisionFunction";
}
