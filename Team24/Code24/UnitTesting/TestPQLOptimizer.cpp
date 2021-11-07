#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include "PQL/PQLResultTuple.h"
#include "PQL/PQLProcessorUtils.h"
#include "PQL/PQLProcessor.h"
#include "PQL/PQLLexer.h"
#include "PQL/PQLParser.h"
#include "PQL/PQLOptimizer.h"
#include <cassert>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTesting
{
    TEST_CLASS(TestPQLOptimizer)
    {
    public:

        TEST_METHOD(TestPriorityStarIsLast)
        {

            string q = "stmt p; stmt s; variable v; Select BOOLEAN such that Affects*(1, 4) and Uses(s, v) and Parent*(p, s)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto affectsTPtr = sel->getEvalClauses()[0];
            const auto usesPtr = sel->getEvalClauses()[1];
            const auto parentTPtr = sel->getEvalClauses()[2];

            auto groups = opt.getClauseGroups();
            Assert::IsTrue(groups.size() == 2);

            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->clauses[0].get() == affectsTPtr.get());

            const auto secondGroup = groups[1];
            Assert::IsTrue(secondGroup->clauses[0].get() == usesPtr.get());
            Assert::IsTrue(secondGroup->clauses[1].get() == parentTPtr.get());
        }

        TEST_METHOD(TestPriorityWithClAlwaysFirst)
        {

            string q = "stmt p; stmt s; variable v; Select BOOLEAN such that Affects*(1, 4) and Uses(s, v) and Parent*(p, s) with s.stmt# = 4 with 3 = 3";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto affectsPtr = sel->getEvalClauses()[0];
            const auto firstWith = sel->getEvalClauses()[3];
            const auto secondWith = sel->getEvalClauses()[4];

            auto groups = opt.getClauseGroups();

            Assert::IsTrue(groups.size() == 2);
            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->clauses[0].get() == affectsPtr.get());
            Assert::IsTrue(firstGroup->clauses[1].get() == secondWith.get());

            const auto secondGroup = groups[1];
            Assert::IsTrue(secondGroup->clauses[0].get() == firstWith.get());
        }

        TEST_METHOD(TestNoSynGroupFirst)
        {

            string q = "stmt p; stmt s; variable v; Select BOOLEAN such that Affects*(1, 4) and Uses(s, v) and Parent*(p, s) with s.stmt# = 4 with 3 = 3";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto affectsPtr = sel->getEvalClauses()[0];
            const auto firstWith = sel->getEvalClauses()[3];
            const auto secondWith = sel->getEvalClauses()[4];

            auto groups = opt.getClauseGroups();

            Assert::IsTrue(groups.size() == 2);
            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->synonymsInsideResultCl == false);
        }

        TEST_METHOD(TestGroupedCorrectly)
        {

            string q = "stmt p; stmt s; variable v; Select BOOLEAN such that Affects*(1, 4) and Uses(s, v) and Parent*(p, s) with s.stmt# = 4 with 3 = 3";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto affectsPtr = sel->getEvalClauses()[0];
            const auto firstWith = sel->getEvalClauses()[3];
            const auto secondWith = sel->getEvalClauses()[4];

            auto groups = opt.getClauseGroups();

            Assert::IsTrue(groups.size() == 2);
            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->clauses.size() == 2);
            const auto secondGroup = groups[1];
            Assert::IsTrue(secondGroup->clauses.size() == 3);
        }

        TEST_METHOD(TestSortWithin)
        {

            string q = "stmt s1,s2,s3; variable v1,v2,v3; Select BOOLEAN such that Uses(s1, v1) and Uses(s2, v1) and Affects*(s2, s3) and Affects*(s1, s2)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto firstAffect = sel->getEvalClauses()[2];
            const auto secondAffect = sel->getEvalClauses()[3];

            auto groups = opt.getClauseGroups();

            Assert::IsTrue(groups.size() == 1);
            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->clauses.size() == 4);
            Assert::IsTrue(firstGroup->clauses[2].get() == secondAffect.get());
        }

        TEST_METHOD(TestOneSynBeforeTwoSyn)
        {

            string q = "stmt s1,s2,s3; variable v1,v2,v3; Select BOOLEAN such that  Uses(s2, v1) and Uses(2, v1) and Affects*(s2, 5) and Affects*(s1, s2)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);

            // first clause is Affects*
            const auto secondUses = sel->getEvalClauses()[1];
            const auto firstAffect = sel->getEvalClauses()[2];
            const auto secondAffect = sel->getEvalClauses()[3];

            auto groups = opt.getClauseGroups();

            Assert::IsTrue(groups.size() == 1);
            const auto firstGroup = groups[0];
            Assert::IsTrue(firstGroup->clauses.size() == 4);
            Assert::IsTrue(firstGroup->clauses[0].get() == secondUses.get());
            Assert::IsTrue(firstGroup->clauses[2].get() == firstAffect.get());
            Assert::IsTrue(firstGroup->clauses[3].get() == secondAffect.get());
        }

        TEST_METHOD(TestFilterTuples)
        {
            string q = "Select x such that Follows*(1,2)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup21 = {
                {"x", "1"}, {"b", "2"}
            };
            unordered_map<string, string> tup22 = {
                {"x", "2"}, {"b", "3"}
            };
            unordered_map<string, string> tup23 = {
                {"x", "2"}, {"b", "4"}
            };

            unordered_map<string, string> tup24 = {
                {"x", "2"}, {"b", "23"}
            };
            unordered_map<string, string> tup25 = {
                {"x", "2"}, {"b", "24"}
            };

            unordered_map<string, string> tup26 = {
                {"x", "2"}, {"b", "7"}
            };
            unordered_map<string, string> tup27 = {
                {"x", "2"}, {"b", "8"}
            };

            unordered_map<string, string> tup28 = {
                {"x", "2"}, {"b", "9"}
            };
            unordered_map<string, string> tup29 = {
                {"x", "2"}, {"b", "10"}
            };

            for (auto& i : { tup21, tup22, tup23, tup24,  tup25, tup26, tup27, tup28, tup29 }) {
                list2.push_back(make_shared<ResultTuple>(i));
            }

            vector<shared_ptr<ResultTuple>> res;
            opt.filterTuples(list2, res);
            Assert::IsTrue(res.size() == 2);
            for (const auto& tup : res) {
                Assert::IsTrue(!tup->synonymKeyAlreadyExists("b"));
            }
        }

        TEST_METHOD(TestFilterTuplesEmpty)
        {
            string q = "Select k such that Follows*(1,2)";
            PQLParser p(pqlLex(q));
            auto sel = p.parseSelectCl();
            PQLOptimizer opt(sel);
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup21 = {
                {"x", "1"}, {"b", "2"}
            };
            unordered_map<string, string> tup22 = {
                {"x", "2"}, {"b", "3"}
            };
            unordered_map<string, string> tup23 = {
                {"x", "2"}, {"b", "4"}
            };

            unordered_map<string, string> tup24 = {
                {"x", "2"}, {"b", "23"}
            };
            unordered_map<string, string> tup25 = {
                {"x", "2"}, {"b", "24"}
            };

            unordered_map<string, string> tup26 = {
                {"x", "2"}, {"b", "7"}
            };
            unordered_map<string, string> tup27 = {
                {"x", "2"}, {"b", "8"}
            };

            unordered_map<string, string> tup28 = {
                {"x", "2"}, {"b", "9"}
            };
            unordered_map<string, string> tup29 = {
                {"x", "2"}, {"b", "10"}
            };

            for (auto& i : { tup21, tup22, tup23, tup24,  tup25, tup26, tup27, tup28, tup29 }) {
                list2.push_back(make_shared<ResultTuple>(i));
            }

            vector<shared_ptr<ResultTuple>> res;
            opt.filterTuples(list2, res);
            Assert::IsTrue(res.size() == 1);
        }

    };

}