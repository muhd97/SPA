#include "PQLParser.h"

#include <iostream>
#include <regex>

using namespace std;

string DesignEntity::PRINT = "print";
string DesignEntity::STMT = "stmt";
string DesignEntity::READ = "read";
string DesignEntity::WHILE = "while";
string DesignEntity::IF = "if";
string DesignEntity::ASSIGN = "assign";
string DesignEntity::VARIABLE = "variable";
string DesignEntity::CONSTANT = "constant";
string DesignEntity::PROCEDURE = "procedure";
string DesignEntity::CALL = "call";

PQLToken PQLParser::peek()
{
    return tokens[index];
}

PQLToken PQLParser::peekNext()
{
    return tokens[index + 1];
}

void PQLParser::advance()
{
    if (index < size)
    {
        index++;
    }
    else
    {
        cout << "PQLParser failed to advance EOF\n";
    }
}

bool PQLParser::tokensAreEmpty()
{
    return index == size;
}

inline PQLToken PQLParser::eat(PQLTokenType exepctedType)
{
    PQLToken tok = peek();
    if (tok.type == exepctedType)
    {
        advance();
    }
    else
    {
        PQLToken temp(exepctedType);
        cout << "Expected: " << getPQLTokenLabel(temp) << " but got: " << getPQLTokenLabel(tok) << " instead\n";
        throw std::invalid_argument("Error parsing PQL Query!!");
    }
    return tok;
}

inline PQLToken PQLParser::eatKeyword(string keyword)
{
    PQLToken tok = peek();
    if (tok.type == PQLTokenType::NAME && tok.stringValue == keyword)
    {
        advance();
    }
    else
    {
        cout << "Expected: " << keyword << " but got: " << getPQLTokenLabel(tok) << " instead\n";
        throw std::invalid_argument("Error parsing PQL Query!!");
    }
    return tok;
}

inline bool isKeyword(PQLToken token, string keyword)
{
    return token.type == PQLTokenType::NAME && token.stringValue == keyword;
}

shared_ptr<Declaration> PQLParser::parseDeclaration()
{
    vector<shared_ptr<Synonym>> synonyms;
    shared_ptr<DesignEntity> d = parseDesignEntity();

    synonyms.push_back(parseSynonym());

    while (peek().type == PQLTokenType::COMMA)
    {
        eat(PQLTokenType::COMMA);
        synonyms.push_back(parseSynonym());
    }

    eat(PQLTokenType::SEMICOLON);

    return make_shared<Declaration>(d, move(synonyms));
}

shared_ptr<DesignEntity> PQLParser::parseDesignEntity()
{
    PQLToken curr = peek();
    if (peek().type == PQLTokenType::NAME)
    {
        if (curr.stringValue == PQL_STMT)
        {
            eatKeyword(PQL_STMT);
        }
        else if (curr.stringValue == PQL_READ)
        {
            eatKeyword(PQL_READ);
        }
        else if (curr.stringValue == PQL_PRINT)
        {
            eatKeyword(PQL_PRINT);
        }
        else if (curr.stringValue == PQL_CALL)
        {
            eatKeyword(PQL_CALL);
        }
        else if (curr.stringValue == PQL_WHILE)
        {
            eatKeyword(PQL_WHILE);
        }
        else if (curr.stringValue == PQL_IF)
        {
            eatKeyword(PQL_IF);
        }
        else if (curr.stringValue == PQL_ASSIGN)
        {
            eatKeyword(PQL_ASSIGN);
        }
        else if (curr.stringValue == PQL_VARIABLE)
        {
            eatKeyword(PQL_VARIABLE);
        }
        else if (curr.stringValue == PQL_CONSTANT)
        {
            eatKeyword(PQL_CONSTANT);
        }
        else if (curr.stringValue == PQL_PROCEDURE)
        {
            eatKeyword(PQL_PROCEDURE);
        }
        else if (curr.stringValue == PQL_PROG_LINE)
        {
            eatKeyword(PQL_PROG_LINE);
        }
        else
        {
            cout << "ERROR: Unrecognized Design Entity!\n" << curr.stringValue;
        }

        return make_shared<DesignEntity>(curr.stringValue);
    }
    else
    {
        cout << "Expected stmt, read, print... but got: " << getPQLTokenLabel(curr) << " instead\n";
        throw std::invalid_argument("Error parsing PQL Query!!");
    }
}

