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

FactorMergeHelper::FactorMergeHelper(const VarSet &v1,
   const VarSet &v2) : mV1(v1), mV2(v2), mVr(v1.GetDb())
{
    int offs1=0;
    int offs2=0;
    int offsr=0;
    

    for(VarId id = mV1.GetFirst(); id != 0; id = mV1.GetNext(id))
    {
        mVr.Add(id);
        mMap_Offs1_OffsR[offs1]=offsr;
        mMap_OffsR_Offs1[offsr]=offs1;
        
        offs2 = mV2.GetOffs(id);
        if(offs2 >= 0)
        {
            mMap_Offs1_Offs2[offs1] = offs2;
            mMap_Offs2_OffsR[offs2] = offsr;
            mMap_OffsR_Offs2[offsr] = offs2;
        }
        
        offsr++;
        offs1++;
    }

    offs2 = 0;
    for(VarId id = mV2.GetFirst(); id!=0; id=mV2.GetNext(id))
    {
        offs1 = mV1.GetOffs(id);
        if (offs1 >= 0)
        {
            // already taken care of
            ;
        }
        else
        {
            mVr.Add(id);
            mMap_Offs2_OffsR[offs2] = offsr;
            mMap_OffsR_Offs2[offsr] = offs2;
            offsr++;
        }
        offs2++;
        
    }

}

int 
FactorMergeHelper::MapOffsRTo1(int n)
{
    if(mMap_OffsR_Offs1.count(n))
        return mMap_OffsR_Offs1[n];
    return -1;    
}

int
FactorMergeHelper::MapOffsRTo2(int n)
{
    if(mMap_OffsR_Offs2.count(n))
        return mMap_OffsR_Offs2[n];
    return -1;    
}
