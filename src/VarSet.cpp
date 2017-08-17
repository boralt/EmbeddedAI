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



VarSet::VarSet() :
   mCachedInstances(0)
{
	mOffsetMapping.fill(-1);
}

VarSet::VarSet(const VarId v) :
   mCachedInstances(0)
{
	mOffsetMapping.fill(-1);
	Add(v);
}

VarSet::VarSet(std::initializer_list<VarId> initlist) :
   mCachedInstances(0)
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
		if (!another.HasVar(*iter))
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
		mList.push_back(id);
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
   for(std::list<VarId>::iterator it = mList.begin(); 
      it != mList.end(); ++it)
   {
	   if (*it == id)
	   {
		   mOffsetMapping[*it] = -1;
		   it = mList.erase(it);		   
		   // recalculate offsets of remaining variables
		   for (; it != mList.end(); ++it)
		   {
			   mOffsetMapping[*it]--;
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
VarSet::HasVarType(const VarDb &vdb, VarType vartype) const
{
   for(std::list<VarId>::const_iterator it = mList.begin(); it != mList.end(); ++it)
   {
      if ( vdb.GetVarType(*it) == vartype)
      {
         return true;
      }
   }
   return false;
}


VarSet 
VarSet::FilterVarSet(const VarDb &vdb, VarType vartype)
{
	VarSet res;
	for (auto it = mList.begin(); it != mList.end(); ++it)
	{
		if (vdb.GetVarType(*it) == vartype)
		{
			res.Add(*it);
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
      if (another.HasVar(*it)) 
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



InstanceId VarSet::_GetInstances(const VarDb &db) const
{
   InstanceId res = 1;
   for(auto it = mList.begin(); 
            it != mList.end(); 
            ++it)
   {
      res *= db[*it].GetDomainSize();
   }
   return res;
}

InstanceId VarSet::GetInstances(const VarDb &db) const
{
   if(!mCachedInstances)
   {
      mCachedInstances = _GetInstances(db);
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

VarId 
VarSet::GetFirst() const
{
   if(GetSize())
   {
      return *(mList.begin());
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
      if (*it == id)
      {
         ++it;
         
         if (it == mList.end())
            return 0;
         return *it;
      }
   }
   return 0;
}

VarSet 
VarSet::Conjuction(const VarSet &vs) const
{
   VarSet res;
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
   VarSet res;
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
VarSet::GetJson(VarDb &db) const
{
   std::string s;
   s = "[ ";
   for(auto it = mList.begin(); it != mList.end(); ++it)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%s", db[*it].c_str() );
      s += sz;
      s += ",";
   }   
   s.erase(s.length()-1);
   s += "]";
   return s;
}


std::string
VarSet::GetJson() const
{
   std::string s;
   s = "[ ";
   for(auto it = mList.begin(); it != mList.end(); ++it)
   {
      char sz[20];
      snprintf(sz, sizeof(sz), "%d", *it);
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
