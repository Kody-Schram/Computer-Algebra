#ifndef ALGEBRA_H
#define ALGEBRA_H

#include "utils/types.h"


long int gcd_dev(long int a, long int b);


ASTNode *gcd(ASTNode *a, ASTNode *b);


int simplify(ASTNode **ast);


#endif