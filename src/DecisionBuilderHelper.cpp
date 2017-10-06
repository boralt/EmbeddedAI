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

std::string 
DecisionBuilderHelper::GetJson(const VarDb &db) const
{
    std::string res;
    res = "{decisions:[ ";
    for(auto iter = mDecisions.begin();
            iter != mDecisions.end(); ++iter)
    {
        res += iter->get()->GetJson(db);
        res += ",";
    }
    res.erase(res.length()-1);
    res += "]}";
    return res;

}

ClauseValue 
DecisionBuilderHelper::GetDecisions(const Clause &sample)
{
	ClauseValue res(sample.GetVarSet().GetDb());
	ValueType val = 0;

	Clause clauseAllData(sample);			// this clause includes sample and decisions
	bool bContinue = true;

	while (bContinue)
	{
		// proceed until can't find more solved decisions
		bContinue = false;

		for (ListDecisions::iterator iter = mDecisions.begin();
			iter != mDecisions.end(); ++iter)
		{
			std::shared_ptr<DecisionFunction> df = *iter;
			
			// entire decision function is resolved by this sample
			if (df->GetVarSet().Conjuction(clauseAllData.GetVarSet()) == df->GetVarSet())
			{
				VarId vidDecisionId = df->GetExtendedVarSet().GetFirst();
				if (res.GetVarSet().HasVar(vidDecisionId))
				{
					// already processed
					continue;
				}

				// fetch decision into return clause and also running clause
				InstanceId instid = clauseAllData.GetInstanceId(df->GetVarSet());
				bool bDecision = df->GetDecision(instid);
				val = df->Get(instid);
				res.AddVar(vidDecisionId, bDecision);
				clauseAllData.AddVar(vidDecisionId, bDecision);
				bContinue = true;
			}

		}
	}

	// last value is what can be consider a utility for this decision list
	res.SetVal(val);
	return res; 
}


std::string 
DecisionBuilderHelper::GetType() const
{
   return "DecisionBuilderHelper";
}
