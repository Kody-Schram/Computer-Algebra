#ifndef TYPES_H
#define TYPES_H

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif

#include <stdio.h>

// Forward declare from environment.h
typedef struct Environment Environment;
void freeEnvironment(Environment *env);

typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef struct Environment Environment;

// Forward declaring types
typedef enum OperationType OperationType;
typedef struct Operation Operation;
typedef enum ExpressionType ExpressionType;
typedef struct Expression Expression;

typedef struct FunctionCall FunctionCall;
typedef enum FunctionType FunctionType;
typedef struct Function Function;


// ASTNode related definitions
enum ExpressionType {
    EXPRESSION_VARIABLE,
    EXPRESSION_INTEGER,
    EXPRESSION_DOUBLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_OPERATOR
};

enum OperationType {
    AXIOMATIC,
    CUSTOM
};


struct Operation {
    bool associative;
    bool commutative;
    OperationType type;
    char symbol;
    Function *definition;
};

struct Expression {
    ExpressionType type;
    
    union {
        struct {
            const Operation *op;
            int arity; // Number of operands
            struct Expression **operands;
        };
        
        char *identifier;
        double value;
        long long integer;
        FunctionCall *call;
    };
};


// Function related definitions
struct FunctionCall {
    char *identifier;
    Expression **parameters;
    int nParams;
};


enum FunctionType {
    BUILTIN,
    DEFINED,
    TRANSFORM
};


struct Function {
    Environment *env;
    FunctionType type;

    union {
        Expression *definition;
        double (*builtin) (double);
        Expression *(*transform) (Expression **args, int nArgs);
    };
};



Expression *dummyExpression(ExpressionType type);


Expression *deepCopyExpression(Expression *expr);


FILE *printExpression(Expression *expr);


void freeExpression(Expression *expr);


char *expressionToString(Expression *expr);

#endif