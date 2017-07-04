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

InteractionGraph::InteractionGraph(FactorSet *pFs)
{
    mVarSet = *pFs->GetVarSet().get();
    for(FactorSet::ListFactors::const_iterator iter = pFs->GetFactors().begin();
        iter != pFs->GetFactors().end();
        ++iter)
    {
        const VarSet &vs = iter->get()->GetVarSet();
        for(VarId id1 = vs.GetFirst(); id1 != 0; id1 = vs.GetNext(id1))
        {
            for(VarId id2 = vs.GetFirst(); id2 != 0; id2 = vs.GetNext(id2))
            {
                if(id1 != id2)
                {
                    if(!FindEdge(id1, id2))
                    {
                        std::pair<VarId, VarId> v1(id1, id2);
                        std::pair<VarId, VarId> v2(id2, id1);

                        // insert both definition of edges
                        mEdges.insert(v1);
                        mEdges.insert(v2);
                    }
                }
            }

        }
    }

}


VarSet
InteractionGraph::GetElimOrder()
{
    VarSet res;

    while(!mEdges.empty())
    {
        VarId idMinInteraction = 0;
        int nMinInteractions = 99999;

        for(VarId idCheck = mVarSet.GetFirst(); idCheck != 0; idCheck = mVarSet.GetNext(idCheck)) {
            int nCount = mEdges.count(idCheck);
            if (nCount > 0 && nCount < nMinInteractions) {
                nMinInteractions = nCount;
                idMinInteraction = idCheck;
            }
        }
        if(idMinInteraction)
        {
            mVarSet.Remove(idMinInteraction);
            res.Add(idMinInteraction);
            std::pair<EdgesMap::iterator, EdgesMap::iterator> range = mEdges.equal_range(idMinInteraction);
            std::list<VarId> listRemove;
            for(EdgesMap::iterator it1 = range.first;
                it1 != range.second; ++it1)
            {
                // list of merge neighbours
                listRemove.push_back(it1->second);
            }
            //remove old edges
            mEdges.erase(idMinInteraction);

            for(EdgesMap::iterator it2 = mEdges.begin();
                it2 != mEdges.end(); )
            {
                if(it2->second == idMinInteraction)
                {
                    // merge edges
                    it2  = mEdges.erase(it2);
                    // it2 = mEdges.begin();
                }
                else
                {
                    ++it2;
                }
            }

            // create new edges
            for(std::list<VarId>::iterator itEdges1 = listRemove.begin(); itEdges1 != listRemove.end(); ++itEdges1)
            {
                for(std::list<VarId>::iterator itEdges2 = itEdges1; itEdges2 != listRemove.end(); ++itEdges2)
                {
                    if(*itEdges1!=*itEdges2 && !FindEdge(*itEdges1, *itEdges2))
                    {
                        mEdges.insert(EdgesMap::value_type(*itEdges1, *itEdges2));
                    }
                    if(*itEdges1!=*itEdges2 && !FindEdge(*itEdges2, *itEdges1))
                    {
                        mEdges.insert(EdgesMap::value_type(*itEdges2, *itEdges1));
                    }



                }
            }

        }

    }
    if(mVarSet.GetSize() == 1)
    {
        res.Add(mVarSet.GetFirst());
    }

    return res;
}

bool
InteractionGraph::FindEdge(VarId id1, VarId id2)
{
    std::pair<EdgesMap::iterator, EdgesMap::iterator> range = mEdges.equal_range(id1);
    for(EdgesMap::iterator it1 = range.first;
        it1 != range.second; ++it1)
    {
        // list of merge neighbours
        if (it1->second == id2)
            return true;
    }

    return false;

}


