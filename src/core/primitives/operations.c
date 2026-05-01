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



int registerOperation(Operation *op) {
    char *id = malloc(sizeof(char) * 2);
    if (id == NULL) {
        perror("Error registering operation");
        return 0;
    }
    id[0] = op->symbol;
    id[1] = '\0';
    
    int result = bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, id, op);
    free(id);
    
    return result;
}


static int initOperationImplementations(Operation *op, BuiltinResult (*function) (unsigned int, Expression **)) {
	op->implementationSize = DEFAULT_IMPLEMENTATIONS;
	op->nImplementations = 1;

	op->implementations = malloc(BUILTIN_FUNCTION_POINTER_SIZE * op->implementationSize);
	if (op->implementations == NULL) return 0;

	op->implementations[0] = function;

	return 1;
}

int registerImplementation(Operation *op, BuiltinResult (*function) (unsigned int, Expression **)) {
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
    Operation *addition = NULL;
    Operation *multiplication = NULL;
    Operation *exponentiation = NULL;
    
    Operation *division = NULL;
    Operation *subtraction = NULL;
    
    
    addition = malloc(sizeof(Operation));
    if (addition == NULL) goto error;

    addition->leftAssociative = true;
    addition->rightAssociative = true;
    addition->commutative = true;
    addition->symbol = '+';
    addition->arity = 2;
	addition->precedence = 1;
	if (!initOperationImplementations(addition, add)) {
		free(addition);
		goto error;
	}

    if (!registerOperation(addition)) return 0;
    
    
    multiplication = malloc(sizeof(Operation));
    if (multiplication == NULL) goto error;

    multiplication->leftAssociative = true;
    multiplication->rightAssociative = true;
    multiplication->commutative = true;
    multiplication->symbol = '*';
    multiplication->arity = 2;
	multiplication->precedence = 2;
   	if (!initOperationImplementations(multiplication, multiply)) {
		free(multiplication);
		goto error;
	}

    if (!registerOperation(multiplication)) return 0;
    
    
    exponentiation = malloc(sizeof(Operation));
    if (exponentiation == NULL) goto error;

    exponentiation->leftAssociative = false;
	exponentiation->rightAssociative = true;
    exponentiation->commutative = false;
    exponentiation->symbol = '^';
    exponentiation->arity = 2;
	exponentiation->precedence = 3;
	if (!initOperationImplementations(exponentiation, exponent)) {
		free(exponentiation);
		goto error;
	}
    
    if (!registerOperation(exponentiation)) return 0;
    
    
    
    division = malloc(sizeof(Operation));
    if (division == NULL) goto error;

    division->leftAssociative = true;
	division->rightAssociative = false;
    division->commutative = false;
    division->symbol = '/';
    division->arity = 2;
	division->precedence = 2;
    if (!initOperationImplementations(division, divide)) {
		free(division);
		goto error;
	}

    if (!registerOperation(division)) return 0;
    
    
    subtraction = malloc(sizeof(Operation));
    if (subtraction == NULL) goto error;
    
    subtraction->leftAssociative = true;
	subtraction->rightAssociative = false;
    subtraction->commutative = false;
    subtraction->symbol = '-';
    subtraction->arity = 2;
	subtraction->precedence = 1;
	if (!initOperationImplementations(subtraction, subtract)) {
		free(subtraction);
		goto error;
	}
    
    if (!registerOperation(subtraction)) return 0;
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        return 0;
}
