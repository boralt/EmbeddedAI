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

char *
dumpbinary(char *sz, size_t lenSz, InstanceId v, int sizeVarSet)
{
   InstanceId mask = 1;
   size_t i =0;
   for (i = 0; i < sizeVarSet && i < (lenSz-1); i++, mask <<=1)
   {
      if (v & mask)
         sz[i] = '1';
      else
         sz[i] = '0';
    }
   sz[i] = 0;
   return sz;
}

void
Factor::Init()
{
	mFactorType = VarType_Normal;

    mFactorSize = mSet.GetInstances();
    int index= 0;
    for(VarId id=mSet.GetFirst(); id != 0; id = mSet.GetNext(id))
    {        
        mBsPresent.set(id);
        mVarToIndex[id] = index;
        index++;        
    }
    mValues.resize(mFactorSize, 0.0F);
    mValuePresent.resize(mFactorSize, false);
}

Factor::Factor(const VarSet &vset) : 
    mSet(vset)
{
    Init();
}

Factor::Factor(const VarSet &vset, VarId clauseHead) : 
    mSet(vset)
{
    Init();
    mClauseHead.Add(clauseHead);
}



Factor::Factor(const VarSet &vset, VarSet clauseHead) : 
    mSet(vset), mClauseHead(clauseHead)
{
    Init();
}


Factor::Factor(std::initializer_list<VarId> varset, std::initializer_list<VarId> clauseHead) :
	mSet(varset), mClauseHead(clauseHead)
{
   Init();
}

Factor::FactorLoader Factor::operator << (ValueType v)
{
	FactorLoader fl( shared_from_this());
	fl << v;
	return std::move(fl) ;
}


void Factor::CompleteProbabilities()
{
	VarSet vsTail = mSet.Substract(mClauseHead);
	Clause cTail(vsTail);

	do
	{
		Clause cHead(mClauseHead);
		ValueType v = 0.;
		bool bUpdateClauseFound = false;    // found unidentified member in the sequence
		Clause cFullUpdateClause;
		do
		{
			Clause tryClause = Clause::Append(mSet, cHead, cTail);
			if (HasVal(tryClause.GetInstanceId()))
			{
				v += Get(tryClause.GetInstanceId());
			}
			else
			{
				if (!bUpdateClauseFound)
				{
					bUpdateClauseFound = true;
					cFullUpdateClause = tryClause;
				}
			}

		} while (!cHead.Incr());

		if (v < 1.0 && bUpdateClauseFound)
		{
			v = 1.0 - v;
			AddInstance(cFullUpdateClause.GetInstanceId(), v);
		}

	} while (!cTail.Incr());

}

void
Factor::AddInstance(InstanceId instance, ValueType val)
{
    if (instance < mFactorSize)
    {
        mValues[instance] = (ValueType) val;
        mValuePresent[instance] = true;
    }
}


bool 
Factor::HasVal(InstanceId id)
{
    if (id < mFactorSize)
        return mValuePresent[id]; 
    return false;   
}

ValueType Factor::Get(InstanceId id)
{
    if(HasVal(id))
        return mValues[id];
    else
        return 0; 
}


std::shared_ptr<Factor> 
Factor::Merge(std::shared_ptr<Factor> f)
{
   FactorMergeHelper h(GetVarSet(), f->GetVarSet());
   VarSet vsTail1 = GetVarSetTail();
   VarSet vsTail2 = f->GetVarSetTail();

   VarSet vsHead = GetClauseHead().Disjuction(f->GetClauseHead());
   vsHead = vsHead.Substract(vsTail1);
   vsHead = vsHead.Substract(vsTail2);

    std::shared_ptr<Factor> res(new Factor(h.mVr, vsHead));
    VarSet newExtendedVs = GetExtendedVarSet().Disjuction(f->GetExtendedVarSet());
    res->SetExtendedVarSet(newExtendedVs);

    InstanceId maxRes = h.mVr.GetInstances();
    for(InstanceId i = 0; i < maxRes; i++)
    {
        InstanceId id1=0;
        InstanceId id2=0;
        int nSize = h.mVr.GetSize();
        for(int n=0; n<nSize; n++)
        {
            int nOffs1 = h.MapOffsRTo1(n);
            int nOffs2 = h.MapOffsRTo2(n);
            bool bSetInRes = ((i & (1ULL << n)) != 0);
            
            if(bSetInRes && nOffs1 >=0)
                id1 |= (1ULL << nOffs1);

            if(bSetInRes && nOffs2 >=0)
                id2 |= (1ULL << nOffs2);
        }
        ValueType v = Get(id1) * f->Get(id2);
        res->AddInstance(i, v);
        Clause newExtendedClause(newExtendedVs);
        Clause cl1(GetExtendedVarSet(), GetExtendedClause(id1));
        Clause cl2(f->GetExtendedVarSet(), f->GetExtendedClause(id2));

        for (VarId id1 = cl1.GetVarSet().GetFirst();
           id1 != 0; id1 = cl1.GetVarSet().GetNext(id1))
        {
           newExtendedClause.SetVar(id1, cl1.GetVar(id1));
        }

        for (VarId id2 = cl2.GetVarSet().GetFirst();
           id2 != 0; id2 = cl2.GetVarSet().GetNext(id2))
        {
           newExtendedClause.SetVar(id2, cl2.GetVar(id2));
        }

        res->AddExtendedClause(i, newExtendedClause.GetInstanceId());
    }
    return res;
}

