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


FactorSet::FactorSet(VarDb &db) : mDb(db), mDebugLevel(0)
{

}


void 
FactorSet::AddFactor(std::shared_ptr<Factor> f)
{
   mFactors.push_back(f);
}

// multiply all variables
std::shared_ptr<Factor> 
FactorSet::Merge()
{
    std::shared_ptr<Factor> res;
    bool bFirst = true;


    for(ListFactors::iterator iter = mFactors.begin();
        iter != mFactors.end(); ++iter)
    {

        if(bFirst)
        {
            res = *iter;
            bFirst = false;
        }
        else
        {
            res = res->Merge(*iter);
        }

    }

    if (!res)
       res = std::make_shared<Factor>(VarSet(mDb)); 
    return res;
}


std::shared_ptr<VarSet>
FactorSet::GetTypedVarSet(VarId vid, VarType vartype, VarDb &db)
{
	std::shared_ptr<VarSet> res = GetAncestors(vid);
	VarSet res2 = res->FilterVarSet(vartype);
		
	return res;
}

std::shared_ptr<VarSet> 
FactorSet::GetVarSet() const
{
	std::shared_ptr<VarSet> res(new VarSet(mDb));

	for (auto iter = mFactors.begin();
		iter != mFactors.end(); ++iter)
	{
		const VarSet &vs = iter->get()->GetVarSet();
		res->MergeIn(vs);

		const VarSet & vsHead = iter->get()->GetClauseHead();
		res->MergeIn(vsHead);
      //const VarSet &vs2 = iter->get()->GetClauseHead();
      //res->MergeIn(vs2);
	}

	return res;
}

std::shared_ptr<VarSet> 
FactorSet::GetAncestors(VarId vid)
{
	Factor *pTestFactor = 0;

	for (ListFactors::iterator iter = mFactors.begin();
		iter != mFactors.end(); ++iter)
	{
		const VarSet &vs = iter->get()->GetClauseHead();
		if (vs.HasVar(vid))
		{
			pTestFactor = iter->get();
			break;
		}
	}



	std::shared_ptr<VarSet> res(new VarSet(mDb));

	if (!pTestFactor)
	{
		return res;
	}

	// Walk up the tree
	VarSet vsLookup = pTestFactor->GetVarSetTail();
	res->MergeIn(vsLookup);

	// B.A. need optimization not to run through the Vars already in the varset 
	for (VarId v = vsLookup.GetFirst(); v != 0; v = vsLookup.GetNext(v))
	{
		res->MergeIn(*(GetAncestors(v).get()));
	}
	return res;
}


std::shared_ptr<VarSet>
FactorSet::GetLeafNodes() const
{

	VarSet fsNonLeafs(mDb);
	for (auto iter = mFactors.begin();
		iter != mFactors.end(); ++iter)
	{
		fsNonLeafs.Add(iter->get()->GetVarSetTail());
	}

	std::shared_ptr<VarSet> res(new VarSet(GetVarSet()->Substract(fsNonLeafs)));
	return res;
}


std::shared_ptr<VarSet>
FactorSet::GetAncestors(VarId vid, VarType vartype)
{
	std::shared_ptr<VarSet> res = GetAncestors(vid);
	VarSet res2 = res->FilterVarSet(vartype);
		
	return std::make_shared<VarSet>(res2);
}



void  
FactorSet::EliminateVar(const VarSet &vs)
{
   std::string s = vs.GetJson(mDb);
	if(mDebugLevel >= DebugLevel_Details) 
	{
      std::string s = vs.GetJson(mDb);
		printf("===Eliminate Vars %s ===\n", s.c_str());
	}

    for(VarId id = vs.GetFirst(); id != 0; id = vs.GetNext(id))
    {
        FactorSet fs(mDb);
        for(ListFactors::iterator iter = mFactors.begin();
            iter != mFactors.end(); )
        {
            std::shared_ptr<Factor> f = *iter;
            if(f->GetFactorType() != VarType_Decision && f->GetVarSet().HasVar(id))
            {
                fs.AddFactor(f);
                iter = mFactors.erase(iter);
            } 
            else
            {
                ++iter;
            }
        }

        if (fs.IsEmpty())
        {
           continue;
        }

        std::shared_ptr<Factor> f = fs.Merge();

        //std::string s = f->GetJson();
        //printf("===SubMerge %d ===\n%s\n", id, s.c_str());

        std::shared_ptr<Factor> f2 = f->EliminateVar(id);
        // s = f2->GetJson();
        // printf("===SubEliminate %d ===\n%s\n", id, s.c_str());


        mFactors.push_back(f2);

		if (mDebugLevel >= DebugLevel_Details)
		{
			std::string s = this->GetJson(mDb);
			printf("===Eliminate %d ===\n%s\n", id, s.c_str());
		}


        // mFactors.push_back(f);
    }     
    // done
}

