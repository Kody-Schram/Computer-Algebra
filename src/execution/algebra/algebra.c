#include "algebra.h"
#include "utils/log.h"
#include "utils/types.h"


static int simplifyRecur(ASTNode **ast) {
    if (ast == NULL) return 1;
    
    return 1;
}


int simplify(ASTNode **ast) {
    Debug(0, "Simplifying AST");
    Debug(1, printAST(*ast));
    
    simplifyRecur(ast);
    
    return 1;
}