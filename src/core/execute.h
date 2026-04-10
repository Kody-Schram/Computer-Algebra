#ifndef EXECUTE_H
#define EXECUTE_H

#include "core/utils/types.h"


int evaluateAddition(ASTNode **ast);


int evaluateSubtraction(ASTNode **ast);


int evaluateMultiplication(ASTNode **ast);


int evaluateDivision(ASTNode **ast);


int evaluateExponentiation(ASTNode **ast);


int execute(ASTNode **ast);

#endif