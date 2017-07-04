/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/

#ifndef __FACTORIES_H
#define __FACTORIES_H

#include "factor.h"
#include <json/json-forwards.h>

namespace bayeslib
{
    class FactorFactory
    {
      public:
        static std::shared_ptr<Factor> Create(VarDb &db,  Json::Value &vFactorDescrJson);
        static std::shared_ptr<Factor> EmptyFactor();
    };

    class VarSetFactory
    {
      public:
        static VarSet Create(VarDb &db, Json::Value &vValuesDescrJson);
    };

    class FactorSetFactory
    {
        public:
        static FactorSet * Create(VarDb &db,  Json::Value &vFactorSetDescrJson);
    };

    class ClauseFactory
    {
        public:
        static Clause Create(VarDb &db, Json::Value &vClauseDescription);
    };

    class VarDbFactory
    {
      public:
         static VarDb *Create(Json::Value &vDbDescription);
    };

    class SessionEntry
    {
    public:
       static std::string RunCommand(std::string sOp);

    };

}
#endif
