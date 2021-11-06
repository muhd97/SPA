#include "PQLAST.h"

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
string DesignEntity::PROG_LINE = "prog_line";

const string& Synonym::getValue() const
{
    return value;
}

const string& Synonym::getSynonymString() {
    return getValue();
}

string Synonym::format()
{
    return "$" + value;
}

ElementType Synonym::getElementType() {
    return ElementType::Synonym;
}

AttrNameType AttrName::getType() {
    return this->name;
}

string AttrName::format() {
    switch (name)
    {
    case AttrNameType::PROC_NAME:
        return "procName";
    case AttrNameType::VAR_NAME:
        return "varName";
    case AttrNameType::VALUE:
        return "value";
    case AttrNameType::STMT_NUMBER:
        return "stmt#";
    }
    return "";
}

AttrNameType AttrName::getAttrNameType() {
    return name;
}

string AttrRef::format() {
    return synonym->format() + "." + attrName->format();
}

ElementType AttrRef::getElementType() {
    return ElementType::AttrRef;
}

const string& AttrRef::getSynonymString() {
    return synonym->getValue();
}

const shared_ptr<Synonym>& AttrRef::getSynonym() {
    return synonym;
}

const shared_ptr<AttrName>& AttrRef::getAttrName() {
    return attrName;
}

const string& DesignEntity::getEntityTypeName() const
{
    return entityTypeName;
}

const vector<shared_ptr<Synonym>>& Declaration::getSynonyms() const
{
    return synonyms;
}

shared_ptr<DesignEntity> Declaration::getDesignEntity()
{
    return de;
}

string Declaration::format()
{
    auto builder = de->getEntityTypeName() + " [";
    for (auto s : synonyms)
    {
        builder += s->format() + ", ";
    }
    return builder + "]";
}

StmtRefType StmtRef::getStmtRefType()
{
    return stmtRefType;
}

string StmtRef::getStmtRefTypeName()
{
    switch (stmtRefType)
    {
    case StmtRefType::INTEGER:
        return "int(" + to_string(getIntVal()) + ")";
    case StmtRefType::SYNONYM:
        return "syn(" + getStringVal() + ")";
    case StmtRefType::UNDERSCORE:
        return "_";
    }
    return "";
}

const string& StmtRef::getStringVal() const
{
    return stringValue;
}

int StmtRef::getIntVal()
{
    return intValue;
}

const string& EntRef::getStringVal() const
{
    return stringValue;
}

EntRefType EntRef::getEntRefType()
{
    return entRefType;
}

string EntRef::getEntRefTypeName()
{
    switch (entRefType)
    {
    case EntRefType::IDENT:
        return "ident(" + getStringVal() + ")";
    case EntRefType::SYNONYM:
        return "syn(" + getStringVal() + ")";
    case EntRefType::UNDERSCORE:
        return "_";
    }
    return "";
}

string EntRef::format()
{
    return getEntRefTypeName();
}

const string& Ref::getStringVal() const
{
    return stringValue;
}

int Ref::getIntVal()
{
    return intValue;
}

RefType Ref::getRefType()
{
    return refType;
}

const shared_ptr<AttrRef>& Ref::getAttrRef()
{
    return attrRef;
}

string Ref::format()
{
    switch (refType)
    {
    case RefType::IDENT:
        return "ident(" + getStringVal() + ")";
    case RefType::SYNONYM:
        return "syn(" + getStringVal() + ")";
    case RefType::INTEGER:
        return "int(" + to_string(getIntVal()) + ")";
    case RefType::ATTR:
        return attrRef->format();
    }
    return "";
}

string UsesS::format()
{
    return "UsesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
}

bool UsesS::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (entRef->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType UsesS::getType()
{
    return RelRefType::USES_S;
}

vector<string> UsesS::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef->getStringVal());
    if (entRef->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef->getStringVal());

    return toReturn;
}

string UsesP::format()
{
    return "UsesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
}

bool UsesP::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType UsesP::getType()
{
    return RelRefType::USES_P;
}

