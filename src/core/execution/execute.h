#ifndef EXECUTE_H
#define EXECUTE_H

#include "core/utils/types.h"


int evaluateAddition(Expression **ast);


int evaluateSubtraction(Expression **ast);


int evaluateMultiplication(Expression **ast);


int evaluateDivision(Expression **ast);


int evaluateExponentiation(Expression **ast);


int execute(Expression **ast);

#endif