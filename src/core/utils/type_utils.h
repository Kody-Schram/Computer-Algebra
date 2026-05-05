#pragma once

#include <stdio.h>

#include "core/primitives/types.h"


Expression *dummyExpression(ExpressionType type);


Expression *deepCopyExpression(Expression const *expr);


FILE *printExpression(Expression const *expr);


void freeExpression(Expression *expr);


char *expressionToString(Expression const *expr);


void freeFunction(Function *func);
