#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "axioms.h"
#include "core/utils/context/context.h"
#include "core/utils/context/environment.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


static Function *createOpFunc(BuiltinResult *(*builtin) (Expression **args, int nArgs)) {
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


BuiltinResult *add(Expression **args, int nArgs) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    if (nArgs != 2) return result;
    
    // If not both numbers, return the addition expression again
    if (args[0]->type != EXPRESSION_INTEGER && args[0]->type != EXPRESSION_DOUBLE && 
        args[1]->type != EXPRESSION_INTEGER && args[1]->type != EXPRESSION_DOUBLE) return nullptr;
    
    if (args[0]->type == EXPRESSION_INTEGER && args[1]->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = args[0]->integer + args[1]->integer;
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double a = (args[0]->type == EXPRESSION_INTEGER) ? (double) args[0]->integer : args[0]->value;
    double b = (args[1]->type == EXPRESSION_INTEGER) ? (double) args[1]->integer : args[1]->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = a + b;
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


BuiltinResult *multiply(Expression **args, int nArgs) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling multiplication builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    if (nArgs != 2) return result;
    
    // If not both numbers, return the addition expression again
    if (args[0]->type != EXPRESSION_INTEGER && args[0]->type != EXPRESSION_DOUBLE && 
        args[1]->type != EXPRESSION_INTEGER && args[1]->type != EXPRESSION_DOUBLE) return nullptr;
    
    if (args[0]->type == EXPRESSION_INTEGER && args[1]->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = args[0]->integer * args[1]->integer;
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double a = (args[0]->type == EXPRESSION_INTEGER) ? (double) args[0]->integer : args[0]->value;
    double b = (args[1]->type == EXPRESSION_INTEGER) ? (double) args[1]->integer : args[1]->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = a * b;
    
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


BuiltinResult *exponent(Expression **args, int nArgs) {
    BuiltinResult *result = malloc(sizeof(BuiltinResult));
    if (result == nullptr) {
        perror("Error calling addition builtin");
        return nullptr;
    }
    result->type = BUILTIN_ERROR;
    result->output = nullptr;
    
    if (nArgs != 2) return result;
    
    // If not both numbers, return the addition expression again
    if (args[0]->type != EXPRESSION_INTEGER && args[0]->type != EXPRESSION_DOUBLE && 
        args[1]->type != EXPRESSION_INTEGER && args[1]->type != EXPRESSION_DOUBLE) return nullptr;
    
    if (args[0]->type == EXPRESSION_INTEGER && args[1]->type == EXPRESSION_INTEGER) {
        Expression *output = dummyExpression(EXPRESSION_INTEGER);
        if (output == nullptr) return result;
        
        output->integer = _powi(args[0]->integer, args[1]->integer);
        
        result->type = BUILTIN_SUCCESS;
        result->output = output;
        return result;
    }
    
    double a = (args[0]->type == EXPRESSION_INTEGER) ? (double) args[0]->integer : args[0]->value;
    double b = (args[1]->type == EXPRESSION_INTEGER) ? (double) args[1]->integer : args[1]->value;
    
    Expression *output = dummyExpression(EXPRESSION_DOUBLE);
    if (output == nullptr) return result;
    output->value = powf(a, b);
    
    result->type = BUILTIN_SUCCESS;
    result->output = output;
    return result;
}


int initAxioms() {
    Operation *addition = nullptr;
    Operation *multiplication = nullptr;
    
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
    
    return 1;
    
    error:
        perror("Error initializing axioms");
        free(addition);
        free(multiplication);
        
        return 0;
}