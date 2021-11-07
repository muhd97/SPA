#include "PKBNextHandler.h"

StatementType PKBPQLNextHandler::getStatementType(PKBDesignEntity de)
{
	switch (de)
	{
	case PKBDesignEntity::Read:
		return StatementType::READ;
	case PKBDesignEntity::Print:
		return StatementType::PRINT;
	case PKBDesignEntity::Assign:
		return StatementType::ASSIGN;
	case PKBDesignEntity::Call:
		return StatementType::CALL;
	case PKBDesignEntity::While:
		return StatementType::WHILE;
	case PKBDesignEntity::If:
		return StatementType::IF;
	case PKBDesignEntity::AllStatements:
		return StatementType::STATEMENT;	// Use this as a hack to represent AllStatements
	default:
		throw "Unknown StatementType - Design Ent";
	}
}

// Next
// Use for Next(_, _)
bool PKBPQLNextHandler::getNextUnderscoreUnderscore()
{
	return mpPKB->nextIntIntTable.begin() != mpPKB->nextIntIntTable.end();
}

// Case 2: Next(_, syn)
unordered_set<int> PKBPQLNextHandler::getNextUnderscoreSyn(PKBDesignEntity to)
{
	auto typePair = make_pair(PKBDesignEntity::AllStatements, to);
	unordered_set<int> result;
	for (auto p : mpPKB->nextSynSynTable[typePair])
	{
		result.insert(p.second);
	}

	return result;
}

// Case 3: Next(_, int)
bool PKBPQLNextHandler::getNextUnderscoreInt(int toIndex)
{
	return mpPKB->nextSynIntTable.find(toIndex) != mpPKB->nextSynIntTable.end();
}

// Case 4: Next(syn, syn)
set<pair<int, int>> PKBPQLNextHandler::getNextSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	auto typePair = make_pair(from, to);
	return mpPKB->nextSynSynTable[typePair];
}

// Case 5: Next(syn, _)
unordered_set<int> PKBPQLNextHandler::getNextSynUnderscore(PKBDesignEntity from)
{
	auto typePair = make_pair(from, PKBDesignEntity::AllStatements);
	unordered_set<int> result;
	for (auto p : mpPKB->nextSynSynTable[typePair])
	{
		result.insert(p.first);
	}

	return result;
}

// Case 6: Next(syn, int)
unordered_set<int> PKBPQLNextHandler::getNextSynInt(PKBDesignEntity from, int toIndex)
{
	return mpPKB->nextSynIntTable[toIndex][from];
}

// Case 7: Next(int, int)
bool PKBPQLNextHandler::getNextIntInt(int fromIndex, int toIndex)
{
	auto typePair = make_pair(fromIndex, toIndex);
	return mpPKB->nextIntIntTable.find(typePair) != mpPKB->nextIntIntTable.end();
}

// Case 8: Next(int, _)
bool PKBPQLNextHandler::getNextIntUnderscore(int fromIndex)
{
	return mpPKB->nextIntSynTable.find(fromIndex) != mpPKB->nextIntSynTable.end();
}

// Case 9: Next(int, syn)
unordered_set<int> PKBPQLNextHandler::getNextIntSyn(int fromIndex, PKBDesignEntity to)
{
	return mpPKB->nextIntSynTable[fromIndex][to];
}

// NextT(p, q)
void PKBPQLNextHandler::getNextTStatementList(vector<shared_ptr <Statement>> list, StatementType from, StatementType to, int fromIndex,
	int toIndex, set<pair<int, int>>* result, set<int>* seenP, bool canExitEarly)
{
	for (auto stmt : list)
	{
		if (canExitEarly && (result->begin() != result->end()))
		{
			return;
		}

		// Statement is used to represent AllStatements
		if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
		{
			for (auto p : *seenP)
			{
				result->insert(make_pair(p, stmt->getIndex()));
			}
		}

		// Statement is used to represent AllStatements
		if (stmt->getStatementType() == from || from == StatementType::STATEMENT || stmt->getIndex() == fromIndex)
		{
			seenP->insert(stmt->getIndex());
		}

		if (stmt->getStatementType() == StatementType::IF)
		{
			shared_ptr<IfStatement> ifS = static_pointer_cast<IfStatement> (stmt);
			set<pair<int, int>> cloneResult = set<pair<int, int>>(*result);
			set<int> cloneSeenP = set<int>(*seenP);

			getNextTStatementList(ifS->getConsequent()->getStatements(), from, to, fromIndex, toIndex, &cloneResult, &cloneSeenP, canExitEarly);
			getNextTStatementList(ifS->getAlternative()->getStatements(), from, to, fromIndex, toIndex, result, seenP,
				canExitEarly);

			result->insert(cloneResult.begin(), cloneResult.end());
			seenP->insert(cloneSeenP.begin(), cloneSeenP.end());
		}
		else if (stmt->getStatementType() == StatementType::WHILE)
		{
			shared_ptr<WhileStatement> whiles = static_pointer_cast<WhileStatement> (stmt);

			auto sizeP = seenP->size();
			getNextTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP, canExitEarly);

			if (sizeP < seenP->size())
			{
				// if there are new things in seenP we wanna do another pass
				getNextTStatementList(whiles->getStatementList(), from, to, fromIndex, toIndex, result, seenP,
					canExitEarly);
			}

			// While to while loop!
			if (stmt->getStatementType() == to || to == StatementType::STATEMENT || stmt->getIndex() == toIndex)
			{
				for (auto p : *seenP)
				{
					result->insert(make_pair(p, stmt->getIndex()));
				}
			}
		}
	}
}

