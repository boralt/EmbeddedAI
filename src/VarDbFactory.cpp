/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/


#include "Factories.h"
#include <json/json.h>

using namespace bayeslib;

VarDb *
VarDbFactory::Create(Json::Value &v)
{
   VarDb *res = new VarDb;
 
   for (Json::Value::iterator it = v.begin();
      it != v.end(); ++it)
   {
      std::string sName = (*it).asString();
      res->AddVar(sName, VarType_Normal);
   }
   return res;
}
