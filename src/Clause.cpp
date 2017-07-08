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
#include <cassert>


using namespace bayeslib;

static 
bool ValToBool(const Json::Value &v)
{
   if(v.isBool())
   {
      return v.asBool();
   }
   else if(v.isInt())
   {
      return v.asInt() > 0;
   }
   else
      return false;
}

Clause::Clause() :
   mInstanceId(0)
{
}


Clause::Clause(const VarSet &vs) :
   mInstanceId(0), mVarSet(vs)
{

}

Clause::Clause(const VarSet &vs, Json::Value &v) :
   mInstanceId(0), mVarSet(vs)
{
   Json::ArrayIndex i = 0;
   for(VarId id = mVarSet.GetFirst();
      id != 0;
      (id=mVarSet.GetNext(id)), ++i)   
   {
      
      if(i < v.size())
      {
         bool b = ValToBool(v[i]);
         mClause[id] = b;
         if(b)
            mInstanceId += ((InstanceId) 1 << i);
      }
   }
}

Clause::Clause(std::initializer_list<ClauseInitializer> initlist)
{
   for (auto iter = initlist.begin(); iter != initlist.end(); ++iter)
   {
      AddVar(iter->varid, iter->bState);
   }
}


void 
Clause::SetVar(VarId id, bool bVal)
{
   assert(id>0);
   bool bCurVal = mClause[id];
   int nOffs = mVarSet.GetOffs(id);

   assert(nOffs >= 0);
   if( nOffs < 0)
      return;

   if(bCurVal && !bVal)
   {
      mClause.reset(id);
      mInstanceId &= ~( ((InstanceId) 1) << nOffs);
   }
   else if (!bCurVal && bVal)
   {
      mClause.set(id);
      mInstanceId |= ( ((InstanceId) 1) << nOffs);
   }
}

void
Clause::AddVar(VarId id, bool bVal)
{
   int nOffs = mVarSet.GetOffs(id);
   if (nOffs < 0)
   {
      mVarSet.Add(id);
      nOffs = mVarSet.GetOffs(id);
   }

   bool bCurVal = mClause[id];
   
   if (bCurVal && !bVal)
   {
      mClause.reset(id);
      mInstanceId &= ~(((InstanceId)1) << nOffs);
   }
   else if (!bCurVal && bVal)
   {
      mClause.set(id);
      mInstanceId |= (((InstanceId)1) << nOffs);
   }
}



bool 
Clause::GetVar(VarId id) const
{
   return mClause[id];
}

InstanceId 
Clause::GetInstanceId(const VarSet &another) const
{
	Clause cl(another);
	for (VarId id = mVarSet.GetFirst();
		id != 0;
		id = mVarSet.GetNext(id))
	{
		if(another.HasVar(id))
			cl.SetVar(id, mClause[id]);
	}
	return cl.GetInstanceId();
}


bool
Clause::Incr()
{
   bool bRes = false;
   InstanceId idMax = ((InstanceId) 1 << mVarSet.GetSize());
   InstanceId tmpId = mInstanceId + 1;
   if (tmpId >= idMax)
   {
      tmpId = 0;
      bRes = true;
   }
   mInstanceId = tmpId;
   UpdateClause();
   return bRes;
}

bool 
Clause::Decr()
{
   bool bRes = false;
   InstanceId idMax = ((InstanceId) 1 << mVarSet.GetSize());
   InstanceId tmpId = mInstanceId -1;
   if (mInstanceId == 0)
   {
      tmpId = idMax-1;
      bRes = true;
   }
   mInstanceId = tmpId;
   UpdateClause();
   return bRes;

}

void 
Clause::UpdateClause()
{
   std::bitset<MAX_SET_SIZE> newclause;
   InstanceId i = 1;
   for(VarId id = mVarSet.GetFirst();
      id  != 0;
      id = mVarSet.GetNext(id), i <<= 1)   
   {
      
      if(mInstanceId & i)
         newclause.set(id);      
   }  
   mClause = newclause; 
}


Clause::Clause(const VarSet &vs, const std::bitset<MAX_SET_SIZE> &clause) :
      mInstanceId(0), mVarSet(vs) 
{
   int i=0;
   for(VarId id = mVarSet.GetFirst();
      id != 0;
      id = mVarSet.GetNext(id), i++)   
   {

      if(clause[id])
      {
         mClause.set(id);
         mInstanceId += ( (InstanceId)1 << i);
      }      
   }
}


Clause::Clause(const VarSet &vs, InstanceId iid) :      
       mInstanceId(iid), mVarSet(vs)
{
   int i=0;
   for(VarId id = mVarSet.GetFirst();
      id != 0;
      id = mVarSet.GetNext(id), i++)   
   {
      
      if(iid & ((InstanceId) 1 << i))
      {
         mClause.set(id);
      }      
   }      
}

std::string
Clause::GetType() const
{
   return "Clause";
}

std::string
Clause::GetJson(VarDb &db) const
{
   std::string s;
   s = "{ ";
   for (VarId id=mVarSet.GetFirst(); id != 0; id = mVarSet.GetNext(id))     
   {
      char sz[100];
      s += AddJsonAttr(sz, sizeof(sz), db[id].c_str(), "%d", mClause[id]?1:0);

   }
   s.erase(s.length() - 1);
   s += "}";
   return s;
}

Clause 
Clause::Append(VarSet target, const Clause &cl1, const Clause &cl2)
{
   Clause res(target);
   const VarSet &vs1 = cl1.GetVarSet();
   const VarSet &vs2 = cl2.GetVarSet();

   for(VarId id1 = vs1.GetFirst(); id1 != 0; id1 = vs1.GetNext(id1))
   {
         if(target.HasVar(id1))
            res.SetVar(id1, cl1.GetVar(id1));

   }

   for(VarId id2 = vs2.GetFirst(); id2 != 0; id2 = vs2.GetNext(id2))
   {
         if(target.HasVar(id2))
            res.SetVar(id2, cl2.GetVar(id2));
   }
   return res;
}

