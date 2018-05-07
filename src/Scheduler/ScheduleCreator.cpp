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
#include <Scheduler/ScheduleCreator.h>
#include <map>

namespace Actul
{

class InternalScheduleCreator
{

   struct TokenId
   {
      size_type releaseNum;
      size_type mapNum;

      bool operator==(const TokenId & in) const
      {
         return releaseNum == in.releaseNum && mapNum == in.mapNum;
      }
   };

   struct AddedToken
   {
      size_type drId;
      ReplayToken token;

      AddedToken()
            : drId(invalid<size_type>())
      {
      }

      AddedToken(const size_type & drId, const ReplayToken & t)
            : drId(drId),
              token(t)
      {
      }

      bool operator<(const AddedToken & in) const
      {
         return token < in.token;
      }
   };

   struct StashedToken
   {
      bool isDepStart;
      TokenId id;
      ReplayToken token;

      StashedToken()
            : isDepStart(false)
      {
      }

      StashedToken(const TokenId & id, const ReplayToken & token)
            : isDepStart(false),
              id(id),
              token(token)
      {
      }

      StashedToken(const bool & isDepStart, const TokenId & id, const ReplayToken & token)
            : isDepStart(isDepStart),
              id(id),
              token(token)
      {
      }
   };

   struct Dependency
   {
      bool isActive;
      TokenId startId;
      TokenId endId;

      Dependency()
            : isActive(false)
      {
      }

      bool startsEarlier(const Dependency & in) const
      {
         return startId.releaseNum < in.startId.releaseNum || (startId.releaseNum == in.startId.releaseNum && startId.mapNum < in.startId.mapNum);
      }
   };

 public:

   InternalScheduleCreator(ReplayTokenShmVec & out, const ReplayTokenVec & baseReplay, const DataRaceShmVec & drs)
         : _out(out),
           _baseReplay(baseReplay),
           _drs(drs)
   {
      createAdditionalTokens();
      createDependencies();
   }

   void createSchedule()
   {
      StashedToken tmpToken;
      for (size_type i = 0; i < _baseReplay.size(); ++i)
      {
         tmpToken.id.releaseNum = i;
         tmpToken.id.mapNum = invalid<size_type>();
         tmpToken.token = _baseReplay[i];
         handleToken(tmpToken);
         auto it = _additionalTokens.find(i);
         if (it != _additionalTokens.end())
         {
            const auto & tokens = it->second;
            for (size_type j = 0; j < tokens.size(); ++j)
            {
               tmpToken.id.mapNum = j;
               tmpToken.token = tokens[j].token;
               handleToken(tmpToken);
            }
         }
      }

      /*printStr("Created replay:\n");
      for (size_type i = 0; i < _out.size(); ++i)
      {
         printStr("(t:" + to_string(_out[i].tid) + " eid:" + to_string(_out[i].et) + ", n: " + to_string(_out[i].numAccesses) + ")");
      }
      printStr("\n");*/
   }
 private:
   ReplayTokenShmVec & _out;
   const ReplayTokenVec & _baseReplay;
   const DataRaceShmVec & _drs;

   std::map<size_type, Vector<AddedToken>> _additionalTokens;
   Vector<Dependency> _dependencies;
   Vector<Dependency> _active;
   Vector<StashedToken> _stash;

   void putToStash(const TokenId & id, const ReplayToken & token)
   {
      _stash.push_back(StashedToken(id, token));
   }
   void addToken(const ReplayToken & token)
   {
      _out.insert(token);
   }

   const ReplayToken & getBaseToken(const TokenId & id) const
   {
      if (id.mapNum == invalid<size_type>())
         return _baseReplay[id.releaseNum];
      else
      {
         const auto & it = _additionalTokens.find(id.releaseNum);
         actulAssert(it != _additionalTokens.end() && it->second.size() > id.mapNum, "ScheduleCreator: Cannot find base token with id");
         return it->second[id.mapNum].token;
      }
   }

   size_type getTokenRestriction(const TokenId & id, const ReplayToken & token)
   {
      size_type res = invalid<size_type>();
      for (size_type i = 0; i < _dependencies.size(); ++i)
      {
         if (_dependencies[i].isActive)
            if (token.tid == getBaseToken(_dependencies[i].startId).tid)
               if (res == invalid<size_type>() || _dependencies[i].startsEarlier(_dependencies[res]))
                  res = i;
      }
      return res;
   }

