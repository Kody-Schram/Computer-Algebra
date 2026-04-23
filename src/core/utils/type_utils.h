#ifndef TYPE_UTILS_H
#define TYPE_UTILS_H

#include <stdio.h>
#include "core/primitives/types.h"


Expression *dummyExpression(ExpressionType type);


Expression *deepCopyExpression(const Expression *expr);


FILE *printExpression(const Expression *expr);


void freeExpression(Expression *expr);


char *expressionToString(const Expression *expr);


void freeFunction(Function *func);

#endif