inline shared_ptr<Synonym> PQLParser::parseSynonym()
{
    PQLToken t = peek();
    eat(PQLTokenType::NAME);
    return make_shared<Synonym>(t.stringValue);
}

inline int PQLParser::parseInteger()
{
    PQLToken intToReturn = peek();
    eat(PQLTokenType::INTEGER);
    return intToReturn.intValue;
}

shared_ptr<StmtRef> PQLParser::parseStmtRef()
{
    switch (peek().type)
    {
    case PQLTokenType::INTEGER:
        return make_shared<StmtRef>(StmtRefType::INTEGER, parseInteger());
    case PQLTokenType::UNDERSCORE:
        eat(PQLTokenType::UNDERSCORE);
        return make_shared<StmtRef>(StmtRefType::UNDERSCORE);
    default:
        return make_shared<StmtRef>(StmtRefType::SYNONYM, parseSynonym()->getValue());
    }
}

shared_ptr<EntRef> PQLParser::parseEntRef()
{
    switch (peek().type)
    {
    case PQLTokenType::UNDERSCORE:
        eat(PQLTokenType::UNDERSCORE);
        return make_shared<EntRef>(EntRefType::UNDERSCORE);

    case PQLTokenType::NAME:
        return make_shared<EntRef>(EntRefType::SYNONYM, parseSynonym()->getValue());

    case PQLTokenType::STRING:
        auto str = eat(PQLTokenType::STRING);
        // remove leading and trailing whitespaces
        string cleaned = regex_replace(str.stringValue, regex("^ +| +$"), "");
        auto toReturn = make_shared<EntRef>(EntRefType::IDENT, cleaned);
        return toReturn;
    }
    cout << "Unrecognized entity ref\n";
    return make_shared<EntRef>(EntRefType::UNDERSCORE);
}

shared_ptr<RelRef> PQLParser::parseUses()
{
    eatKeyword(PQL_USES);
    eat(PQLTokenType::LEFT_PAREN);

    if (peek().type == PQLTokenType::STRING)
    { /* If first arg of Uses() is a
       string, it must be a UsesP */
        auto eRef = parseEntRef();
        if (eRef->getEntRefType() == EntRefType::UNDERSCORE)
        {
            // TODO: Handle Error. INVALID to have underscore first Uses (_, x)
        }

        eat(PQLTokenType::COMMA);
        auto rRef = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<UsesP>(eRef, rRef);
    }
    else
    {
        auto sRef = parseStmtRef();
        if (sRef->getStmtRefType() == StmtRefType::UNDERSCORE)
        {
            // TODO: Handle Error. INVALID to have underscore first Uses (_, x)
        }

        eat(PQLTokenType::COMMA);
        auto rRef = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<UsesS>(sRef, rRef);
    }
}

shared_ptr<RelRef> PQLParser::parseModifies()
{
    eatKeyword(PQL_MODIFIES);
    eat(PQLTokenType::LEFT_PAREN);
    if (peek().type != PQLTokenType::STRING)
    { /* If first arg of Modifies() is a string, it must
be a ModifiesP */
        auto sRef11 = parseStmtRef();
        if (sRef11->getStmtRefType() == StmtRefType::UNDERSCORE)
        {
            // TODO: Handle Error. INVALID to have underscore first Modifies (_,
            // x)
        }

        eat(PQLTokenType::COMMA);
        auto eRef12 = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<ModifiesS>(sRef11, eRef12);
    }
    else
    { /* If first arg of Modifies() is a string, it must be a ModifiesP */

        auto eRef11 = parseEntRef();
        if (eRef11->getEntRefType() == EntRefType::UNDERSCORE)
        {
            // TODO: Handle Error. INVALID to have underscore first Modifies (_,
            // x)
        }

        eat(PQLTokenType::COMMA);
        auto eRef12 = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<ModifiesP>(eRef11, eRef12);
    }
}

