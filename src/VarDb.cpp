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


VarDb::VarDb()
{

}

void 
VarDb::AddVar(std::string sName, VarType vtype)
{
   // no repeating names
   if (HasVar(sName))
      return;

   VarId id = mAr.size() + 1;
   //mMap[id] = VarId(sName, id);
   Var v(sName, id, vtype);
   v.AddState("0");
   v.AddState("1");

   mMap[sName] = id;
   mAr.push_back(v);
   mArVarTypes.push_back(vtype);
}

void 
VarDb::AddVar(std::string sName, std::initializer_list<const char *> initlist, VarType vtype)
{
   // no repeating names
   if (HasVar(sName))
      return;

   VarId id = mAr.size() + 1;
   //mMap[id] = VarId(sName, id);
   Var v(sName, id, vtype);
   for (auto it = initlist.begin(); it != initlist.end(); ++it)
   {
      v.AddState(*it);

   }

	mMap[sName] = id;
	mAr.push_back(v);
	mArVarTypes.push_back(vtype);
}


bool
VarDb::HasVar(std::string sName)
{
   return mMap.count(sName) > 0;
}

VarSet 
VarDb::GetVarSet() const
{
   VarSet vs(*this);
   for (VarId i = 1; i <= mAr.size(); ++i)
   {
      vs.Add(i);
   }
   return vs;
}

VarType 
VarDb::GetVarType(VarId id) const
{
   return mArVarTypes[id-1];
}


VarId 
VarDb::operator[](const std::string &s) const
{
   if (mMap.count(s))
   {
      return mMap.at(s);
   }
   //return Var("", 0);
   return 0;
}

std::string 
VarDb::operator[](VarId id) const
{
   if (id == 0 || id > mAr.size())
      return "";
   return mAr[id-1].GetName();
}


std::string 
VarDb::GetJson()
{
   std::string s;
   s += "[ ";

   for(VarMap::iterator it = mMap.begin();
         it != mMap.end();
         ++it)
   {
      Var v(it->first, it->second);
      s += v.GetJson(*this);
      s += ",";
   }

   s.erase(s.length() -1);
   s += "]";
   return s;
}


VarState
VarDb::JsonValToState(VarId id, const Json::Value &v) const
{
   VarState res =0;
   Var var = GetVar(id);
   if ( var.IsNull())
      return res;

   if(v.isIntegral())
   {
      res = (VarState) v.asInt();
      if ( res < var.GetDomainSize())
      {
         return res;
      }
      else
      {
         return 0;
      }
   }
   else if(v.isBool())
   {
      if ( var.GetDomainSize() >= 2)
      {
         return v.asBool() ? 1: 0;
      }

      return 0;
   }
   else
   {
      const char *pStr = v.asCString();

      for(res=0; res < var.GetDomainSize(); res++)
      {
         if (!strcmp(pStr, var.GetState(res))
         {
            return res;
         }
      }
   }

   return 0;

}


std::string 
VarDb::GetType()
{
   return "VarDb";  
}

