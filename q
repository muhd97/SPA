[33m3a7a5ae[m[33m ([m[1;36mHEAD -> [m[1;32mmaster[m[33m, [m[1;31morigin/master[m[33m, [m[1;31morigin/HEAD[m[33m)[m Update TestPQLParser test cases
[33m623d616[m Add parsing support for new result types
[33m367399c[m Add ResultCl to PQLParser AST
[33mdc791eb[m Fix exception handling bug in PQL parser
[33m56bac39[m Add exception handling in simple parser
[33me4f0ac1[m[33m ([m[1;33mtag: submission[m[33m)[m Iteration 1 submission
[33mec8a79e[m Edit faulty test cases
[33m65d57f6[m Bug fix: Join algorithm was buggy, iterated through keys of leftTuple only. Need to iterate through keys ot rightTuple too
[33mf75cebc[m Bug fix: Check if targetSynonym exists in ResultTuples first before extracting
[33m90c284c[m Clean up TestWrapper.cpp
[33m27c6cfe[m Format codebase
[33m4b3f3ea[m Format codebase
[33m75b5825[m fix bug: handleFollowsTFirstArgInteger
[33m77f2d64[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m3f395a1[m end to end for followsT
[33m12cc2da[m Merge pull request #18 from nus-cs3203/wk6_manas
[33mf7f46fe[m[33m ([m[1;31morigin/wk6_manas[m[33m)[m Merge branch 'master'  into wk6_manas
[33mf338f8b[m Add regression tests for Follows
[33m2e5b369[m Bug fixes for Follows
[33m2e2342e[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m6a2658f[m remove couts in pattern
[33me269c38[m Update FollowsT in PQLProcessor.cpp Update error in test cases for Follows*
[33ma3231c9[m Add Follow*(s1, ?) support
[33mb2574cf[m Bug fix: Join algorithm can now join on multiple keys
[33mbec5f9a[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md9b8a0e[m final pattern fixes
[33m4c619b8[m Refine some error handling in PQLProcessor.cpp
[33m405e8a8[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mf16935d[m more fix : PQLProcessor::handlePatternClause
[33md2f6110[m Bug fix: Checking if targetSynonym is in clauses was BUGGED
[33m668bee9[m query file change
[33mb1a6254[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mc28a208[m Update some scripts to output fail if some test cases fail
[33md4ccc23[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md87aee4[m handle pattern clause
[33mdf00157[m Fix first system acceptance test
[33m0ab18f9[m Merge pull request #17 from nus-cs3203/wk6_manas
[33m84df373[m Fix errors in follows test cases
[33m25dd435[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into wk6_manas
[33maa1555f[m Merge pull request #16 from nus-cs3203/system-test-cases
[33m7dc2d45[m Remove getSubExpressions from SimpleAST
[33m48b457c[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into wk6_manas
[33mb2e8244[m Add positive test cases for Follows (non-special tuples)
[33me4fa0a5[m Almost fully implement Follows
[33m96fab83[m Add necessary methods for Follows to eval
[33mac29611[m Bug fix: Parent(s, s) and Parent*(s, s) cannot be true for any statements s
[33mc257713[m Remove unnecessary cout statements
[33m01b17bc[m Bug fix: Issue-uses-1 While with Uses was buggy
[33md5a2e7d[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m0972c6a[m Add scripts to run individual tests
[33m82ce0b1[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m7cb2b7e[m Fix parent* keyword parsing
[33m0dc47b5[m Remove unnecessary couts
[33m32e5d33[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md85f56f[m Finish ParentT and add validation of PQL Query. All synonyms used in PQL Query must be declared first
[33ma9930cf[m Add system acceptance cases Organise Files
[33m831c4ac[m Added Follows test cases
[33m2431b4c[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md1e62f9[m Remove reserved keywords from pql lexer & parser
[33m91951fc[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mf22128f[m Bug fix for Parent(syn, int)
[33mda8517a[m Add line number to simple parser errors
[33mfc17290[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md1a4780[m Implementing Parents*(syn, ?)
[33m3afdbf0[m Fix PQL processor error
[33m054b504[m Organise Files
[33mfe7e763[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m179b295[m Finish Parents*(INT, ?)
[33m96222c6[m Fix interger overflow error in simpler parser
[33mf4a8b93[m Fix interger cannot start with 0 lexer error
[33ma8cc272[m Allow keywords as names in simple parser
[33mcc32468[m Update SPA.vcxproj
[33m97a21e6[m Update SPA.vcxproj.filters
[33m46f39d5[m Organise Files
[33mb5bf118[m Fix unused variable compile warning
[33m5cf13e6[m Fix unsigned int cast warning
[33m8b1c92f[m implement pkbevaluator::getConstants() with unit tests
[33m88e4c75[m add unit tests for pattern
[33m62b75ba[m add pattern in pkbevaluator
[33m0c28920[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m8cdae15[m add more evaluator follows test
[33m3620b30[m Procedures cannot be Parents of other statements, and cannot be children
[33m902bb13[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m2f723e0[m Identation fix
[33m5402958[m Finish Parents(_, ?) case
[33m7ee22b8[m fix getStatement()
[33ma50349b[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m0bb0647[m half implementation of revised parents
[33m5d0be12[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m81471f8[m Finish Parents(INT, ?)
[33mf59aa7d[m Merge pull request #15 from nus-cs3203/Modifies_PQLProcessor
[33m57411e8[m[33m ([m[1;31morigin/Modifies_PQLProcessor[m[33m)[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into Modifies_PQLProcessor
[33m27e4bdf[m Fix merge complications
[33mbe6c87d[m Change PQLEvaluator return types
[33medfcfaf[m use set<pair<int, int>> instead of vector<pair<int, int>>
[33m437f4cf[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m8bb5cbf[m temporary non-working but compiles
[33mfda1a1c[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m780fc2e[m Starting on Parent relationships
[33ma5879df[m change vector to set for parents, children, Ts
[33mac13e29[m pkbeval: has children
[33ma59ff67[m some PKBEvaluator tests
[33m2d61bbb[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into Modifies_PQLProcessor
[33m4fe7b32[m merge
[33m6246504[m Bug Fix: multiple in follows add pkbevaluator tester
[33m7e61bf7[m Merge branch into Modifies_PQLProcessor
[33mbecefdb[m Cleaning up some messages and exception handling
[33m120538c[m Add error handling for UsesP when right side synonym is not declared Variable
[33m7075d98[m Fix indexing bug in getStatement
[33m6416702[m Fix indentation
[33m5d15543[m Todo: Implemeent function in Evaluator to extract unordered_set of all Constants from program
[33m61949f1[m Bug fix: PKB.cpp check identifiers,consequentGroup AND altGroup in If statement when deciding whether to add to set of all Statements that Use vars
[33m0dfceb9[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m58179ab[m Bug fix: Yida accidentally edited out-of-range-check in PKB getStatement(int idx)
[33m6869dcb[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into Modifies_PQLProcessor
[33ma451b60[m Add tests for positive cases for Modifies for yida_src1
[33mfb05d86[m Completed implementation of Modifies with tuples
[33mb7b9b49[m Add check in pkb to get modifying statements
[33m7ceeb2f[m Fix sample queries for Follows*
[33m3905744[m Update Follows in PQLProcessor.cpp
[33m17f71d6[m Update FollowsT in PQLProcessor.cpp
[33m4f2f48d[m Update Follows in PQLProcess.cpp
[33m9527d28[m Bug fix: getStatementAfter going out of range
[33m71090c5[m Fix merge mistake
[33m2a6ae31[m Merge branch 'master' into Modifies_PQLProcessor
[33m353f6db[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m3a3651a[m Bug Fix: Modifies was not parsed correctly. Eat Modifies token first before check for ModifiesS or ModifiesP
[33mb84beaa[m Preliminary implementation of ModifiesP
[33m42d5355[m Modifies parsing bug fix
[33m2b0e7c2[m Update Follows in PQLProcessor.cpp
[33md25450b[m Minor style changes
[33ma0f969d[m Use placeholder synonyms for RelRefs without any synonym
[33m5b19306[m Refactor evaluation of each RelRef to return 1-tuples or 2-tuples instead of single values
[33m228a1b0[m Fix merge conflicts
[33md51e834[m Refactoring PQLProcessor
[33m39b7963[m Update Follows in PQLProcessor.cpp
[33mccafe98[m Update Follows in PQLProcessor.cpp
[33mf62cfa8[m Throw error when there are duplicate synonyms in PQLQuery
[33m0563318[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mc863a93[m Finish Uses cases, including when TargetSynonym not in Uses clause
[33m1e25cfe[m Add helper methods to get size() of collections without returning copies directly
[33m101fb55[m Add Follows in PQLProcessor
[33m50e2174[m Add helper method to retrieve DesignEntityType of given declared Synonym in SelectCl
[33mf732956[m fix bug: getBefore, getAfter: continue instead of break
[33m71e8a52[m remove redundancy in pkb extract call stmt
[33m57cd80f[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mdedf591[m fix bug: if/while/call adds itself to var->addUserStatement() / var->addModifierStatement()
[33m820a602[m Bug fix: check existence of keys first in PKB maps
[33mb99f1ef[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m274e447[m Add Uses cases where 2nd arg is IDENT
[33m52651f4[m bug fix: getBefore getAfter: edge cases at first and last statement
[33m72cf187[m fix fatal bug caused by fixing previous fatal bug
[33m2f5eea7[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33me25716d[m bug fix: getAfter, getBefore
[33mbb4e639[m Bug fix: PQLParser containsSynonym functions were broken
[33m3ee17b6[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m219ed40[m Merge pull request #14 from nus-cs3203/Modifies_PQLProcessor
[33md14df3b[m Add verify clauses method in PQLProcessor
[33m1e89ea1[m Merge branch 'master' into Modifies_PQLProcessor
[33m5cf55c9[m Bare minimum queries for ModifiesS
[33m6f6711d[m Fix bug in the ModifiesS for assignment stmts
[33m0cfa124[m Use _ design entity to get all the modifiers
[33m55c8402[m Fix containsSynonym check for Modifies
[33mb682995[m Fix parsing bug for Modifies
[33m33cd042[m Add Synonym class to PQL parser
[33me7076f8[m Make PQLProcessor more readable. Abstract methods
[33m3e8c43d[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mb9038a0[m correct some wanted renaming of _ to AllExceptProcedure
[33m77e571d[m Fix PQL format bug
[33md5108e6[m change _ to AllExceptProcedure
[33mdf76d9c[m fix bug: return modifyStmts instead of useStmts
[33m7b995cc[m fix bug: get modified variables instead of used variable
[33m18c495e[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m31bbb52[m documentation for PQLEvaluator api
[33md97a0db[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mafcaf8c[m Move instance variable in TestWrapper.h to private section to comply with requirements
[33m4fb8ce1[m Add logging (cout) when exception is caught in TestWrapper.cpp
[33m41f93d0[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mf2a87be[m Add format to PQL AST
[33ma94a10e[m Fix Uses(s, v) where s is a STMT synonym
[33m0dff059[m Fix method to getDesignEntity
[33m0e5d4ee[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into Modifies_PQLProcessor
[33m0a927d2[m Implement Modifies_S
[33me17375d[m Add better comments
[33mdfc8524[m Add more Uses test cases for nested containers and nested Calls
[33mdd11feb[m Add Uses cases for container statements, calls and procedures.
[33m78a7306[m Parse into UsesP and ModifiesP if first arg is IDENT
[33m8c4a5d4[m Bug fix: PKBStatement.h addModifiedVariables was inserting into mUses
[33me9b6beb[m Bug fix: PKBStatement.h addModifiedVariables was inserting into mUses
[33m5d13160[m Add utility functions to match synonym types in PQLProcessor.cpp
[33m7420ba8[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m89418d8[m Implement more Uses cases. Uses for procedure and call and containers not implemented yet
[33me2fbba1[m Add output.xml to .gitignore
[33m4062361[m Make simple expression parser left recursive
[33m1e76959[m Bug fix: getStatement() takes in 1-based stmtNumber. Stored vector is 0-based indexing.
[33m33f02db[m Bug fix: addUsedVariable() was adding to PKBDesign::Assign only
[33m2e9078f[m childrenT of PKBEvaluator (forgot to implement this function previously)
[33ma545aba[m extractIf, extractWhile, extractCall changes to extractProcedure
[33m5c5228f[m Update unit tests for PQLParser
[33m3b004ba[m E2E flow started for Uses
[33mf5dde4d[m[33m ([m[1;31morigin/Yida-PKB-DesignExtractor-Integration[m[33m)[m Integrate DesignExtractor into PKB
[33m828a0ab[m Standardize SPtr to shared_ptr
[33mc7cedfe[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24 into master
[33m35f5002[m Add unit tests for PQLParser
[33mce6c40a[m Add unit tests for PQLLexer
[33m8969161[m Add pattern expression support in PQL parser
[33mfd77921[m PKBStatement::getContainerGroups() now returns vector of up to 2 container groups, which is needed because If Statements can hold two different container grps
[33macb0411[m additional getters in SimpleAST extractAssign, extractRead, extractPrint done
[33m4622242[m added types for SIMPLE Expressions
[33maa7e5c3[m added basic structure for CraftingBench (probably called DesignExtractor in future)
[33m5d5f953[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33md27718e[m Revert "added crafting table"
[33m4daa335[m added crafting table
[33m76e24ad[m E2E flow with manually populated PKB
[33mc29f7cb[m Add unit testing for PQLLexer
[33m5773fa6[m Merge pull request #13 from nus-cs3203/muhd97-patch-1
[33ma57af94[m Add string token to PQLLexer
[33me953e85[m[33m ([m[1;31morigin/muhd97-patch-1[m[33m)[m Create c-cpp.yml
[33mca462a9[m Merge pull request #12 from nus-cs3203/GetStatementType
[33mea9dd3c[m[33m ([m[1;31morigin/GetStatementType[m[33m, [m[1;32mGetStatementType[m[33m)[m Fix build
[33m729d098[m Add shared_ptr to the SimpleParser
[33md72ead1[m Add getStatementType and shared_ptr to SimpleAST
[33mba95e6f[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33m6896cf9[m Fix the system acceptance test case for iteration 1
[33m6117bb3[m fixed PKBGroup static int initialization warning
[33m019589a[m Merge branch 'PKB' into master
[33mb244214[m added uses and modifies in evaluator, cleaned up code
[33m9ddecfc[m Add a system acceptance test case for iteration 1
[33m55cccce[m commit for before meeting
[33mddf64ad[m parent and child mostly finished, started follow
[33m65c2dab[m if i lose my code one more time, i will flip
[33m9d651c4[m [WIP] Add pattern clause to PQL parser (2/x)
[33mabaa3f6[m [WIP] Add pattern clause to PQL parser (1/x)
[33m0968d51[m Integrate PQL lexer and parser to test wrapper
[33m7209958[m Edit readme
[33md6b04a0[m Add getSubexpression to expression
[33m505e60f[m Print expressions using parentheses
[33m50569d2[m Rename AST to SimpleAST
[33md86089c[m Fix typo breaking build
[33m276df9d[m Update README.md
[33m6a15d53[m Update README.md
[33m16abdaa[m Merge branch 'master' of https://github.com/nus-cs3203/21s1-win-spa-team-24
[33mb770c68[m Fix solution structure
[33me6a808a[m Update contacts in README.md
[33ma02fdb0[m Add PQLParser, Pattern not supported yet
[33m9ce92cd[m Add simple parser
[33m659552d[m Initial commit
