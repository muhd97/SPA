#include "stdafx.h"
#include "CppUnitTest.h"
#include <iostream>
#include "PQL/PQLResultTuple.h"
#include "PQL/PQLProcessorUtils.h"
#include "PQL/PQLProcessor.h"
#include <cassert>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTesting
{
    TEST_CLASS(TestPQLProcessor)
    {
    public:

        TEST_METHOD(TestJoining)
        {
            vector<shared_ptr<ResultTuple>> list1;
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup11 = {
                {"x", "1"}, {"a", "2"}
            };
            unordered_map<string, string> tup12 = {
                {"x", "1"}, {"a", "3"}
            };
            unordered_map<string, string> tup13 = {
                {"x", "1"}, {"a", "4"}
            };
            list1.push_back(make_shared<ResultTuple>(tup11));
            list1.push_back(make_shared<ResultTuple>(tup12));
            list1.push_back(make_shared<ResultTuple>(tup13));
            unordered_map<string, string> tup21 = {
                {"x", "1"}, {"b", "2"}
            };
            unordered_map<string, string> tup22 = {
                {"x", "2"}, {"b", "3"}
            };
            unordered_map<string, string> tup23 = {
                {"x", "2"}, {"b", "4"}
            };
            list2.push_back(make_shared<ResultTuple>(tup21));
            list2.push_back(make_shared<ResultTuple>(tup22));
            list2.push_back(make_shared<ResultTuple>(tup23));

            vector<shared_ptr<ResultTuple>> result;

            auto joinKeys = getSetOfSynonymsToJoinOn(list1, list2);
            hashJoinResultTuples(list1, list2, joinKeys, result);
            assert(result.size() == 3);
        }

        TEST_METHOD(TestCartesian)
        {
            vector<shared_ptr<ResultTuple>> list1;
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup11 = {
                {"x", "1"}, {"a", "2"}
            };
            unordered_map<string, string> tup12 = {
                {"x", "1"}, {"a", "3"}
            };
            unordered_map<string, string> tup13 = {
                {"x", "1"}, {"a", "4"}
            };
            list1.push_back(make_shared<ResultTuple>(tup11));
            list1.push_back(make_shared<ResultTuple>(tup12));
            list1.push_back(make_shared<ResultTuple>(tup13));
            unordered_map<string, string> tup21 = {
                {"y", "1"}, {"b", "2"}
            };
            unordered_map<string, string> tup22 = {
                {"y", "2"}, {"b", "3"}
            };
            unordered_map<string, string> tup23 = {
                {"y", "2"}, {"b", "4"}
            };
            list2.push_back(make_shared<ResultTuple>(tup21));
            list2.push_back(make_shared<ResultTuple>(tup22));
            list2.push_back(make_shared<ResultTuple>(tup23));

            vector<shared_ptr<ResultTuple>> result;

            auto joinKeys = getSetOfSynonymsToJoinOn(list1, list2);
            assert(joinKeys.size() == 0);
            cartesianProductResultTuples(list1, list2, result);
            assert(result.size() == 9);
        }

        TEST_METHOD(TestParallelJoin)
        {
            vector<shared_ptr<ResultTuple>> list1;
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup11 = {
                {"x", "1"}, {"a", "2"}
            };
            unordered_map<string, string> tup12 = {
                {"x", "1"}, {"a", "3"}
            };
            unordered_map<string, string> tup13 = {
                {"x", "1"}, {"a", "4"}
            };
            list1.push_back(make_shared<ResultTuple>(tup11));
            list1.push_back(make_shared<ResultTuple>(tup12));
            list1.push_back(make_shared<ResultTuple>(tup13));
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

            vector<shared_ptr<ResultTuple>> result;
            auto joinKeys = getSetOfSynonymsToJoinOn(list1, list2);
            for (int i = 0; i < 2000; i++) {
                result.clear();
                hashJoinResultTuples(list1, list2, joinKeys, result);
                Assert::IsTrue(result.size() == 3);
            }
        }

        TEST_METHOD(TestParallelCartesian)
        {
            vector<shared_ptr<ResultTuple>> list1;
            vector<shared_ptr<ResultTuple>> list2;
            unordered_map<string, string> tup11 = {
                {"y", "1"}, {"a", "2"}
            };
            unordered_map<string, string> tup12 = {
                {"y", "1"}, {"a", "3"}
            };
            unordered_map<string, string> tup13 = {
                {"y", "1"}, {"a", "4"}
            };
            list1.push_back(make_shared<ResultTuple>(tup11));
            list1.push_back(make_shared<ResultTuple>(tup12));
            list1.push_back(make_shared<ResultTuple>(tup13));
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

            vector<shared_ptr<ResultTuple>> result;
            auto joinKeys = getSetOfSynonymsToJoinOn(list1, list2);
            assert(joinKeys.size() == 0);
            for (int i = 0; i < 2000; i++) {
                result.clear();
                cartesianProductResultTuples(list1, list2, result);
                Assert::IsTrue(result.size() == 27);
            }
        }

    };

}