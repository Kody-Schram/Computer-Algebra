#include "algebra.h"
#include "core/utils/log.h"

long long _gcd(long long a, long long b) {
    long long temp;
    while (b != 0)
    {
        temp = a % b;

        a = b;
        b = temp;
    }
    return a;
}


long long _powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


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