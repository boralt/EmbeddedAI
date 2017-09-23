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



VarSet::VarSet(const VarDb &db) :
   mCachedInstances(0), mDb(db)
{
	mOffsetMapping.fill(-1);
}

VarSet::VarSet(const VarDb &db, const VarId v) :
   mCachedInstances(0), mDb(db)
{
	mOffsetMapping.fill(-1);
	Add(v);
}

VarSet::VarSet(const VarDb &db, std::initializer_list<VarId> initlist) :
   mCachedInstances(0), mDb(db)
{
	mOffsetMapping.fill(-1);
	for (auto iter = initlist.begin(); iter != initlist.end(); ++iter)
	{
		Add(*iter);
	}
}


bool VarSet::operator ==(const VarSet &another) const
{
	if (another.GetSize() != GetSize())
		return false;

	for (auto iter = mList.begin();
		iter != mList.end(); ++iter)
	{
		if (!another.HasVar(iter->mId))
		{
			return false;
		}
	}
	return true;
}


void 
VarSet::_Add(VarId id)
{ 
	if (!HasVar(id))
	{
      // instances until now
      InstanceId inst = GetInstances();
      Var v = mDb.GetVar(id);
		mList.push_back({id,(VarState) v.GetDomainSize(), inst });
		mOffsetMapping[id] = mList.size()-1;
      // invalidate the cache
      mCachedInstances = 0;
	}
}


void 
VarSet::Add(VarId id)
{ 
	DBC_CHECK_VID(id);
   _Add(id);
}

void
VarSet::Add(const VarSet &vs)
{
	for (VarId vid = vs.GetFirst(); vid != 0; vid = vs.GetNext(vid))
	{
       _Add(vid);
	}
}


void
VarSet::Remove(VarId id)
{
   for(auto it = mList.begin(); 
      it != mList.end(); ++it)
   {
	   if (it->mId == id)
	   {
		   mOffsetMapping[it->mId] = -1;
		   it = mList.erase(it);		   
		   // recalculate offsets of remaining variables
		   for (; it != mList.end(); ++it)
		   {
			   mOffsetMapping[it->mId]--;
		   }

         mCachedInstances = 0;
		 break;
      }
   }
}


void 
VarSet::MergeIn(const VarSet &another)
{
   for( VarId id = another.GetFirst(); 
            id!=0; 
            id =another.GetNext(id))
   {
	   _Add(id);
   }
}


bool 
VarSet::HasVar(VarId id) const
{
	if (id < 1 || id > MAX_SET_SIZE)
		return false;

    return mOffsetMapping[id] >= 0;
}

bool 
VarSet::HasVarType(VarType vartype) const
{
   for(auto it = mList.begin(); it != mList.end(); ++it)
   {
      if (mDb.GetVarType(it->mId) == vartype)
      {
         return true;
      }
   }
   return false;
}


VarSet 
VarSet::FilterVarSet(VarType vartype)
{
	VarSet res(mDb);
	for (auto it = mList.begin(); it != mList.end(); ++it)
	{
		if (mDb.GetVarType(it->mId) == vartype)
		{
			res.Add(it->mId);
		}
	}
	return res;
}


bool 
VarSet::HasVar(const VarSet &another) const
{
   for(auto it = mList.begin(); 
            it != mList.end(); 
            ++it)
   {
      if (another.HasVar(it->mId))
      {
         return true;
      }
   }
   return false;
}
   
unsigned int 
VarSet::GetSize() const
{
   return mList.size();
}



InstanceId VarSet::_GetInstances() const
{
   InstanceId res = 1;
   for(auto it = mList.begin(); 
            it != mList.end(); 
            ++it)
   {
      res *= it->mSize;
   }
   return res;
}

InstanceId VarSet::GetInstances() const
{
   if(!mCachedInstances)
   {
      mCachedInstances = _GetInstances();
   }
   return mCachedInstances;
}


int 
VarSet::GetOffs(VarId varid) const
{
	if (varid <= 0 && varid > MAX_SET_SIZE)
		return -1;
	return mOffsetMapping[varid];
}


