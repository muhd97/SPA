#pragma once
using namespace std;
#include <cassert>

class PKB {
public:
	using SharedPtr = std::shared_ptr<PKB>;

	enum class Relation {
		//Parent
		Parent = 0,
		Child = 1,
		//ParentT
		ParentT = 2,
		ChildT = 3,
		//Follow
		Before = 4,
		After = 5,
		//FollowT
		BeforeT = 6,
		AfterT = 7,
		//Uses
		Uses = 8,
		//Modifies
		Modifies = 9
	};

	// for all statements, use Synonym::_, where position corresponds to statement index
	map<Synonym, vector<Statement::SharedPtr>> mStatements;

	// statement number, starting from index 1
	Statement::SharedPtr getStatement(int stmtNumber) {
		if (stmtNumber > mStatements[Synonym::_].size()) {
			throw std::invalid_argument("Requested statement number higher than max number of statements");
		}
		Statement::SharedPtr s = mStatements[stmtNumber];
		assert(s->getIndex() == stmtNumber);
		return s;
	}

	// note: position of statement in vector does NOT correspond to statement index except for Synonym::_
	vector<Statement::SharedPtr> getStmtsOfSynonym(Synonym s) {
		return mStatements[s];
	}

	bool getCached(Relation rel, Synonym a, Synonym b, vector<int> &res) {
		try {
			//todo @nicholas: check if this is desired behavior
			res = cache.at(rel).at(a).at(b);
			return true;
		}
		catch (std::out_of_range) {
			// result does not exist in the map, it is not cached
			return false;
		}
	}

	void insertintoCache(Relation rel, Synonym a, Synonym b, vector<int> &res) {
		cache[rel][a][b] = res;
	}





protected:
	// cache of our results, can be prebuilt
	// using vector<int> as this stores results at the moment, can be returned immediately
	map<Relation, 
		map<Synonym, 
		map<Synonym, vector<int>>>> cache;

};