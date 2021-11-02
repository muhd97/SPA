#pragma once
#pragma optimize( "gty", on )

#include <iostream>
#include <unordered_map>
#include <vector>

#include "PQLLexer.h"
#include "PQLAST.h"
#include "..\SimpleAST.h"
#include "..\SimpleLexer.h"
#include "..\SimpleParser.h"

using namespace std;

const string PQL_PROG_LINE = "prog_line";
const string PQL_PROCEDURE = "procedure";
const string PQL_READ = "read";
const string PQL_PRINT = "print";
const string PQL_CALL = "call";
const string PQL_WHILE = "while";
const string PQL_IF = "if";
const string PQL_ASSIGN = "assign";
const string PQL_VARIABLE = "variable";
const string PQL_CONSTANT = "constant";
const string PQL_STMT = "stmt";
const string PQL_SELECT = "Select";
const string PQL_FOLLOWS = "Follows";
const string PQL_FOLLOWS_T = "Follows*";
const string PQL_PARENT = "Parent";
const string PQL_PARENT_T = "Parent*";
const string PQL_USES = "Uses";
const string PQL_MODIFIES = "Modifies";
const string PQL_CALLS = "Calls";
const string PQL_NEXT = "Next";
const string PQL_NEXT_BIP = "NextBip";
const string PQL_AFFECTS = "Affects";
const string PQL_AFFECTS_BIP = "AffectsBip";
const string PQL_PATTERN = "pattern";
const string PQL_SUCH = "such";
const string PQL_THAT = "that";
const string PQL_WITH = "with";
const string PQL_BOOLEAN = "BOOLEAN";
const string PQL_PROC_NAME = "procName";
const string PQL_VAR_NAME = "varName";
const string PQL_VALUE = "value";
const string PQL_STMT_NUMBER = "stmt#";
const string PQL_AND = "and";

class PQLParser
{
private:
    vector<PQLToken> tokens;
    int index;
    int size;

public:
    PQLParser(vector<PQLToken> tok) : tokens(move(tok)), index(0)
    {
        size = tokens.size();
    }

    PQLToken peek();
    PQLToken peekNext();
    void advance();
    bool tokensAreEmpty();
    PQLToken eat(PQLTokenType exepctedType);
    PQLToken eatKeyword(string keyword);
    shared_ptr<Declaration> parseDeclaration();
    shared_ptr<DesignEntity> parseDesignEntity();
    vector<shared_ptr<SuchThatCl>> parseSuchThat();
    shared_ptr<RelRef> parseRelRef();
    shared_ptr<Synonym> parseSynonym();
    int parseInteger();
    shared_ptr<StmtRef> parseStmtRef();
    shared_ptr<EntRef> parseEntRef();
    shared_ptr<Ref> parseRef();
    shared_ptr<RelRef> parseUses();
    shared_ptr<RelRef> parseModifies();
    vector<shared_ptr<PatternCl>> parsePatternCl();
    shared_ptr<PatternCl> parsePatternClCond();
    vector<shared_ptr<WithCl>> parseWithCl();
    shared_ptr<WithCl> parseAttrCompare();
    shared_ptr<ExpressionSpec> parseExpressionSpec();
    shared_ptr<SelectCl> parseSelectCl();
    shared_ptr<ResultCl> parseResultCl();
    shared_ptr<Element> parseElement();
    shared_ptr<AttrName> parseAttrName();
};
