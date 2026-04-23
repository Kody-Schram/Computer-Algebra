#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "operations.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/type_utils.h"


static Function *createBinOpFunc(BuiltinResult (*builtin) (int nArgs, Expression **exprs)) {
    Function *op = calloc(1, sizeof(Function));
    if (op == NULL) return NULL;
    op->nParameters = 2;
    op->type = BUILTIN;
    op->builtin = builtin;

    return op;
}


BuiltinResult add(int nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_ERROR, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE &&
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) return result;

        output->integer = a->integer + b->integer;

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) return result;
    output->value = av + bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
}


BuiltinResult multiply(int nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_ERROR, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE &&
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) return result;

        output->integer = a->integer * b->integer;

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) return result;
    output->value = av * bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
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


BuiltinResult exponent(int nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_ERROR, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE &&
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;

    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == NULL) return result;

        output->integer = _powi(a->integer, b->integer);

        result.type = BUILTIN_SUCCESS;
        result.output = output;
        return result;
    }

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) return result;
    output->value = powf(av, bv);

    result.type = BUILTIN_SUCCESS;
    result.output = output;
    return result;
}


BuiltinResult divide(int nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_ERROR, NULL};

    if (GLOBALCONTEXT->config->PRESERVE_FRACS) {
        result.type = BUILTIN_SUCCESS;
        return result;
    }

    Expression *a = operands[0];
    Expression *b = operands[1];

    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;

    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == NULL) return result;
    output->value = av / bv;

    result.type = BUILTIN_SUCCESS;
    result.output = output;
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
    
    
    addition = malloc(sizeof(Operation));
    if (addition == NULL) goto error;

    addition->associative = true;
    addition->commutative = true;
    addition->symbol = '+';
    addition->definition = createBinOpFunc(add);
    
    if (!registerOperation(addition)) return 0;
    
    
    multiplication = malloc(sizeof(Operation));
    if (multiplication == NULL) goto error;

    multiplication->associative = true;
    multiplication->commutative = true;
    multiplication->symbol = '*';
    multiplication->definition = createBinOpFunc(multiply);
    
    if (!registerOperation(multiplication)) return 0;
    
    
    exponentiation = malloc(sizeof(Operation));
    if (exponentiation == NULL) goto error;

    exponentiation->associative = false;
    exponentiation->commutative = false;
    exponentiation->symbol = '^';
    exponentiation->definition = createBinOpFunc(exponent);
    
    if (!registerOperation(exponentiation)) return 0;
    
    
    division = malloc(sizeof(Operation));
    if (division == NULL) goto error;

    division->associative = false;
    division->commutative = false;
    division->symbol = '/';
    division->definition = createBinOpFunc(divide);
    
    if (!registerOperation(division)) return 0;
    
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        free(addition);
        free(multiplication);
        free(exponentiation);
        
        free(division);

        return 0;
}