vector<shared_ptr<SuchThatCl>> PQLParser::parseSuchThat()
{
    vector<shared_ptr<SuchThatCl>> clauses;
    eatKeyword(PQL_SUCH);
    eatKeyword(PQL_THAT);
    auto r = parseRelRef();
    clauses.push_back(make_shared<SuchThatCl>(r));
    
    if (peek().type == PQLTokenType::NAME && peek().stringValue == PQL_AND) {
        eatKeyword(PQL_AND);
        auto r = parseRelRef();
        clauses.push_back(make_shared<SuchThatCl>(r));
    }

    return clauses;
}

shared_ptr<RelRef> PQLParser::parseRelRef()
{
    auto curr = peek();
    if (curr.type == PQLTokenType::FOLLOWS_T) {
        // Follows*
        eat(PQLTokenType::FOLLOWS_T);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef3 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef4 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<FollowsT>(sRef3, sRef4);
    }
    else if (isKeyword(curr, PQL_FOLLOWS))
    {
        // Follows
        eatKeyword(PQL_FOLLOWS);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef1 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef2 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<Follows>(sRef1, sRef2);
    }
    else if (curr.type == PQLTokenType::PARENT_T) {
        eat(PQLTokenType::PARENT_T);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef7 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef8 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<ParentT>(sRef7, sRef8);
    }
    else if (isKeyword(curr, PQL_PARENT))
    {
        eatKeyword(PQL_PARENT);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef5 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef6 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<Parent>(sRef5, sRef6);
    }
    else if (isKeyword(curr, PQL_USES))
    {
        return parseUses();
    }
    else if (isKeyword(curr, PQL_MODIFIES))
    {
        return parseModifies();
    }
    else
    {
        cout << "Expected: Follow, FollowsT, Parent, ParentT, Uses and Modifies "
                "but got: "
             << getPQLTokenLabel(curr) << " instead\n";
        throw std::invalid_argument("Error parsing PQL Query!!");
    }
}

shared_ptr<ExpressionSpec> PQLParser::parseExpressionSpec()
{
    if (peek().type == PQLTokenType::UNDERSCORE)
    {
        eat(PQLTokenType::UNDERSCORE);
        if (peek().type == PQLTokenType::STRING)
        {
            auto token = eat(PQLTokenType::STRING);
            auto expressionTokens = simpleLex(token.stringValue);
            shared_ptr<Expression> expr = parseSimpleExpression(expressionTokens);
            eat(PQLTokenType::UNDERSCORE);
            return make_shared<ExpressionSpec>(false, true, expr);
        }
        else
        {
            return make_shared<ExpressionSpec>(true, false, nullptr);
        }
    }
    else if (peek().type == PQLTokenType::STRING)
    {
        auto token = eat(PQLTokenType::STRING);
        auto expressionTokens = simpleLex(token.stringValue);
        shared_ptr<Expression> expr = parseSimpleExpression(expressionTokens);
        return make_shared<ExpressionSpec>(false, false, expr);
    }
    else
    {
        cout << "Error: Invalid expression spec." << endl;
    }

    return make_shared<ExpressionSpec>(false, false, nullptr);
}

vector<shared_ptr<PatternCl>> PQLParser::parsePatternCl()
{
    vector<shared_ptr<PatternCl>> clauses;
    clauses.push_back(parsePatternClCond());
    eatKeyword(PQL_PATTERN);

    if (peek().type == PQLTokenType::NAME && peek().stringValue == PQL_AND) {
        eatKeyword(PQL_AND);
        clauses.push_back(parsePatternClCond());
    }
    return clauses;
}

shared_ptr<PatternCl> PQLParser::parsePatternClCond()
{
    auto syn = parseSynonym();
    eat(PQLTokenType::LEFT_PAREN);
    auto entRef = parseEntRef();
    eat(PQLTokenType::COMMA);
    string rawExpression;
    auto exprSpec = parseExpressionSpec();

    eat(PQLTokenType::RIGHT_PAREN);
    return make_shared<PatternCl>(syn, entRef, exprSpec);
}


