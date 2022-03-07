#include "include/ParserInterface.h"

int main(){
    // Init ParserInterface Object
    ParserInterface p;
    // Load parsed query AST.
    p.loadAndParse();
    // Get an AST and store in res. Now the res pointer is the one you want to manipulate with.
    // hsql::SQLParserResult* res = p.getStmt(1);
    
    // This print function is for debug purpose.
    p.print(1);
    return 0;
}