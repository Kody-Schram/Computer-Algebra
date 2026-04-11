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


long long _powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


Expression *gcd(Expression *a, Expression *b) {
    if (a->type != EXPRESSION_INTEGER || b->type != EXPRESSION_INTEGER) {
        printf("GCD expects two integers as inputs.\n");
        return nullptr;
    }
    
    Expression *new = dummyExpression(EXPRESSION_INTEGER);
    if (new == nullptr) return nullptr;
    
    new->integer = _gcd(a->integer, b->integer);
    return new;
}