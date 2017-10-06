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

Clause::Clause(const VarDb &db) :
   mInstanceId(0), mVarSet(db)
{
   mClause.fill(0);
}


Clause::Clause(const VarSet &vs) :
   mInstanceId(0), mVarSet(vs)
{
   mClause.fill(0);
}

Clause::Clause(const VarSet &vs, Json::Value &v) :
   mInstanceId(0), mVarSet(vs)
{
   mClause.fill(0);
   Json::ArrayIndex i = 0;
   for(VarId id = mVarSet.GetFirst();
      id != 0;
      (id=mVarSet.GetNext(id)), ++i)   
   {
      
      if(i < v.size())
      {
         VarState vs = mVarSet.GetDb().JsonValToState(id, v[i]);
         mClause[id] = vs;
         if(vs)
            mInstanceId += (InstanceId) vs  * mVarSet.GetInstanceComponent(id, vs);
      }
   }
}

Clause::Clause(const VarDb &db, std::initializer_list<ClauseInitializer> initlist) :
   mInstanceId(0), mVarSet(db)
{
   mClause.fill(0);
   for (auto iter = initlist.begin(); iter != initlist.end(); ++iter)
   {
      AddVar(iter->varid, iter->nState);
   }
}


void 
Clause::SetVar(VarId id, VarState val)
{
   assert(id>0);
   VarState curVal = mClause[id];
   int nOffs = mVarSet.GetOffs(id);

   assert(nOffs >= 0);
   if( nOffs < 0)
      return;

   if(curVal != val)
   {
      InstanceId oldInstanceComponent = mVarSet.GetInstanceComponent(id, curVal);
      InstanceId newInstanceComponent = mVarSet.GetInstanceComponent(id, val);
      mInstanceId -= oldInstanceComponent;
      mInstanceId += newInstanceComponent;
      mClause[id] = val;
   }

}


void
Clause::AddVar(VarId id, VarState val)
{
   int nOffs = mVarSet.GetOffs(id);
   if (nOffs < 0)
   {
      mVarSet.Add(id);
      nOffs = mVarSet.GetOffs(id);
   }

   SetVar(id, val);
}



VarState
Clause::GetVar(VarId id) const
{
   return mClause[id];
}

InstanceId 
Clause::GetInstanceId(const VarSet &another) const
{
   // correlate this varset with another
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
   InstanceId idMax = mVarSet.GetInstances();
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
   InstanceId idMax = mVarSet.GetInstances();
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
   mClause = mVarSet.ConvertVarArray(mInstanceId);
}


Clause::Clause(const VarSet &vs, const std::array<VarState ,MAX_SET_SIZE> &clause) :
      mInstanceId(0), mVarSet(vs) 
{
   int i=0;
   for(VarId id = mVarSet.GetFirst();
      id != 0;
      id = mVarSet.GetNext(id), i++)   
   {
      VarState v = clause[id];
      mClause[id] = v ;
      mInstanceId += mVarSet.GetInstanceComponent(id, v);
   }
}


Clause::Clause(const VarSet &vs, InstanceId iid) :      
       mInstanceId(iid), mVarSet(vs)
{
   UpdateClause();
}

std::string
Clause::GetType() const
{
   return "Clause";
}

std::string
Clause::GetJson(const VarDb &db)const
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


