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

Var::Var(std::string sName, VarId id, VarType vtype) :
mName(sName), mId(id), mVarType(vtype)
{}

    
      
std::string 
Var::GetJson(VarDb &db) const
{
   char sz[200];

      std::string s;
      s += "{";
      s += AddJsonAttr(sz, sizeof(sz), "id", "%d", mId);
      s += AddJsonAttr(sz, sizeof(sz), "name", "%s", mName.c_str() );
      s.erase(s.length()-1);
      s += "}";
      return s;
}
      
      
std::string 
Var::GetType() const
{
      return "Var";
}
 