vector<string> UsesP::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef1->getStringVal());
    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef2->getStringVal());

    return toReturn;
}

string ModifiesS::format()
{
    return "ModifiesS(" + stmtRef->getStmtRefTypeName() + ", " + entRef->getEntRefTypeName() + ")";
}

bool ModifiesS::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef->getStringVal() == s->getSynonymString();
    }

    if (entRef->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = flag || (entRef->getStringVal() == s->getSynonymString());
    }
    return flag;
}

RelRefType ModifiesS::getType()
{
    return RelRefType::MODIFIES_S;
}

vector<string> ModifiesS::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef->getStringVal());
    if (entRef->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef->getStringVal());

    return toReturn;
}

bool ModifiesP::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef1->getStringVal() == s->getSynonymString();
    }

    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = flag || (entRef2->getStringVal() == s->getSynonymString());
    }

    return flag;
}

RelRefType ModifiesP::getType()
{
    return RelRefType::MODIFIES_P;
}

string ModifiesP::format()
{
    return "ModifiesP(" + entRef1->getEntRefTypeName() + ", " + entRef2->getEntRefTypeName() + ")";
}

vector<string> ModifiesP::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef1->getStringVal());
    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef2->getStringVal());

    return toReturn;
}

string Parent::format()
{
    return "Parent(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool Parent::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType Parent::getType()
{
    return RelRefType::PARENT;
}

vector<string> Parent::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return toReturn;
}

string ParentT::format()
{
    return "Parent*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool ParentT::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType ParentT::getType()
{
    return RelRefType::PARENT_T;
}

vector<string> ParentT::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return toReturn;
}

string Follows::format()
{
    return "Follows(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool Follows::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType Follows::getType()
{
    return RelRefType::FOLLOWS;
}

vector<string> Follows::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return toReturn;
}

string FollowsT::format()
{
    return "Follows*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool FollowsT::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType FollowsT::getType()
{
    return RelRefType::FOLLOWS_T;
}

vector<string> FollowsT::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return toReturn;
}

string Calls::format()
{
    return "Calls(" + entRef1->format() + ", " + entRef2->format() + ")";
}

bool Calls::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType Calls::getType()
{
    return RelRefType::CALLS;
}

vector<string> Calls::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef1->getStringVal());
    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef2->getStringVal());

    return toReturn;
}

string CallsT::format()
{
    return "CallsT(" + entRef1->format() + ", " + entRef2->format() + ")";
}

bool CallsT::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
    {
        flag = entRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType CallsT::getType()
{
    return RelRefType::CALLS_T;
}

vector<string> CallsT::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (entRef1->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef1->getStringVal());
    if (entRef2->getEntRefType() == EntRefType::SYNONYM)
        toReturn.emplace_back(entRef2->getStringVal());

    return toReturn;
}