inline bool tokenIsDesignEntity(PQLToken tk)
{
    return tk.type == PQLTokenType::NAME &&
        (tk.stringValue == PQL_STMT || tk.stringValue == PQL_READ || tk.stringValue == PQL_PRINT ||
            tk.stringValue == PQL_CALL || tk.stringValue == PQL_WHILE || tk.stringValue == PQL_IF ||
            tk.stringValue == PQL_ASSIGN || tk.stringValue == PQL_VARIABLE || tk.stringValue == PQL_CONSTANT ||
            tk.stringValue == PQL_PROCEDURE);
}

shared_ptr<AttrName> PQLParser::parseAttrName() {
    PQLToken name = eat(PQLTokenType::NAME);
    if (name.stringValue == PQL_PROC_NAME) {
        return make_shared<AttrName>(AttrNameType::PROC_NAME);
    }
    else if (name.stringValue == PQL_VAR_NAME) {
        return make_shared<AttrName>(AttrNameType::VAR_NAME);
    }
    else if (name.stringValue == PQL_VALUE) {
        return make_shared<AttrName>(AttrNameType::VALUE);
    }
    else if (name.stringValue == PQL_STMT_NUMBER) {
        return make_shared<AttrName>(AttrNameType::STMT_NUMBER);
    } else {
        throw "Unreconized attribute name: " + name.stringValue;
    }
}

shared_ptr<Element> PQLParser::parseElement() {
    auto syn = parseSynonym();
    
    if (!tokensAreEmpty() && peek().type == PQLTokenType::DOT) {
        // parse attr ref
        eat(PQLTokenType::DOT);
        shared_ptr<AttrName> attrName = parseAttrName();
        return make_shared<AttrRef>(syn, attrName);

    }
    else {

        return syn;
    }
}

shared_ptr<ResultCl> PQLParser::parseResultCl() {

    if (peek().type == PQLTokenType::NAME && peek().stringValue == PQL_BOOLEAN) {

        return make_shared<ResultCl>();
    }
    else if (peek().type == PQLTokenType::LT) {

        vector<shared_ptr<Element>> elements;
        elements.push_back(parseElement());

        eat(PQLTokenType::LT);
        while (peek().type == PQLTokenType::COMMA) {
            eat(PQLTokenType::COMMA);
            elements.push_back(parseElement());
        }
        eat(PQLTokenType::GT);
   
        return make_shared<ResultCl>(elements);
    }
    else {
        
        vector<shared_ptr<Element>> elements;
        elements.push_back(parseElement());
        
        return make_shared<ResultCl>(elements);
    }
}

shared_ptr<SelectCl> PQLParser::parseSelectCl()
{
    vector<shared_ptr<Declaration>> declarations;
    vector<shared_ptr<SuchThatCl>> suchThatClauses;
    vector<shared_ptr<PatternCl>> patternClauses;
    shared_ptr<ResultCl> result;

    while (tokenIsDesignEntity(peek()))
    {
        declarations.push_back(parseDeclaration());
    }


    eatKeyword(PQL_SELECT);
    result = parseResultCl();

    /* YIDA Note: For iteration 1, multiple such that clauses are NOT allowed */
    while (!tokensAreEmpty())
    {
        if (peek().type == PQLTokenType::NAME && peek().stringValue == PQL_SUCH)
        { 
            vector<shared_ptr<SuchThatCl>> clauses = parseSuchThat();
            suchThatClauses.insert(end(suchThatClauses), begin(clauses), end(clauses));
        }
        else if (peek().type == PQLTokenType::NAME && peek().stringValue == PQL_PATTERN)
        {
            vector<shared_ptr<PatternCl>> clauses = parsePatternCl();
            patternClauses.insert(end(patternClauses), begin(clauses), end(clauses));
        }
        else
        {
            cout << "ParseSelectCl Unknown token: " + getPQLTokenLabel(peek()) << endl;
            break;
        }
    }

    return make_shared<SelectCl>(move(result), move(declarations), move(suchThatClauses), move(patternClauses));
}
