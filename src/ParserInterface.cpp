#include "include/ParserInterface.h"

void ParserInterface::loadAndParse(){
    // Load query.
    std::ifstream test_in("test/queries/queries-good.txt");
    std::string q;
    while(std::getline(test_in, q)){
        hsql::SQLParserResult* result = new hsql::SQLParserResult();
        hsql::SQLParser::parse(q, result);
        stmts_->push_back(result);
    }
}

hsql::SQLParserResult* ParserInterface::getStmt(const size_t i){
    return (*stmts_)[i];
}

void ParserInterface::print(const size_t i){
    hsql::SQLParserResult* res = getStmt(i);
    if(res->isValid()){
        for (auto i = 0u; i < res->size(); ++i) {
        // Print a statement summary.
        hsql::printStatementInfo(res->getStatement(i));
        }
    }
}