string NextT::format()
{
    return "Next*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

RelRefType NextT::getType()
{
    return RelRefType::NEXT_T;
}

string Next::format()
{
    return "Next(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool Next::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType Next::getType()
{
    return RelRefType::NEXT;
}

vector<string> Next::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return move(toReturn);
}

string NextBip::format()
{
    return "NextBip(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

RelRefType NextBip::getType()
{
    return RelRefType::NEXT_BIP;
}

string NextBipT::format()
{
    return "Next*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

RelRefType NextBipT::getType()
{
    return RelRefType::NEXT_BIP_T;
}

string AffectsT::format()
{
    return "Next*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool AffectsT::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType AffectsT::getType()
{
    return RelRefType::AFFECTS_T;
}

vector<string> AffectsT::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return move(toReturn);
}

string Affects::format()
{
    return "Next(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool Affects::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType Affects::getType()
{
    return RelRefType::AFFECTS;
}

vector<string> Affects::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return move(toReturn);
}

string AffectsBip::format()
{
    return "NextBip(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool AffectsBip::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType AffectsBip::getType()
{
    return RelRefType::AFFECTS_BIP;
}

vector<string> AffectsBip::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return move(toReturn);
}

string AffectsBipT::format()
{
    return "Next*(" + stmtRef1->getStmtRefTypeName() + ", " + stmtRef2->getStmtRefTypeName() + ")";
}

bool AffectsBipT::containsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef1->getStringVal() == s->getSynonymString();
        if (flag)
            return flag;
    }

    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
    {
        flag = stmtRef2->getStringVal() == s->getSynonymString();
    }

    return flag;
}

RelRefType AffectsBipT::getType()
{
    return RelRefType::AFFECTS_BIP_T;
}

vector<string> AffectsBipT::getAllSynonymsAsString()
{
    vector<string> toReturn;

    if (stmtRef1->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef1->getStringVal());
    if (stmtRef2->getStmtRefType() == StmtRefType::SYNONYM)
        toReturn.emplace_back(stmtRef2->getStringVal());

    return move(toReturn);
}

EvalClType SuchThatCl::getEvalClType() {
    return EvalClType::SuchThat;
}

string SuchThatCl::format()
{
    return "\nSUCHTHAT " + relRef->format();
}

bool SuchThatCl::containsSynonym(shared_ptr<Element> s)
{
    return relRef->containsSynonym(s);
}

const vector<string>& SuchThatCl::getAllSynonymsAsString()
{
    return synonymsUsed;
}

string ExpressionSpec::format()
{
    if (isAnything)
    {
        return "_";
    }
    else if (isPartialMatch)
    {
        return "_\"" + expression->format(0) + "\"_";
    }
    else
    {
        return "\"" + expression->format(0) + "\"";
    }
}


const vector<shared_ptr<Element>>& ResultCl::getElements() const {
    return elements;
}

string ResultCl::format()
{
    if (isBoolean) {
        return "BOOLEAN";
    }
    else {
        string str = "<";
        for (auto elem : elements) {
            str += elem->format() + ", ";
        }
        return str + ">";
    }
}

bool ResultCl::isBooleanReturnType() {
    return isBoolean;
}

bool ResultCl::isMultiTupleReturnType() {
    return elements.size() > 1;
}

bool ResultCl::isSingleValReturnType() {
    return elements.size() == 1;
}

EvalClType PatternCl::getEvalClType() {
    return EvalClType::Pattern;
}

string PatternCl::format()
{
    return "\nPATTERN " + synonym->format() + " (" + entRef->format() + ", " + exprSpec->format() + ")";
}

bool PatternCl::containsSynonym(shared_ptr<Element> s)
{
    return synonym->getSynonymString() == s->getSynonymString() ||
        (entRef->getEntRefType() == EntRefType::SYNONYM && entRef->getStringVal() == s->getSynonymString());
}

const vector<string>& PatternCl::getAllSynonymsAsString()
{
    return synonymsUsed;
}

const string& PatternCl::getSynonymType(unordered_map<string, shared_ptr<Declaration>>& synonymToParentDeclarationMap)
{
    auto it = synonymToParentDeclarationMap.find(synonym->getValue());
    if (it == synonymToParentDeclarationMap.end()) throw "Failed to resolve synonym. It was not declared";
    return (*it).second->getDesignEntity()->getEntityTypeName();
}
PatternClType PatternCl::getPatternClType(unordered_map<string, shared_ptr<Declaration>>& synonymToParentDeclarationMap)
{
    const string& synonymType = getSynonymType(synonymToParentDeclarationMap);
    if (synonymType == DesignEntity::ASSIGN) return PatternClType::PatternAssign;
    else if (synonymType == DesignEntity::WHILE) return PatternClType::PatternWhile;
    else if (synonymType == DesignEntity::IF) return PatternClType::PatternIf;
    else {
        throw "Unsupported Synonym Type for Pattern Clause";
    }
}

EvalClType WithCl::getEvalClType() {
    return EvalClType::With;
}

string WithCl::format()
{
    return "\nWITH (" + lhs->format() + ") = (" + rhs->format() + ")\n";
}

bool WithCl::containsSynonym(shared_ptr<Element> s)
{
    const string& toCheck = s->getSynonymString();
    for (auto& x : synonymsUsed) {
        if (x == toCheck) return true;
    }

    return false;
}

const vector<string>& WithCl::getAllSynonymsAsString()
{
    return synonymsUsed;
}


shared_ptr<Declaration>& SelectCl::getParentDeclarationForSynonym(const string& s)
{
    if (synonymToParentDeclarationMap.find(s) == synonymToParentDeclarationMap.end())
    {
        throw "Warning: requested synonym of value [" + s +
            "] is NOT declared in this SelectCl. Null DesignEntityType is "
            "returned.\n";
    }
    return synonymToParentDeclarationMap[s];
}

shared_ptr<Declaration>& SelectCl::getParentDeclarationForSynonym(shared_ptr<Synonym> s)
{
    if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end())
    {
        throw "Warning: requested synonym of value [" + s->getValue() +
            "] is NOT declared in this SelectCl. Null DesignEntityType is "
            "returned.\n";
    }
    return synonymToParentDeclarationMap[s->getValue()];
}

