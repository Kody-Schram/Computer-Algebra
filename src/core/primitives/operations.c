#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "operations.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"


static Function *createBinOpFunc(BuiltinResult (*builtin) (unsigned int nArgs, Expression **exprs)) {
    Function *op = calloc(1, sizeof(Function));
    if (op == NULL) return NULL;
    op->nParameters = 2;
    op->type = BUILTIN;
    op->builtin = builtin;

    return op;
}


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
    addition->definitions = malloc(sizeof(Function *));
    if (addition->definitions == NULL) {
        free(addition);
        goto error;
    }
    
    addition->definitions[0] = createBinOpFunc(add);
    addition->nDefs = 1;
    
    if (!registerOperation(addition)) return 0;
    
    
    multiplication = malloc(sizeof(Operation));
    if (multiplication == NULL) goto error;

    multiplication->leftAssociative = true;
    multiplication->rightAssociative = true;
    multiplication->commutative = true;
    multiplication->symbol = '*';
    multiplication->arity = 2;
    multiplication->definitions = malloc(sizeof(Function *));
    if (multiplication->definitions == NULL) {
        free(multiplication);
        goto error;
    }
    
    multiplication->definitions[0] = createBinOpFunc(multiply);
    multiplication->nDefs = 1;
    
    if (!registerOperation(multiplication)) return 0;
    
    
    exponentiation = malloc(sizeof(Operation));
    if (exponentiation == NULL) goto error;

    exponentiation->leftAssociative = false;
	exponentiation->rightAssociative = true;
    exponentiation->commutative = false;
    exponentiation->symbol = '^';
    exponentiation->arity = 2;
    exponentiation->definitions = malloc(sizeof(Function *));
    if (exponentiation->definitions == NULL) {
        free(exponentiation);
        goto error;
    }
    
    exponentiation->definitions[0] = createBinOpFunc(exponent);
    exponentiation->nDefs = 1;
    
    if (!registerOperation(exponentiation)) return 0;
    
    
    
    division = malloc(sizeof(Operation));
    if (division == NULL) goto error;

    division->leftAssociative = true;
	division->rightAssociative = false;
    division->commutative = false;
    division->symbol = '/';
    division->arity = 2;
    division->definitions = malloc(sizeof(Function *));
    if (division->definitions == NULL) {
        free(division);
        goto error;
    }
    
    division->definitions[0] = createBinOpFunc(divide);
    division->nDefs = 1;
    
    if (!registerOperation(division)) return 0;
    
    
    subtraction = malloc(sizeof(Operation));
    if (subtraction == NULL) goto error;
    
    subtraction->leftAssociative = true;
	subtraction->rightAssociative = false;
    subtraction->commutative = false;
    subtraction->symbol = '-';
    subtraction->arity = 2;
    subtraction->definitions = malloc(sizeof(Function *));
    if (subtraction->definitions == NULL) {
        free(subtraction);
        goto error;
    }

    subtraction->definitions[0] = createBinOpFunc(subtract);
    subtraction->nDefs = 1;
    
    if (!registerOperation(subtraction)) return 0;
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        return 0;
}