set<pair<int, int>> PKBPQLNextHandler::getNextT(shared_ptr<Program> program, StatementType from, StatementType to, int fromIndex,
	int toIndex, bool canExitEarly)
{
	set<pair<int, int>> result = {};
	const auto& procs = program->getProcedures();
	if (procs.empty()) return move(result);
	vector<set<pair<int, int>>> procSets(procs.size(), set<pair<int, int>>());

	auto* baseAddr = &procs[0];

	for_each(execution::par_unseq, procs.begin(), procs.end(),
		[&from, &to, &fromIndex, &toIndex, &procs, &procSets, baseAddr](auto&& item)
		{
			int idx = &item - baseAddr;

			const auto& procedure = procs[idx];

			set<int> seenP = {};
			getNextTStatementList(procedure->getStatementList()->getStatements(), from, to, fromIndex, toIndex, &procSets[idx], &seenP, false);
		}

	);

	for (const auto& set : procSets)
	{
		result.insert(set.begin(), set.end());
	}

	return move(result);
}

// Use for NextT(_, _)
bool PKBPQLNextHandler::getNextTUnderscoreUnderscore()
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::STATEMENT, 0, 0, true);
	return result.begin() != result.end();
}

// Case 2: NextT(_, syn)
unordered_set<int> PKBPQLNextHandler::getNextTUnderscoreSyn(PKBDesignEntity to)
{
	set<pair<int, int>> result = getNextT(mpPKB->program, StatementType::STATEMENT, getStatementType(to), 0, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return move(toResult);
}

// Case 3: NextT(_, int)
bool PKBPQLNextHandler::getNextTUnderscoreInt(int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::STATEMENT, StatementType::NONE, 0, toIndex, true);
	return result.begin() != result.end();
}

// Case 4: NextT(syn, syn)
set<pair<int, int>> PKBPQLNextHandler::getNextTSynSyn(PKBDesignEntity from, PKBDesignEntity to)
{
	return getNextT(mpPKB->program, getStatementType(from), getStatementType(to), 0, 0, false);
}

// Case 5: NextT(syn, _)
unordered_set<int> PKBPQLNextHandler::getNextTSynUnderscore(PKBDesignEntity from)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::STATEMENT, 0, 0, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 6: NextT(syn, int)
unordered_set<int> PKBPQLNextHandler::getNextTSynInt(PKBDesignEntity from, int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, getStatementType(from), StatementType::NONE, 0, toIndex, false);
	unordered_set<int> fromResult = {};
	for (auto p : result)
	{
		fromResult.insert(p.first);
	}

	return move(fromResult);
}

// Case 7: NextT(int, int)
bool PKBPQLNextHandler::getNextTIntInt(int fromIndex, int toIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::NONE, fromIndex, toIndex, true);
	return result.begin() != result.end();
}

// Case 8: NextT(int, _)
bool PKBPQLNextHandler::getNextTIntUnderscore(int fromIndex)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, StatementType::STATEMENT, fromIndex, 0, true);
	return result.begin() != result.end();
}

// Case 9: NextT(int, syn)
unordered_set<int> PKBPQLNextHandler::getNextTIntSyn(int fromIndex, PKBDesignEntity to)
{
	set<pair<int, int>> result =
		getNextT(mpPKB->program, StatementType::NONE, getStatementType(to), fromIndex, 0, false);
	unordered_set<int> toResult = {};
	for (auto p : result)
	{
		toResult.insert(p.second);
	}

	return toResult;
}