void
VarSet::GetVarParams(VarId varid, InstanceId &varMultiplier, int &varSize)
{
   int offs = 0;

   if (varid <= 0 || varid > MAX_SET_SIZE || (offs = mOffsetMapping[varid]) < 0)
   {
      varMultiplier = 0;
      varSize = 0;
   }

   VarOperator op = GetOpByOffset(offs);
   varMultiplier = op.mMultiplier;
   varSize = op.mSize;
}



std::array<VarState, MAX_SET_SIZE>
VarSet::ConvertVarArray(InstanceId instanceId)
{
   std::array<VarState, MAX_SET_SIZE> res;
   res.fill(0);

   for(auto it = mList.begin();
       it != mList.end();
       ++it)
   {
      VarOperator &op = *it;
      // fetch value from InstanceId
      InstanceId tmp = instanceId / op.mMultiplier;
      VarState var = (VarState) (tmp % op.mSize);
      res[op.mId] = var;
   }
   return res;
};


VarState
VarSet::FetchVarState(VarId  id, InstanceId instanceId)
{
   int offs = GetOffs(id);
   if( offs < 0)
   {
      return 0;
   }

   VarOperator op = GetOpByOffset(offs);
   return (VarState) ((instanceId / op.mMultiplier)%op.mSize);
}

VarState
VarSet::FetchVarStateByOffs(int offs, InstanceId instanceId) {
   if (offs <0 || offs >= (int) mList.size())
      return 0;

   VarOperator op = GetOpByOffset(offs);
   return (VarState) ((instanceId / op.mMultiplier)%op.mSize);
}

VarSet::VarOperator
VarSet::GetOpByOffset(int offs)
{
   // B.A. improve this for scalability
   for (auto it = mList.begin(); it != mList.end();  ++it) {
      VarOperator &op = *it;
      if(!offs)
         return op;
      offs--;
   }

   return VarOperator(0,0,0);
}


VarId 
VarSet::GetFirst() const
{
   if(GetSize())
   {
      return mList.begin()->mId;
   }
   return 0;
}
   
VarId 
VarSet::GetNext(VarId id) const 
{
   for(auto it = mList.begin(); 
      it != mList.end(); 
      ++it)
   {
      if (it->mId == id)
      {
         ++it;
         
         if (it == mList.end())
            return 0;
         return it->mId;
      }
   }
   return 0;
}

VarSet 
VarSet::Conjuction(const VarSet &vs) const
{
   VarSet res(mDb);
   for(VarId id= vs.GetFirst(); 
      id != 0; 
      id = vs.GetNext(id))
   {
      if(HasVar(id))
         res.Add(id);      
   }
   return res;
}

VarSet 
VarSet::Disjuction(const VarSet &vs) const
{
   VarSet res = *this;
   for(VarId id= vs.GetFirst(); 
      id != 0; 
      id = vs.GetNext(id))
   {
      if(!HasVar(id))
            res.Add(id);      
   }
   return res;
} 

VarSet 
VarSet::Substract(const VarSet &vs) const
{
   VarSet res(mDb);
   for(VarId id= GetFirst(); 
      id != 0; 
      id = GetNext(id))
   {
      if(!vs.HasVar(id))
            res.Add(id);      
   }
   return res;
      
}


std::string 
VarSet::GetJson(const VarDb &) const
{
   std::string s;
   s = "[ ";
   for(auto it = mList.begin(); it != mList.end(); ++it)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%s", mDb[it->mId].c_str() );
      s += sz;
      s += ",";
   }   
   s.erase(s.length()-1);
   s += "]";
   return s;
}


std::string
VarSet::GetJsonAbbrev() const
{
   std::string s;
   s = "[ ";
   for(auto it = mList.begin(); it != mList.end(); ++it)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%d", it->mId);
      s += sz;
      s += ",";
   }
   s.erase(s.length()-1);
   s += "]";
   return s;
}


std::string 
VarSet::GetType() const
{
   return "VarSet";
}

const VarSet::VarOperator &
VarSet::_GetByOffset(int offs)
{
   for(auto it = mList.begin();
       it != mList.end();
       ++it)
   {
      if(!offs)
         return *it;
      offs--;
   }
   return *mList.begin();
}
