#include "functions.h"
#include "execution/algebra/algebra.h"
#include "utils/types.h"

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