#include "algebra.h"
#include "core/utils/types.h"

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


Expression *gcd(Expression *a, Expression *b) {
    if (a->type != EXPRESSION_INTEGER || b->type != EXPRESSION_INTEGER) {
        printf("GCD expects two integers as inputs.\n");
        return NULL;
    }
    
    Expression *new = dummyExpression(EXPRESSION_INTEGER);
    if (new == NULL) return NULL;
    
    new->integer = _gcd(a->integer, b->integer);
    return new;
}