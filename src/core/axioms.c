#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "axioms.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


static Function *createOpFunc(BuiltinResult *(*builtin) (int nArgs, Expression **exprs)) {
    Function *op = calloc(1, sizeof(Function));
    if (op == nullptr) return nullptr;
    op->parameters = malloc(2 * sizeof(char *));
    op->type = BUILTIN;
    op->builtin = builtin;
    
    return op;
}


BuiltinResult *add(int nArgs, Expression **operands) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    Expression *a = operands[0];
    Expression *b = operands[1];
    
    
    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE && 
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;
    
    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = a->integer + b->integer;
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = av + bv;
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


BuiltinResult *multiply(int nArgs, Expression **operands) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    Expression *a = operands[0];
    Expression *b = operands[1];
    
    
    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE && 
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;
    
    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = a->integer * b->integer;
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = av * bv;
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


long long _powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


BuiltinResult *exponent(int nArgs, Expression **operands) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    Expression *a = operands[0];
    Expression *b = operands[1];
    
    
    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE && 
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;
    
    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = _powi(a->integer, b->integer);
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = powf(av, bv);
        
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


BuiltinResult *subtract(int nArgs, Expression **operands) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    Expression *a = operands[0];
    Expression *b = operands[1];
    
    
    // If not both numbers, return the addition expression again
    if (a->type != EXPRESSION_INTEGER && a->type != EXPRESSION_DOUBLE && 
        b->type != EXPRESSION_INTEGER && b->type != EXPRESSION_DOUBLE) return result;
    
    if (a->type == EXPRESSION_INTEGER && b->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = a->integer - b->integer;
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = av - bv;
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


BuiltinResult *divide(int nArgs, Expression **operands) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    if (GLOBALCONTEXT->config->PRESERVE_FRACS) {
        result->type = BUILTIN_SUCCESS;
        return result;
    }
    
    Expression *a = operands[0];
    Expression *b = operands[1];
    
    double av = (a->type == EXPRESSION_INTEGER) ? (double) a->integer : a->value;
    double bv = (b->type == EXPRESSION_INTEGER) ? (double) b->integer : b->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = av / bv;
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


int initAxioms() {
    Operation *addition = nullptr;
    Operation *multiplication = nullptr;
    Operation *exponentiation = nullptr;
    
    Operation *subtraction = nullptr;
    Operation *division = nullptr;
    
    addition = malloc(sizeof(Operation));
    if (addition == nullptr) goto error;
    
    addition->associative = true;
    addition->commutative = true;
    addition->symbol = '+';
    addition->type = OP_AXIOMATIC;
    addition->definition = createOpFunc(add);
    
    Debug(0, "Binding addition operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "+", addition)) goto error;
    
    multiplication = malloc(sizeof(Operation));
    if (multiplication == nullptr) goto error;
    
    multiplication->associative = true;
    multiplication->commutative = true;
    multiplication->symbol = '*';
    multiplication->type = OP_AXIOMATIC;
    multiplication->definition = createOpFunc(multiply);
    
    Debug(0, "Binding multiplication operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "*", multiplication)) goto error;
    
    exponentiation = malloc(sizeof(Operation));
    if (exponentiation == nullptr) goto error;
    
    exponentiation->associative = false;
    exponentiation->commutative = false;
    exponentiation->symbol = '^';
    exponentiation->type = OP_AXIOMATIC;
    exponentiation->definition = createOpFunc(exponent);
    
    Debug(0, "Binding exponentiation operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "^", exponentiation)) goto error;
    
    
    // =========================================================
    // These will hopfully later be replaced with using inverses
    // =========================================================
    
    subtraction = malloc(sizeof(Operation));
    if (subtraction == nullptr) goto error;
    
    subtraction->associative = true;
    subtraction->commutative = false;
    subtraction->symbol = '-';
    subtraction->type = OP_AXIOMATIC;
    subtraction->definition = createOpFunc(subtract);
    
    Debug(0, "Binding subtraction operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "-", subtraction)) goto error;
    
    division = malloc(sizeof(Operation));
    if (division == nullptr) goto error;
    
    division->associative = false;
    division->commutative = false;
    division->symbol = '/';
    division->type = OP_AXIOMATIC;
    division->definition = createOpFunc(divide);
    
    Debug(0, "Binding division operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "/", division)) goto error;
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        free(addition);
        free(multiplication);
        free(exponentiation);
        
        free(subtraction);
        free(division);
        
        return 0;
}