void 
FactorSet::MaximizeVar(const VarSet &vs)
{
   for (VarId id = vs.GetFirst(); id != 0; id = vs.GetNext(id))
   {
      FactorSet fs(mDb);
      for (ListFactors::iterator iter = mFactors.begin();
         iter != mFactors.end(); )
      {
         if (iter->get()->GetVarSet().HasVar(id))
         {
            fs.AddFactor(*iter);
            iter = mFactors.erase(iter);
         }
         else
         {
            ++iter;
         }
      }

      if (fs.IsEmpty())
      {
         continue;
      }

      std::shared_ptr<Factor> f = fs.Merge();

      // std::string s = f->GetJson();
      // printf("===SubMerge %d ===\n%s\n", id, s.c_str());

      std::shared_ptr<Factor> f2 = f->MaximizeVar(id);
      //std::string s = f2->GetJson(GetDb());
      //printf("===SubEliminate %d ===\n%s\n", id, s.c_str());


      mFactors.push_back(f2);

      //s = this->GetJson(GetDb());
      //printf("===Eliminate %d ===\n%s\n", id, s.c_str());


      // mFactors.push_back(f);
   }
   // done
}

std::shared_ptr<DecisionBuilderHelper> 
FactorSet::BuildDecision()
{
	std::shared_ptr<DecisionBuilderHelper> pDecisionHelper = std::make_shared<DecisionBuilderHelper>();

   // Find Utility Variable
   std::shared_ptr<VarSet> vsAll = GetVarSet();
   VarSet vs = vsAll->FilterVarSet(VarType_Utility);
   int nError = 0;

   //printf("VarSet all: %s\n", vsAll->GetJson(db).c_str());
   //printf("VarSet util: %s\n", vs.GetJson(db).c_str());


   // only operate on graph with single utility node
   if(vs.GetSize() != 1)
   {
      return pDecisionHelper;
   }




   VarId varUtility = vs.GetFirst();
   std::shared_ptr<VarSet> vsUtilityAnsestors = 
                           GetAncestors(varUtility);   
   VarSet vsToRemove = vsAll->Substract(*vsUtilityAnsestors);
   vsToRemove = vsToRemove.Substract(VarSet(mDb,varUtility));
   
   // 1. Remove variables that are not ancestors of utility
   if(vsToRemove.GetSize() > 0)
		RemoveVars(vsToRemove);	

   // 2.1 Build Varsets
   VarSet vsDecisions(mDb);
   VarId  vidUtility = 0; 

   // Calculate important varsets
   for(ListFactors::iterator iter = mFactors.begin();
                iter != mFactors.end(); ++iter)
   {
      VarSet vsHead = (*iter)->GetClauseHead();
      if(vsHead.GetSize() == 1)
      {
         VarId vid = vsHead.GetFirst();
         VarType vtype = mDb.GetVarType(vid);
         if (vtype == VarType_Decision)
         {
            vsDecisions.Add(vid);
         }
         else if (vtype == VarType_Utility)
         {
            //vidUtility = vid;
            if(!nError)
              nError = 1;
         }
      }
   }

   // calculate order of decision variables
   int nDecisions = vsDecisions.GetSize(); 
   std::vector<VarId> vDecisionOrder(nDecisions, 0);

   for(VarId vDecision = vsDecisions.GetFirst(); 
      vDecision != 0; 
            vDecision = vsDecisions.GetNext(vDecision))
   {
	   std::shared_ptr<VarSet> vsOtherDecisions = GetAncestors(vDecision, VarType_Decision);
      int nNumAncestorDecisions = vsOtherDecisions->GetSize();
      
	  // decisions should be sequential
      if(vDecisionOrder[nNumAncestorDecisions] )
         if (!nError)
            nError = 2;
      vDecisionOrder[nNumAncestorDecisions] = vDecision;
   }


   // iterative sequence to build decision tree
   for(int nDecisionOrder=(nDecisions-1);
               nDecisionOrder>=0; nDecisionOrder--)
   {
      // recalculate remaining VarSet
      vsAll = GetVarSet();
      VarId vidDecision = vDecisionOrder[nDecisionOrder];
      // B.A. error in graph structure
	  if (mDebugLevel >= DebugLevel_Details)
	  {
		  printf("Resolve Decision %d\n", vidDecision);
	  }
      if(vidDecision == 0)
         continue;

      VarSet vsRetain(mDb);
      for(ListFactors::iterator iter = mFactors.begin();
                iter != mFactors.end(); ++iter)
      {
         VarSet vsHead = (*iter)->GetClauseHead();
         if(vsHead.GetSize() == 1)
         {
            VarId vid = vsHead.GetFirst();
            VarType vtype = mDb.GetVarType(vid);
            if (vtype == VarType_Decision)
            {
               // parents of decision nodes should be retained
			   const VarSet &rVs = (*iter)->GetVarSet();
               vsRetain.MergeIn(rVs);
			   vsRetain.MergeIn(vsHead);
            }
            // vsRetain.Add(vidUtility);
         }
      }

      // eliminate all non-parents of decision variables   
      VarSet vsEliminate = vsAll->Substract(vsRetain);
      EliminateVar(vsEliminate);  

      // Find a factor with varUtility -- there should be
      // only one
      for(ListFactors::iterator iter2 = mFactors.begin();
                iter2 != mFactors.end(); ++iter2)
      {
         if((*iter2)->GetClauseHead().HasVar(varUtility))
         {
            printf("!!!! Actually running maximizer for decision %d %s!!!!\n", vidDecision, mDb[vidDecision].c_str());

            std::shared_ptr<Factor> pFactor = (*iter2);
            //std::string sDebug1 = pFactor->GetJson(mDb);
            //printf("\n Interm quest: %s", sDebug1.c_str());


            std::shared_ptr<Factor> pResult =  pFactor->MaximizeVar(vidDecision);

            //std::string sDebug2 = pResult->GetJson(mDb);
            //printf("\n Interm result: %s\n", sDebug2.c_str());

            // pResult incrporates decision Function and new Factor
            // separate both

            const VarSet &pNextVs = pResult->GetVarSet();
            std::shared_ptr<DecisionFunction> pDecisionFunction(
               new DecisionFunction(pResult, vidDecision));
            // form new DecisionFunction
            pDecisionHelper->AddDecisionFunction(pDecisionFunction);
            // Remove maxed VarId from pResult and insert this 
            // factor into this FactorSet
            pResult->EraseExtendedInfo();
            mFactors.erase(iter2);
            mFactors.push_back(pResult);
			break;
         }
      }

	  // Also remove decision factor vidDecision
	  for (ListFactors::iterator iter2 = mFactors.begin();
		  iter2 != mFactors.end(); ++iter2)
	  {
		if ((*iter2)->GetClauseHead().HasVar(vidDecision))
		{
			iter2 = mFactors.erase(iter2);
			break;
		}
	  }
   }    

   return pDecisionHelper;

   //VarSet vsDecisions = vsAll.FilterVarSet(db, VarType_Utility);
}




