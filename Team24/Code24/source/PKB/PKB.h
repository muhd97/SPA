#pragma once
using namespace std;

class PKB {
public:
	using SharedPtr = std::shared_ptr<PKB>;

	enum class Relation {
		Parent = 0,
		ParentT = 1,
		Follow = 2,
		FollowT = 3,
		Uses = 4,
		Modifies = 5
	};

	vector<Statement::SharedPtr> mStatements;
	

	// statement number, starting from index 1
	Statement::SharedPtr getStatement(int stmtNumber) {
		if (stmtNumber > mStatements.size()) {
			throw std::invalid_argument("Requested statement number higher than max number of statements");
		}
		return mStatements[stmtNumber];
	}

	bool getCached(Relation rel, Synonym a, Synonym b, vector<int> res) {
		try {
			//todo @nicholas: check if this is desired behavior
			res = cache.at(rel).at(a).at(b);
			return true;
		}
		catch (std::out_of_range) {
			return false;
		}
	}

	void insertintoCache(Relation rel, Synonym a, Synonym b, vector<int> res) {
		cache[rel][a][b] = res;
	}



protected:
	// cache of our results, can be prebuilt
	// using vector<int> as this stores results at the moment, can be returned immediately
	map<Relation, 
		map<Synonym, 
		map<Synonym, vector<int>>>> cache;
};