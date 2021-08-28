#include "PQLParser.h"
#include <iostream>

using namespace std;

template <typename T>
using SPtr = std::shared_ptr<T>;

PQLToken PQLParser::peek() {
    return tokens[index];
}

PQLToken PQLParser::peekNext()
{
    return tokens[index + 1];
}

void PQLParser::advance()
{
    if (index < size) {
        index++;
    }
    else {
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
    //cout << "Eat1\n";
    if (tok.type == exepctedType) {
        advance();
    }
    else {
        PQLToken temp(exepctedType);
        cout << "Expected: " << getPQLTokenLabel(temp) << " but got: " << getPQLTokenLabel(tok) << " instead\n";
    }
    return tok;
}

void PQLParser::parsePQLQuery()
{
}

SPtr<Declaration> PQLParser::parseDeclaration()
{
    vector<string> synonyms;
    SPtr<DesignEntity> d = parseDesignEntity();

    synonyms.push_back(parseSynonym());

    while (peek().type == PQLTokenType::COMMA) {
        eat(PQLTokenType::COMMA);
        synonyms.push_back(parseSynonym());
    }

    eat(PQLTokenType::SEMICOLON);

    return make_shared<Declaration>(d, move(synonyms));
}


SPtr<DesignEntity> PQLParser::parseDesignEntity()
{
    PQLToken curr = peek();
    switch (curr.type) {
    case PQLTokenType::STMT:
        eat(PQLTokenType::STMT);
        break;
    case PQLTokenType::READ:
        eat(PQLTokenType::READ);
        break;
    case PQLTokenType::PRINT:
        eat(PQLTokenType::PRINT);
        break;
    case PQLTokenType::CALL:
        eat(PQLTokenType::CALL);
        break;
    case PQLTokenType::WHILE:
        eat(PQLTokenType::WHILE);
        break;
    case PQLTokenType::IF:
        eat(PQLTokenType::IF);
        break;
    case PQLTokenType::ASSIGN:
        eat(PQLTokenType::ASSIGN);
        break;
    case PQLTokenType::VARIABLE:
        eat(PQLTokenType::VARIABLE);
        break;
    case PQLTokenType::CONSTANT:
        eat(PQLTokenType::CONSTANT);
        break;
    case PQLTokenType::PROCEDURE:
        eat(PQLTokenType::PROCEDURE);
        break;
    default:
        cout << "Unrecognized Design Entity!\n";
    }

    return make_shared<DesignEntity>(getPQLTokenLabel(curr));
}


inline string PQLParser::parseSynonym()
{
    PQLToken t = peek();
    eat(PQLTokenType::NAME);
    return t.stringValue;
}

inline int PQLParser::parseInteger()
{
    PQLToken intToReturn = peek();
    eat(PQLTokenType::INTEGER);
    return intToReturn.intValue;
}

SPtr<StmtRef> PQLParser::parseStmtRef()
{
    switch (peek().type) {
    case PQLTokenType::INTEGER:
        return make_shared<StmtRef>(StmtRefType::INTEGER, parseInteger());
    case PQLTokenType::UNDERSCORE:
        eat(PQLTokenType::UNDERSCORE);
        return make_shared<StmtRef>(StmtRefType::UNDERSCORE);
    default:
        return make_shared<StmtRef>(StmtRefType::SYNONYM, parseSynonym());
    }
}

SPtr<EntRef> PQLParser::parseEntRef()
{

    switch (peek().type) {
    case PQLTokenType::UNDERSCORE:
        eat(PQLTokenType::UNDERSCORE);
        return make_shared<EntRef>(EntRefType::UNDERSCORE);

    case PQLTokenType::NAME:
        return make_shared<EntRef>(EntRefType::SYNONYM, parseSynonym());

    case PQLTokenType::QUOTE_MARK:
        eat(PQLTokenType::QUOTE_MARK);
        auto toReturn = make_shared<EntRef>(EntRefType::IDENT, parseSynonym());
        eat(PQLTokenType::QUOTE_MARK);
        return toReturn;
    }
    cout << "Unrecognized entity ref\n";
    return make_shared<EntRef>(EntRefType::UNDERSCORE);
}

SPtr<UsesS> PQLParser::parseUses()
{
    eat(PQLTokenType::USES);
    eat(PQLTokenType::LEFT_PAREN);
    auto sRef = parseStmtRef();
    if (sRef->getStmtRefType() == StmtRefType::UNDERSCORE) {
        // TODO: Handle Error. INVALID to have underscore first Uses (_, x)
    }

    eat(PQLTokenType::COMMA);
    auto rRef = parseEntRef();
    eat(PQLTokenType::RIGHT_PAREN);
    return make_shared<UsesS>(sRef, rRef);
}

SPtr<SuchThatCl> PQLParser::parseSuchThat() // todo
{
    eat(PQLTokenType::SUCH_THAT);
    auto r = parseRelRef();
    return make_shared<SuchThatCl>(r);

}

SPtr<RelRef> PQLParser::parseRelRef() // todo
{
    switch (peek().type) {
    case PQLTokenType::FOLLOWS:
    {
        eat(PQLTokenType::FOLLOWS);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef1 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef2 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<Follows>(sRef1, sRef2);
    }

    case PQLTokenType::FOLLOWS_T:
    {
        eat(PQLTokenType::FOLLOWS_T);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef3 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef4 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<FollowsT>(sRef3, sRef4);
    }
    case PQLTokenType::PARENT:
    {
        eat(PQLTokenType::PARENT);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef5 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef6 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<Parent>(sRef5, sRef6);
    }

    case PQLTokenType::PARENT_T:
    {
        eat(PQLTokenType::PARENT_T);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef7 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto sRef8 = parseStmtRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<ParentT>(sRef7, sRef8);
    }

    case PQLTokenType::USES: // ONLY UsesS, not UsesP
    {
        eat(PQLTokenType::USES);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef9 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto eRef10 = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<UsesS>(sRef9, eRef10);
    }

    default: // Only ModifiesS, not ModifiesP
    {
        eat(PQLTokenType::MODIFIES);
        eat(PQLTokenType::LEFT_PAREN);
        auto sRef11 = parseStmtRef();
        eat(PQLTokenType::COMMA);
        auto eRef12 = parseEntRef();
        eat(PQLTokenType::RIGHT_PAREN);
        return make_shared<ModifiesS>(sRef11, eRef12);
    }
    }
}

SPtr<ExpressionSpec> PQLParser::parseExpressionSpec()
{
    if (peek().type == PQLTokenType::UNDERSCORE) {
        eat(PQLTokenType::UNDERSCORE);

        if (peek().type == PQLTokenType::QUOTE_MARK) {
            eat(PQLTokenType::QUOTE_MARK);
            
            // TODO: Expose simple expression parse function
            // Expression* expr = parseExpression();
            // hack for now
            while (peek().type != PQLTokenType::QUOTE_MARK) {
                advance();
            }
            Expression* expr = nullptr;

            eat(PQLTokenType::QUOTE_MARK);
            eat(PQLTokenType::UNDERSCORE);
            return make_shared<ExpressionSpec>(false, true, expr);
        }
        else {
            return make_shared<ExpressionSpec>(true, false, nullptr);
        }
        
    }
    else if (peek().type == PQLTokenType::QUOTE_MARK) {
        eat(PQLTokenType::QUOTE_MARK);
        // TODO: Expose simple expression parse function
        // Expression* expr = parseExpression()
        // hack for now
        while (peek().type != PQLTokenType::QUOTE_MARK) {
            advance();
        }
        Expression* expr = nullptr;

        eat(PQLTokenType::QUOTE_MARK);
        eat(PQLTokenType::UNDERSCORE);
        return make_shared<ExpressionSpec>(false, false, expr);
    }
    else {
        // TODO: Handle Error. 
    }
    return make_shared<ExpressionSpec>(false, false, nullptr);
}

SPtr<PatternCl> PQLParser::parsePatternCl()
{
    eat(PQLTokenType::PATTERN);
    auto syn = parseSynonym();

    // TODO (@jiachen247) Check syn is of type assign

    eat(PQLTokenType::LEFT_PAREN);

    auto entRef = parseEntRef();
    eat(PQLTokenType::COMMA);
    string rawExpression;
    auto exprSpec = parseExpressionSpec();

    eat(PQLTokenType::RIGHT_PAREN);
    return make_shared<PatternCl>("x", nullptr, nullptr);

}

inline bool tokenIsDesignEntity(PQLTokenType tk) {
    return tk == PQLTokenType::STMT
        || tk == PQLTokenType::READ
        || tk == PQLTokenType::PRINT
        || tk == PQLTokenType::CALL
        || tk == PQLTokenType::WHILE
        || tk == PQLTokenType::IF
        || tk == PQLTokenType::ASSIGN
        || tk == PQLTokenType::VARIABLE
        || tk == PQLTokenType::CONSTANT
        || tk == PQLTokenType::PROCEDURE;
}

SPtr<SelectCl> PQLParser::parseSelectCl()
{
    vector<SPtr<Declaration>> declarations;
    vector<SPtr<SuchThatCl>> suchThatClauses;
    vector<SPtr<PatternCl>> patternClauses;
    string synonym;
    while (tokenIsDesignEntity(peek().type)) {
        declarations.push_back(parseDeclaration());
    }

    eat(PQLTokenType::SELECT);
    synonym = parseSynonym();

    while (!tokensAreEmpty()) {
        if (suchThatClauses.size() == 0 && peek().type == PQLTokenType::SUCH_THAT) {
            if (suchThatClauses.size() == 0) {
                cout << "Duplicate such that clauses are not allowed." << endl;
                break;
            }
            suchThatClauses.push_back(parseSuchThat());
        }
        else if (peek().type == PQLTokenType::PATTERN) {
            if (patternClauses.size() == 0) {
                cout << "Duplicate pattern clauses are not allowed." << endl;
                break;
            }
            patternClauses.push_back(parsePatternCl());
        }
        else {
            cout << "Unknown token: " + getPQLTokenLabel(peek()) << endl;
            break;
        }
    }

    return make_shared<SelectCl>(move(synonym), move(declarations), move(suchThatClauses));

}


