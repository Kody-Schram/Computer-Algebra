#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "axioms.h"
#include "core/utils/context/context.h"
#include "core/utils/context/environment.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


static Function *createOpFunc(BuiltinResult *(*builtin) (int nArgs, Expression **exprs)) {
    Function *op = malloc(sizeof(Function));
    if (op == nullptr) return nullptr;
    op->type = BUILTIN;
    op->parameters = 2;
    op->builtin = builtin;
    op->env = createEnvironment(ENV_LIST);
    
    Expression *a = dummyExpression(EXPRESSION_DOUBLE);
    Expression *b = dummyExpression(EXPRESSION_DOUBLE);
    if (a == nullptr || b == nullptr) return nullptr;
    
    a->value = 0;
    b->value = 0;
    if (!bindComponent(op->env, COMP_VARIABLE, "a", a) || !bindComponent(op->env, COMP_VARIABLE, "b", b)) return nullptr;
    
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


int initAxioms() {
    Operation *addition = nullptr;
    Operation *multiplication = nullptr;
    Operation *exponentiation = nullptr;
    
    addition = malloc(sizeof(Operation));
    if (addition == nullptr) goto error;
    
    addition->associative = 1;
    addition->commutative = 1;
    addition->symbol = '+';
    addition->type = OP_AXIOMATIC;
    addition->definition = createOpFunc(add);
    
    Debug(0, "Binding addition operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "+", addition)) goto error;
    
    multiplication = malloc(sizeof(Operation));
    if (multiplication == nullptr) goto error;
    
    multiplication->associative = 1;
    multiplication->commutative = 1;
    multiplication->symbol = '*';
    multiplication->type = OP_AXIOMATIC;
    multiplication->definition = createOpFunc(multiply);
    
    Debug(0, "Binding multiplication operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "*", multiplication)) goto error;
    
    exponentiation = malloc(sizeof(Operation));
    if (exponentiation == nullptr) goto error;
    
    exponentiation->associative = 0;
    exponentiation->commutative = 0;
    exponentiation->symbol = '^';
    exponentiation->type = OP_AXIOMATIC;
    exponentiation->definition = createOpFunc(exponent);
    
    Debug(0, "Binding multiplication operation\n");
    if (!bindComponent(GLOBALCONTEXT->env, COMP_OPERATION, "^", exponentiation)) goto error;
    
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        free(addition);
        free(multiplication);
        free(exponentiation);
        
        return 0;
}