std::shared_ptr<Factor> 
Factor::EliminateVar(VarId id)
{
    if (!mBsPresent[id])
    {
        // variable is not present
        return shared_from_this();
    }

    VarSet vsEliminate;
    vsEliminate.Add(id);

    VarSet vsResTail = mSet.Substract(vsEliminate);
    VarSet vsResHead = mClauseHead.Substract(vsEliminate);
   

    //for( VarId resId = mSet.GetFirst(); resId != 0; resId = mSet.GetNext(resId))
    //{
    //    // not eliminated var 
    //    if (resId != id)
    //        vsRes.Add(resId);
    //}

    std::shared_ptr<Factor> res(new Factor(vsResTail, vsResHead));
    InstanceId nEliminateBit = 1ULL << mVarToIndex[id];
    InstanceId nRightPart = nEliminateBit - 1;
    InstanceId nShiftMask  = ~(nRightPart | nEliminateBit);

    for(InstanceId nLoop=0; nLoop < vsResTail.GetInstances(); nLoop++)
    {
        // translate new factor instance into original instances
        InstanceId nOrigInstance0 = ((nLoop << 1) & nShiftMask) | (nLoop & nRightPart) ;
        InstanceId nOrigInstance1 = nOrigInstance0 | nEliminateBit ;
            
        ValueType val = mValues[nOrigInstance0] + mValues[nOrigInstance1];
        res->AddInstance(nLoop, val);
    }
    return res;
}

std::shared_ptr<Factor> 
Factor::EliminateVar(const VarSet &ids)
{
    std::shared_ptr<Factor> res = shared_from_this();
    for(VarId id = ids.GetFirst(); id!=0; id = ids.GetNext(id))
    {
        res = res->EliminateVar(id);
    }
    return res;
}


std::shared_ptr<Factor> 
Factor::PruneEdge(VarId v, bool val)
{
   VarSet newVs = mSet.Substract(v);
   std::shared_ptr<Factor> res = std::make_shared<Factor>(newVs);
   Clause clNew(newVs);
   Clause clOld(mSet);

   do
   {
      for (VarId v = newVs.GetFirst(); v != 0; v = newVs.GetNext(v))
      {
         
         clOld.SetVar(v, clNew.GetVar(v));
      }
      clOld.SetVar(v, val);
      res->AddInstance(clNew.GetInstanceId(), Get(clOld.GetInstanceId()));

   } while (!clNew.Incr());
   return res;
}

std::shared_ptr<Factor> 
Factor::ApplyClause(const Clause &c)
{
    // varset for new Factor
    VarSet vsNew = mSet.Substract(c.GetVarSet());
    VarSet vsHeadNew = mClauseHead.Substract(c.GetVarSet());
    
    // part of input clause c that applies to this Factor
    VarSet vsAply = mSet.Conjuction(c.GetVarSet());   
    
    std::shared_ptr<Factor> res = std::make_shared<Factor>(mSet, mClauseHead); 

    Clause resClause(vsNew);
    do 
    {
       Clause old = Clause::Append(mSet, resClause, c);
       res->AddInstance(old.GetInstanceId(), Get(old.GetInstanceId()));        

    } while(!resClause.Incr());

    return res;
}

std::shared_ptr<Factor> 
Factor::Normalize()
{
   VarSet vsTail = mSet.Substract(mClauseHead);
   Clause cHead(mClauseHead);
   std::shared_ptr<Factor> resFactor = std::make_shared<Factor>(mSet, mClauseHead);

   do
   {
      Clause cTail(vsTail);
      ValueType v = 0.;
      do
      {
         Clause res = Clause::Append(mSet, cHead, cTail);
         v += Get(res.GetInstanceId());
      } while (!cTail.Incr());

      if (v == 0.)
         v = 1.;
      
      do
      {
         Clause res = Clause::Append(mSet, cHead, cTail);
         resFactor->AddInstance(res.GetInstanceId(), Get(res.GetInstanceId()) / v);
      } while (!cTail.Incr());
      
   } while (!cHead.Incr());
   return resFactor;
}

