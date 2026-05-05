#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "operations.h"
#include "core/context/context.h"
#include "core/primitives/types.h"
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


Operation *createOperation(const char symbol, associativity a, bool c, uint32_t precedence) {
	Operation *op = malloc(sizeof(Operation));
	if (op == NULL) {
		perror("Error creating operation");
		return NULL;
	}

	op->symbol = symbol;
	op->associativity = a;
	op->commutative = c;
	op->arity = 2;
	op->precedence = precedence;

	op->implementationSize = DEFAULT_IMPLEMENTATIONS;
	op->nImplementations = 0;
	op->implementations = malloc(sizeof(BuiltinImplementation) * op->implementationSize);
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


int addOperationImplementation(Operation *op, BuiltinImplementation fn){
	if (op->nImplementations >= op->implementationSize) { 
		op->implementationSize += DEFAULT_IMPLEMENTATIONS;
		BuiltinImplementation *temp = realloc(op->implementations, sizeof(BuiltinImplementation) * op->implementationSize);
		if (temp == NULL) {
			perror("Error registering operation implementation");
			return 0;
		}

		op->implementations = temp;
	}
	op->implementations[op->nImplementations] = fn;
	op->nImplementations ++;
	return 1;
}


int initPrimitiveOperations() {
	Operation *addition = createOperation('+', ASSOC_BOTH, true, 1);
	if (addition == NULL) goto error; 

	if (!addOperationImplementation(addition, add)) {
		freeOperation(addition);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, addition)) return 0;
    
    
	Operation *multiplication = createOperation('*', ASSOC_BOTH, true, 2);
	if (multiplication == NULL) goto error;

	if (!addOperationImplementation(multiplication, multiply)) {
		freeOperation(multiplication);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, multiplication)) return 0;
    
    
	Operation *exponentiation = createOperation('^', ASSOC_RIGHT, false, 3);
	if (exponentiation == NULL) goto error;
    
	if (!addOperationImplementation(exponentiation, exponent)) {
		freeOperation(exponentiation);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, exponentiation)) return 0;
    
    
   	Operation *division = createOperation('/', ASSOC_LEFT, false, 2);
	if (division == NULL) goto error;

	if (!addOperationImplementation(division, divide)) {
		freeOperation(division);
		goto error;
	}

    if (!registerOperation(GLOBALCONTEXT->registry, division)) return 0;
    
    
	Operation *subtraction = createOperation('-', ASSOC_LEFT, false, 1);
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
