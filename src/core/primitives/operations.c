#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "operations.h"
#include "core/context/context.h"
#include "core/context/registry.h"
#include "core/primitives/types.h"
#include "core/utils/log.h"
#include "core/utils/type_utils.h"


#define DEFAULT_IMPLEMENTATIONS 2

BuiltinResult add(uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if ((a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE) ||
        (b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE)) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) goto error;

        output->integer = a->integer + b->integer;

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) goto error;
    output->value = av + bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
    
    error:
        result.type = BUILTIN_ERROR;
        return result;
}


BuiltinResult subtract(uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if ((a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE) ||
        (b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE)) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) goto error;

        output->integer = a->integer - b->integer;

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) goto error;
    output->value = av - bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
    
    error:
        result.type = BUILTIN_ERROR;
        return result;
}


BuiltinResult multiply(uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if ((a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE) ||
        (b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE)) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) goto error;

        output->integer = a->integer * b->integer;

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) goto error;
    output->value = av * bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
    
    error:
        result.type = BUILTIN_ERROR;
        return result;
}


static uint64_t _powi(uint64_t a, uint64_t e) {
    uint64_t r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


BuiltinResult exponent(uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if ((a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE) ||
        (b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE)) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) goto error;

        output->integer = _powi(a->integer, b->integer);

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) goto error;
    output->value = powf(av, bv);

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
    
    error:
        result.type = BUILTIN_ERROR;
        return result;
}


BuiltinResult divide(uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    if (GLOBALCONTEXT->config->PRESERVE_FRACS) {
        result.type = BUILTIN_NEUTRAL;
        return result;
    }

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if ((a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE) ||
        (b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE)) return result;

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) goto error;
    output->value = av / bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
    
    error:
        result.type = BUILTIN_ERROR;
        return result;
}


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t precedence) {
	BuiltinImplementation *implementations = malloc(sizeof(BuiltinImplementation) * DEFAULT_IMPLEMENTATIONS);
	if (implementations == NULL) {
		perror("Error creating operation");
		return false;
	}

	(*out) = (Operation) {
		.symbol = symbol,
		.associativity = a,
		.commutative = c,
		.arity = 2,
		.precedence = precedence,
		.implementationSize = DEFAULT_IMPLEMENTATIONS,
		.nImplementations = 0,
		.implementations = implementations
	};
	
	return true;
}


void freeOperation(Operation *op) {
	free(op->implementations);
	free(op);
}


bool initPrimitiveOperations() {
	Debug(0, "initing primitive ops\n");
	Operation add;
	if (!createOperation(&add, '+', ASSOC_BOTH, true, 1)) return false;
	if (!registerOperation(GLOBALCONTEXT->registry, add)) return false;

	Debug(0, "mult\n");
	Operation mult;
	if (!createOperation(&mult, '*', ASSOC_BOTH, true, 2)) return false;
	if (!registerOperation(GLOBALCONTEXT->registry, mult)) return false;

	Debug(0, "expo\n");
	Operation expo;
	if (!createOperation(&expo, '^', ASSOC_RIGHT, false, 3)) return false;
	if (!registerOperation(GLOBALCONTEXT->registry, expo)) return false;

	Debug(0, "div\n");
   	Operation div;
	if (!createOperation(&div, '/', ASSOC_LEFT, false, 2)) return false;
	if (!registerOperation(GLOBALCONTEXT->registry, div)) return false;

	Debug(0, "sub\n");
	Operation sub;
	if (!createOperation(&sub, '-', ASSOC_LEFT, false, 1)) return false;
	if (!registerOperation(GLOBALCONTEXT->registry, sub)) return false;

	Debug(0, "return from op init\n");
    return true;
}