std::shared_ptr<Factor> 
Factor::MaximizeVar(VarId id)
{
   if (!mBsPresent[id])
   {
      // variable is not present
      return shared_from_this();
   }

   VarSet vsEliminate;
   vsEliminate.Add(id);

   VarSet vsResTail = mSet.Substract(vsEliminate);
   VarSet vsResHead = mClauseHead.Substract(vsEliminate);


   //for( VarId resId = mSet.GetFirst(); resId != 0; resId = mSet.GetNext(resId))
   //{
   //    // not eliminated var 
   //    if (resId != id)
   //        vsRes.Add(resId);
   //}

   std::shared_ptr<Factor> res(new Factor(vsResTail, vsResHead));
   InstanceId nEliminateBit = 1ULL << mVarToIndex[id];
   InstanceId nRightPart = nEliminateBit - 1;
   InstanceId nShiftMask = ~(nRightPart | nEliminateBit);

   VarSet newExtendedVs = mExtendedVarSet;
   newExtendedVs.Add(id);
   res->SetExtendedVarSet(newExtendedVs);

   for (InstanceId nLoop = 0; nLoop < vsResTail.GetInstances(); nLoop++)
   {
      // translate new factor instance into original instances
      InstanceId nOrigInstance0 = ((nLoop << 1) & nShiftMask) | (nLoop & nRightPart);
      InstanceId nOrigInstance1 = nOrigInstance0 | nEliminateBit;

      ValueType val = 0.;

      if (mValues[nOrigInstance0] > mValues[nOrigInstance1])
      {
         res->AddInstance(nLoop, mValues[nOrigInstance0]);
         Clause cl(newExtendedVs, GetExtendedClause(nOrigInstance0));
         cl.SetVar(id, false);
         res->AddExtendedClause(nLoop, cl.GetInstanceId());
      }
      else
      {
         res->AddInstance(nLoop, mValues[nOrigInstance1]);
         Clause cl(newExtendedVs, GetExtendedClause(nOrigInstance1));
         cl.SetVar(id, true);
         res->AddExtendedClause(nLoop, cl.GetInstanceId());
      }
   }
   return res;
}

void 
Factor::AddExtendedClause(InstanceId instance, InstanceId extendedInstance)
{
   if(mExtendedClauseVector.size() <= extendedInstance)
      mExtendedClauseVector.resize(mSet.GetInstances());
   mExtendedClauseVector[instance] = extendedInstance;
}

InstanceId 
Factor::GetExtendedClause(InstanceId instance)
{

   if(mExtendedClauseVector.size() <= instance)
      return 0;
   return mExtendedClauseVector[instance];
}

void
Factor::EraseExtendedInfo()
{
   mExtendedClauseVector.clear();
   mExtendedVarSet = VarSet();
}



std::string 
Factor::GetJson(VarDb &db) const
{
   std::string s;
   s = "{varset:";
   s += mSet.GetJson(db);
   s += ",extset:";
   s += mExtendedVarSet.GetJson(db);
   s += ",vals:[ ";

   for(InstanceId id = 0; id < mFactorSize; id++)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%f", mValues[id] );
      s += sz;
      s += ",";
   }   
   s.erase(s.length()-1);
   s += "],";

   s += "extvals:[ ";

   for (InstanceId id = 0; id < mExtendedClauseVector.size(); id++)
   {
      char sz[64];
      dumpbinary(sz, sizeof(sz), mExtendedClauseVector[id], mExtendedVarSet.GetSize());
      s += sz;
      s += ",";
   }
   s.erase(s.length() - 1);
   s += "]}";
 
   return s;
}

std::string 
Factor::GetType() const
{
    return "Factor";
}

Factor::FactorLoader::FactorLoader(std::shared_ptr<Factor> f) :
      mClause(f->GetVarSet()), mFactor(f)
{

}


Factor::FactorLoader &
Factor::FactorLoader::operator <<(Factor::FactorLoaderManipulator man)
{
   man(mFactor);
   return *this;
}


Factor::FactorLoader &
Factor::FactorLoader::operator <<(ValueType v)
{
   mFactor->AddInstance(mClause.GetInstanceId(), v);
   mClause.Incr();
   return *this;
}



