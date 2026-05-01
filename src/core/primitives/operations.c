#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "operations.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"

#define DEFAULT_IMPLEMENTATIONS 2
#define BUILTIN_FUNCTION_POINTER_SIZE sizeof(BuiltinResult (*)(unsigned int, Expression **))


BuiltinResult add(unsigned int nArgs, Expression **operands) {
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


BuiltinResult subtract(unsigned int nArgs, Expression **operands) {
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


BuiltinResult multiply(unsigned int nArgs, Expression **operands) {
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


static long long _powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


BuiltinResult exponent(unsigned int nArgs, Expression **operands) {
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


BuiltinResult divide(unsigned int nArgs, Expression **operands) {
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


Operation *createOperation(const char symbol, bool lA, bool rA, bool c, unsigned int precedence) {
	Operation *op = malloc(sizeof(Operation));
	if (op == NULL) {
		perror("Error creating operation");
		return NULL;
	}

	op->symbol = symbol;
	op->leftAssociative = lA;
	op->rightAssociative = rA;
	op->commutative = c;
	op->arity = 2;
	op->precedence = precedence;

	op->implementationSize = DEFAULT_IMPLEMENTATIONS;
	op->nImplementations = 0;
	op->implementations = malloc(BUILTIN_FUNCTION_POINTER_SIZE * op->implementationSize);
	if (op == NULL) {
		perror("Error creating operation");
		free(op);
		return NULL;
	}

	return op;
}


void freeOperation(Operation *op) {
	free(op->implementations);
	free(op);
}


int addOperationImplementation(Operation *op, BuiltinResult (*function) (unsigned int, Expression **)) {
	if (op->nImplementations >= op->implementationSize) { 
		op->implementationSize += DEFAULT_IMPLEMENTATIONS;
		BuiltinResult (*(*temp)) (unsigned int, Expression **) = realloc(op->implementations, BUILTIN_FUNCTION_POINTER_SIZE * op->implementationSize);
		if (temp == NULL) {
			perror("Error registering operation implementation");
			return 0;
		}

		op->implementations = temp;
	}
	op->implementations[op->nImplementations] = function;
	op->nImplementations ++;
	return 1;
}


int initPrimitiveOperations() {
	Operation *addition = createOperation('+', true, true, true, 1);
	if (addition == NULL) goto error; 

	if (!addOperationImplementation(addition, add)) {
		freeOperation(addition);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, addition)) return 0;
    
    
	Operation *multiplication = createOperation('*', true, true, true, 2);
	if (multiplication == NULL) goto error;

	if (!addOperationImplementation(multiplication, multiply)) {
		freeOperation(multiplication);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, multiplication)) return 0;
    
    
	Operation *exponentiation = createOperation('^', false, true, false, 3);
	if (exponentiation == NULL) goto error;
    
	if (!addOperationImplementation(exponentiation, exponent)) {
		freeOperation(exponentiation);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, exponentiation)) return 0;
    
    
   	Operation *division = createOperation('/', true, false, false, 2);
	if (division == NULL) goto error;

	if (!addOperationImplementation(division, divide)) {
		freeOperation(division);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, division)) return 0;
    
    
	Operation *subtraction = createOperation('-', true, false, false, 1);
	if (subtraction == NULL) goto error;

	if (!addOperationImplementation(subtraction, subtract)) {
		freeOperation(subtraction);
		goto error;
	}
    
    if (!registerOperation(GLOBALCONTEXT->registry, subtraction)) return 0;
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        return 0;
}
