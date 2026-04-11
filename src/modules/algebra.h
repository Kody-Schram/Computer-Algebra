#ifndef ALGEBRA_H
#define ALGEBRA_H

#include "core/utils/types.h"


long long _powi(long long a, long long e);


long long _gcd(long long a, long long b);


ASTNode *gcd(ASTNode *a, ASTNode *b);


int simplify(ASTNode **ast);


#endif