void 
FactorSet::RemoveVars(const VarSet &vs)
{
    for(ListFactors::iterator iter = mFactors.begin();
                iter != mFactors.end(); )
    {
        if ((*iter)->GetClauseHead().HasVar(vs))            
        {
            iter = mFactors.erase(iter);
        } 
        else
        {
            ++iter;
        }
    }
} 

void
FactorSet::PruneVars(const VarSet &vs)
{

   bool bPruned = true;
   while(bPruned)
   {
	  auto vsLeafNodes = GetLeafNodes();

      bPruned = false;
      for(ListFactors::iterator iter = mFactors.begin();
          iter != mFactors.end(); )
      {
         std::shared_ptr<Factor> &f = *iter;
         VarSet vsF = f->GetClauseHead();
         if (!vsF.HasVar(vs) && vsLeafNodes->HasVar(vsF) )
         {
            bPruned = true;
            iter = mFactors.erase(iter);
         }
         else
         {
            ++iter;
         }
      }
   }
}


void
FactorSet::PruneEdges(const Clause &c)
{
   VarSet vs = c.GetVarSet();
   ListFactors tmpListNewFactors;

   for (ListFactors::iterator iter = mFactors.begin();
      iter != mFactors.end(); )
   {
      std::shared_ptr<Factor> pFactor = *iter;
      VarSet vsTail = pFactor->GetVarSetTail();

      if (vsTail.HasVar(vs))
      {
         std::shared_ptr<Factor> newFactor = pFactor;
         iter = mFactors.erase(iter);
         for (VarId v = vs.GetFirst(); v != 0; v = vs.GetNext(v))
         {
            if (vsTail.HasVar(v))

            {
               newFactor = newFactor->PruneEdge(v, c.GetVar(v));
            }
         }
        
         tmpListNewFactors.push_back(newFactor);
      }
      else
      {
         ++iter;
      }
   }
   mFactors.insert(mFactors.begin(), tmpListNewFactors.begin(), tmpListNewFactors.end());

}



void 
FactorSet::ApplyClause(const Clause &c)
{

    ListFactors tmpF;

    for(ListFactors::iterator iter = mFactors.begin();
                iter != mFactors.end(); ++iter)
    {
        std::shared_ptr<Factor>  &f(*iter);
        if(f->GetVarSet().HasVar(c.GetVarSet()))
        {
            std::shared_ptr<Factor> f1 = f->ApplyClause(c);
            if(!f1->IsEmpty())
               tmpF.push_back(f1);
        }
        else
        {
            tmpF.push_back(f);
        }
    }        

    mFactors = tmpF;

}


std::string 
FactorSet::GetJson(const VarDb &db) const
{
    std::string res;
    res = "{factors:[ ";
    for(auto iter = mFactors.begin();
            iter != mFactors.end(); ++iter)
    {
        res += iter->get()->GetJson(db);
        res += ",";
    }
    res.erase(res.length()-1);
    res += "]}";
    return res;

}

std::string 
FactorSet::GetType() const
{
    return "FactoRest";
}