   bool startDependencies(const TokenId & id, const ReplayToken & token)
   {
      bool started = false;
      for (size_type i = 0; i < _dependencies.size(); ++i)
      {
         if (!_dependencies[i].isActive && id == _dependencies[i].startId)
         {
            started = true;
            _dependencies[i].isActive = true;
            const ReplayToken & bt = getBaseToken(id);
            actulAssert(token == bt, "ScheduleCreator: Tokens are inconsistent");
         }
      }
      return started;
   }

   std::pair<bool, Vector<StashedToken>> resolveDependency(const TokenId & id, const ReplayToken & token, const bool & alreadyStashed)
   {
      std::pair<bool, Vector<StashedToken>> res;
      for (size_type i = 0; i < _dependencies.size();)
      {
         if (id == _dependencies[i].endId)
         {
            _dependencies.unordered_remove(i);
            res.first = true;
         } else
            ++i;
      }
      if (!alreadyStashed && res.first)
      {
         res.second.swap(_stash);
      }
      return res;
   }

   void handleToken(const StashedToken & stoken, const bool & alreadyStashed = false)
   {
      bool startedDependency = startDependencies(stoken.id, stoken.token);
      size_type rPos = getTokenRestriction(stoken.id, stoken.token);
      if (rPos != invalid<size_type>() && !alreadyStashed)
         _stash.push_back(StashedToken(startedDependency, stoken.id, stoken.token));
      else
      {
         auto resolvedTokens = resolveDependency(stoken.id, stoken.token, alreadyStashed);
         if (stoken.isDepStart || stoken.id.mapNum == invalid<size_type>() || !resolvedTokens.first)
            addToken(stoken.token);
         for (size_type i = 0; i < resolvedTokens.second.size(); ++i)
            handleToken(resolvedTokens.second[i], true);
      }
   }

   TokenId getAndMarkIdFromAccess(const MemoryAccess & m, const size_type & drId)
   {
      TokenId res;
      res.releaseNum = m.numTotalReleases;
      res.mapNum = invalid<size_type>();
      ReplayToken tmp(m);
      if (tmp != _baseReplay[m.numTotalReleases])
      {
         const auto & it = _additionalTokens.find(m.numTotalReleases);
         if (it != _additionalTokens.end())
         {
            auto & posTokens = it->second;
            for (size_type i = 0; i < posTokens.size(); ++i)
               if (posTokens[i].drId == drId && posTokens[i].token == tmp)
               {
                  res.mapNum = i;
                  break;
               }
         }
         actulAssert(res.mapNum != invalid<size_type>(), "ScheduleCreator: No token for access id found.");
      }
      return res;
   }

   void createDependencies()
   {
      _dependencies.resize(_drs.size());
      for (size_type i = 0; i < _drs.size(); ++i)
      {
         const DataRace & d = _drs[i];
         TokenId first = getAndMarkIdFromAccess(d.getAccess(0), i), second = getAndMarkIdFromAccess(d.getAccess(1), i);
         if (d.order())
         {
            _dependencies[i].startId = first;
            _dependencies[i].endId = second;
         } else
         {
            _dependencies[i].startId = second;
            _dependencies[i].endId = first;
         }
      }
   }

   void createAdditionalTokens()
   {
      for (size_type i = 0; i < _drs.size(); ++i)
      {
         const MemoryAccess & m0 = _drs[i].getAccess(0);
         const MemoryAccess & m1 = _drs[i].getAccess(1);
         ReplayToken tmp(m0);
         tmp.releaseViolated = _baseReplay[m0.numTotalReleases].releaseViolated;
         if (_baseReplay[m0.numTotalReleases] != tmp)
         {
            _additionalTokens[m0.numTotalReleases].push_back(AddedToken(i, ReplayToken(m0)));
         }

         tmp = m1;
         tmp.releaseViolated = _baseReplay[m1.numTotalReleases].releaseViolated;
         if (_baseReplay[m1.numTotalReleases] != ReplayToken(m1))
         {
            _additionalTokens[m1.numTotalReleases].push_back(AddedToken(i, ReplayToken(m1)));
         }
      }
      for (auto & p : _additionalTokens)
      {
         p.second.sort();
      }
   }
}
;

void ScheduleCreator::createSchedule(ReplayTokenShmVec & out, const ReplayTokenVec & baseReplay, const DataRaceShmVec & drs)
{
   InternalScheduleCreator creator(out, baseReplay, drs);
   //printStr("creating replay for " + to_string(drs.size()) + " data races\n");
   creator.createSchedule();
}

} /* namespace Actul */
