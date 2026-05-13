#pragma once

#include <stdio.h>

#include "core/primitives/types.h"


Expression *dummyExpression(ExpressionType type);


Expression *deepCopyExpression(Expression const *expr);


FILE *printExpression(Expression const *expr);


void freeExpression(Expression *expr);


char *expressionToString(Expression const *expr);


void freeFunction(Function *func);


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t operands);


bool createObject(
		Object *out,
		char const *identifier, char const *originModule,
		void (*cleanup)(void *data), int32_t (*compare)(void const *a, void const *b)
);
