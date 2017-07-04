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
 *  [
 *    { ... factor },
 *    { .... factor },
 *  ]
 */

FactorSet *
FactorSetFactory::Create(VarDb &db, Json::Value &v)
{
    FactorSet *pRes = new FactorSet(db);
    for (Json::Value::iterator it = v.begin();
         it != v.end(); ++it)
    {
        if( it->isObject()  )
        {
            Json::Value &vf = *it;
            std::shared_ptr<Factor> f = FactorFactory::Create(db, vf);
            if(f)
            {
               pRes->AddFactor(f);

            }
        }
    }
    return pRes;


}
