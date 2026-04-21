#ifndef TYPES_H
#define TYPES_H

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif

#include <stdio.h>

// Forward declare from environment.h
typedef struct Environment Environment;

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
typedef enum BuiltinResultType BuiltinResultType;
typedef struct BuiltinResult BuiltinResult;
typedef struct Function Function;


// Expression related definitions
enum ExpressionType {
    EXPRESSION_VARIABLE,
    EXPRESSION_INTEGER,
    EXPRESSION_DOUBLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_OPERATOR
};


enum OperationType {
    OP_AXIOMATIC,
    OP_ABSTRACT
};


struct Operation {
    bool associative;
    bool commutative;
    char symbol;
    OperationType type;
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
};

enum BuiltinResultType {
    BUILTIN_SUCCESS,
    BUILTIN_ERROR
};

struct BuiltinResult {
    BuiltinResultType type;
    Expression *output;
};


struct Function {
    FunctionType type;
    char **parameters;
    int nParameters;

    union {
        Expression *definition;
        BuiltinResult *(*builtin) (int nArgs, Expression ** exprs);
    };
};



Expression *dummyExpression(ExpressionType type);


Expression *deepCopyExpression(const Expression *expr);


FILE *printExpression(const Expression *expr);


void freeExpression(Expression *expr);


char *expressionToString(const Expression *expr);

#endif