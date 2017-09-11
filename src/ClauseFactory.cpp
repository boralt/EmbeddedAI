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

/*
 *  "EXAMPLE" :
 *  {
 *     "varset" : ["a", "b", "c"],
 *     values : [ 0, 1, 1]
 *          or
 *     raised: ["b", "c"] or on:["b", "c"]
 *  }
 *
 *
 *
 */

Clause
ClauseFactory::Create(VarDb &db, Json::Value &v)
{
    Clause res(db);

    if (v.isMember("varset"))
    {
       res = Clause(VarSetFactory::Create(db, v["varset"]));
    }
    else
    {
       return res;
    }

    for (Json::Value::iterator it = v.begin();
         it != v.end(); ++it)
    {
        Json::Value &jv = *it;
        if(it.name() == "varset")
        {
           ;
        }
        else
        if (it.name() == "values" && jv.isArray())
        {

            VarId vid = res.GetVarSet().GetFirst();
            for (Json::Value::iterator itA = jv.begin(); itA != jv.end() && vid != 0; ++itA)
            {
                bool b = false;
                if(itA->isInt())
                    b = (itA->asInt() > 0);
                else
                    b = itA->asBool();

                res.SetVar(vid, b);
                vid = res.GetVarSet().GetNext(vid);
            }
        }
        else
        if (it.name() == "raised" || it.name() == "on")
        {
            for (Json::Value::iterator itA = jv.begin(); itA != jv.end(); ++itA)
            {
                if(itA->isString())
                {
                    std::string sName = itA->asString();
                    if(db.HasVar(sName))
                    {
                        VarId vid= db[sName];
                        res.SetVar(vid, true);
                    }
                }
            }

        }
    }
    return res;
}
