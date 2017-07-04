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
      res->AddVar(sName);
   }
   return res;
}
