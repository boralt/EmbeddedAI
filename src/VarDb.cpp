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
   mMap[sName] = id;
   mAr.push_back(sName);
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
   VarSet vs;
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
VarDb::operator[](const std::string &s)
{
   if (mMap.count(s))
   {
      return mMap[s];
   }
   //return Var("", 0);
   return 0;
}

std::string 
VarDb::operator[](VarId id)
{
   if (id == 0 || id > mAr.size())
      return "";
   return mAr[id-1];
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

std::string 
VarDb::GetType()
{
   return "VarDb";  
}

