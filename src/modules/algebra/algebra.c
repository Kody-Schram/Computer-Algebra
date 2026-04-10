#include "algebra.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


ASTNode *gcd(ASTNode *a, ASTNode *b) {
    if (a->type != NODE_INTEGER || b->type != NODE_INTEGER) {
        printf("GCD expects two integers as inputs.\n");
        return nullptr;
    }
    
    ASTNode *new = dummyASTNode(NODE_INTEGER);
    if (new == nullptr) return nullptr;
    
    new->integer = _gcd(a->integer, b->integer);
    return new;
}


static int simplifyRecur(ASTNode **ptr) {
    if (ptr == nullptr) return 1;
    ASTNode *ast = *ptr;
    
    switch (ast->type) {
        case NODE_OPERATOR:
        simplifyRecur(&ast->left);
        simplifyRecur(&ast->right);
        
        // Handle associativity
            
        default:
            break;
    }
    
    return 1;
}


int simplify(ASTNode **ast) {
    Debug(0, "Simplifying AST");
    Debug(1, printAST(*ast));
    
    simplifyRecur(ast);
    
    return 1;
}