#include "algebra.h"
#include "utils/log.h"
#include "utils/types.h"


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