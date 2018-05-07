////////////////////////////////////////////////////////////////////////////////
// Copyright 2017-2018 Zuse Institute Berlin
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
//
// Contributors:
// 	Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#include <Util/CycleFinder.h>
#include <Util/Utility.h>

namespace Actul
{

struct Node
{
   bool visited;
   bool finished;

   Vector<size_type> succs;

   Node()
         : visited(false),
           finished(false)
   {
   }
};

struct Graph
{
   Vector<Node> nodes;
};

bool hasCycleRecursive(Graph & g, Node & n)
{
   bool res = false;
   if (!n.finished)
   {
      if (n.visited)
         res = true;
      else
      {
         n.visited = true;
         for (size_type i = 0; i < n.succs.size(); ++i)
            if ((res = hasCycleRecursive(g, g.nodes[n.succs[i]])))
               break;
      }
      n.finished = true;
   }
   return res;

}

bool hasCycleRecursive(Graph & g)
{
   bool res = false;
   for (size_type i = 0; i < g.nodes.size(); ++i)
   {
      if ((res = hasCycleRecursive(g, g.nodes[i])))
         break;
   }
   return res;
}

size_type getEquivalenceClassesOfAccesses(Array<SizeVec, 2> & res, const DataRaceVec & vec)
{
   size_type numAdded = 0, id1, id2;
   res[0].resize(vec.size(),invalid<size_type>());
   res[1].resize(vec.size(),invalid<size_type>());
   for (size_type i = 0; i < vec.size(); ++i)
   {
      id1 = numAdded;
      if (res[0][i] == invalid<size_type>())
      {
         res[0][i] = id1;
         ++numAdded;
      }
      else
         id1 = res[0][i];
      id2 = numAdded;
      if (res[1][i] == invalid<size_type>())
      {
         res[1][i] = id2;
         ++numAdded;
      }
      else
         id2 = res[1][i];

      for (size_type j = i + 1; j < vec.size(); ++j)
      {
         if (vec[i].getAccess(0).isSameInstance(vec[j].getAccess(0)))
            res[0][j] = res[0][i];
         if (vec[i].getAccess(0).isSameInstance(vec[j].getAccess(1)))
            res[1][j] = res[0][i];
         if (vec[i].getAccess(1).isSameInstance(vec[j].getAccess(0)))
            res[0][j] = res[1][i];
         if (vec[i].getAccess(1).isSameInstance(vec[j].getAccess(1)))
            res[1][j] = res[1][i];
      }
   }
   return numAdded;
}

struct DataRaceAccessRef
{
   size_type dNum;
   size_type aNum;

   DataRaceAccessRef()
   : dNum(invalid<size_type>()),
     aNum(invalid<size_type>())
   {

   }

   DataRaceAccessRef(const size_type & d, const size_type & a)
         : dNum(d),
           aNum(a)
   {
   }
};

void getAccessByTid(Vector<Vector<DataRaceAccessRef>> & res, const DataRaceVec & vec)
{
   actulAssert(res.empty(), "CycleFinder: passed out argument should be empty");
   tid_type tmp;
   for (size_type i = 0; i < vec.size(); ++i)
   {
      for (size_type j = 0; j < 2; ++j)
      {
         tmp = vec[i].tid(j);
         if (tmp >= res.size())
            res.resize(tmp + 1);
         res[tmp].push_back(DataRaceAccessRef(i, j));
      }
   }
}

Graph && generateGraph(const DataRaceVec & vec)
{
   Graph res;
   // find similar accesses and merge them to equivalance classes:
   Array<SizeVec, 2> classes;
   size_type numClasses = getEquivalenceClassesOfAccesses(classes, vec);
   Vector<Node> & nodes = res.nodes;
   nodes.resize(numClasses);
   // add artificial data race orders:
   for (size_type i = 0; i < vec.size(); ++i)
      if (vec[i].order())
         nodes[classes[1][i]].succs.push_back(classes[0][i]);

      else
         nodes[classes[0][i]].succs.push_back(classes[1][i]);

   // find accesses on same thread:
   Vector<Vector<DataRaceAccessRef>> tidClassified;
   getAccessByTid(tidClassified, vec);

   // add sequential consistency orders: TODO a lot of edges are added, this could be a bottleneck
   for (size_type i = 0; i < tidClassified.size(); ++i)
   {
      const Vector<DataRaceAccessRef> & tidAccs = tidClassified[i];
      for (size_type j = 0; j < tidAccs.size(); ++j)
         for (size_type k = j + 1; k < tidAccs.size(); ++k)
         {
            const MemoryAccess & a1 = vec[tidAccs[j].dNum].getAccess(tidAccs[j].aNum), &a2 = vec[tidAccs[k].dNum].getAccess(tidAccs[k].aNum);

            const size_type & id1 = classes[tidAccs[j].aNum][tidAccs[j].dNum], &id2 = classes[tidAccs[k].aNum][tidAccs[k].dNum];
            if (a1.happendBefore(a2))
               nodes[id1].succs.push_back(id2);
            else if (a2.happendBefore(a1))
               nodes[id2].succs.push_back(id1);
            else
               actulAssert(a1 == a2, "generateGraph failed. Two different accesses by one thread should be ordered");
         }
   }

   return std::move(res);
}

bool CycleFinder::hasCycles(const DataRaceVec & vec) const
{
   bool res = false;
   if (vec.size() > 1)
   {
      Graph g = generateGraph(vec);
      res = hasCycleRecursive(g);
   }
   return res;
}

CycleFinder::CycleFinder(const TestDatabase& tDb)
      : _tDb(tDb)
{
}

bool CycleFinder::hasCylces(const SizeVec & ids, const BoolVec & orders) const
{
   bool res = true;
   DataRaceVec tmp(ids.size());
   size_type replayId = _tDb.getReplayWith(ids);
   if (replayId != invalid<size_type>())
   {
      for (size_type i = 0; i < ids.size(); ++i)
      {
         tmp[i] = _tDb.getDataRace(ids[i], replayId);
         bool newOrder = Xor(tmp[i].order(), orders[i]);  // need to be set, because the order the data race is currently in and the order enforced need to be adjusted
         tmp[i].setOrder(newOrder);
      }
      res = hasCycles(tmp);
   }
   return res;

}

}

