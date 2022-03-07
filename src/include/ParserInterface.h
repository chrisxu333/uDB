#ifndef UDB_PARSER_INTERFACE_H
#define UDB_PARSER_INTERFACE_H
#include "SQLParser.h"
// contains printing utilities
#include "util/sqlhelper.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class ParserInterface{
    private:
        // TODO: Use shared_ptr later on.
        std::vector<hsql::SQLParserResult*>* stmts_;
    public:
        ParserInterface(){
            stmts_ = new std::vector<hsql::SQLParserResult*>();
        };
        ~ParserInterface(){
            for(size_t i = 0; i < stmts_->size(); ++i) delete (*stmts_)[i];
            delete stmts_;
        };
        // Restrict usage of copy constructor.
        ParserInterface(const ParserInterface&) = delete;
        // Restrict usage of copy assignment operator.
        ParserInterface& operator=(const ParserInterface&) = delete;

        // TODO: Restrict usage of move constructor.

        // Contruct in-memory parse tree given test queries under ./test folder.
        void loadAndParse();
        // Get individual statement in stmt_.
        hsql::SQLParserResult* getStmt(const size_t);
        // Helper function to visualize the parsed statement.
        void print(const size_t);
};
#endif  //UDB_PARSER_INTERFACE_H