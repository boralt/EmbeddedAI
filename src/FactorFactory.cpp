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

std::shared_ptr<Factor> 
FactorFactory::Create(VarDb &db, Json::Value &vFactorDescrJson)
{
    // std::shared_ptr<Factor> res = new Factor();

    if(!vFactorDescrJson.isMember("vars"))
        return EmptyFactor();

    Json::Value &vars = vFactorDescrJson["vars"];
    
    VarSet headVars;
    if (vFactorDescrJson.isMember("head"))
    {
       headVars = VarSetFactory::Create(db, vFactorDescrJson["head"]);
    }

    VarSet vs = VarSetFactory::Create(db, vars);
    if (!vs.GetSize())
        return EmptyFactor();
    std::shared_ptr<Factor> res = std::make_shared<Factor>(vs, headVars);

    //no available values
    if(!vFactorDescrJson.isMember("vals"))
    {
        return res;
    }

    Json::Value &values = vFactorDescrJson["vals"];
    Factor::FactorLoader fl(res);
    for (Json::Value::iterator it = values.begin();
       it != values.end(); ++it)
    {
       ValueType val = (*it).asFloat();
       fl << val;       
    }
    return res;
} 

std::shared_ptr<Factor>
FactorFactory::EmptyFactor()
{
   VarSet vs;
   return std::make_shared<Factor>(vs);
}