bool SelectCl::isSynonymDeclared(string toTest)
{
    return synonymToParentDeclarationMap.find(toTest) != synonymToParentDeclarationMap.end();
}

const string& SelectCl::getDesignEntityTypeBySynonym(const string& s)
{
    if (synonymToParentDeclarationMap.find(s) == synonymToParentDeclarationMap.end())
    {
        string toThrow = "Warning: requested synonym of value [" + s +
            "] is NOT declared in this SelectCl. Null "
            "DesignEntityType is returned.\n";
        throw toThrow;
    }

    return synonymToParentDeclarationMap[s]->getDesignEntity()->getEntityTypeName();
}

const string& SelectCl::getDesignEntityTypeBySynonym(const shared_ptr<Synonym>& s)
{
    if (synonymToParentDeclarationMap.find(s->getValue()) == synonymToParentDeclarationMap.end())
    {
        cout << "Warning: requested synonym of value [" << s->getValue()
            << "] is NOT declared in this SelectCl. Null DesignEntityType is "
            "returned.\n";
        throw "Synonym " + s->getValue() + " not declared, cannot resolve it's DesignEntityType";
    }

    return synonymToParentDeclarationMap[s->getValue()]->getDesignEntity()->getEntityTypeName();
}

string SelectCl::format()
{
    string builder = "";
    for (auto& d : declarations)
    {
        builder += d->format() + ", ";
    }

    builder += "\nSELECT " + target->format();

    for (auto& st : suchThatClauses)
    {
        builder += st->format();
    }

    for (auto& pt : patternClauses)
    {
        builder += pt->format();
    }

    for (auto& wt : withClauses)
    {
        builder += wt->format();
    }

    return builder;
}

bool SelectCl::hasSuchThatClauses()
{
    return suchThatClauses.size() > 0;
}

bool SelectCl::hasPatternClauses()
{
    return patternClauses.size() > 0;
}

bool SelectCl::hasWithClauses()
{
    return withClauses.size() > 0;
}

bool SelectCl::hasEvalClauses()
{
    return hasSuchThatClauses() || hasPatternClauses() || hasWithClauses();
}

const vector<shared_ptr<EvalCl>>& SelectCl::getEvalClauses()
{
    return evalClauses;
}

bool SelectCl::suchThatContainsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    for (auto& st : this->suchThatClauses)
    {
        flag = st->containsSynonym(s);
        if (flag)
            break;
    }
    return flag;
}

bool SelectCl::patternContainsSynonym(shared_ptr<Element> s)
{
    bool flag = false;
    for (auto& pt : this->patternClauses)
    {
        flag = pt->containsSynonym(s);
        if (flag)
            break;
    }
    return flag;
}

bool SelectCl::withContainsSynonym(shared_ptr<Element> e) {
    bool flag = false;
    for (auto& pt : this->withClauses)
    {
        flag = pt->containsSynonym(e);
        if (flag)
            break;
    }

    return flag;
}


const shared_ptr<ResultCl>& SelectCl::getTarget() {
    return target;
}