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
typedef enum NodeType NodeType;
typedef enum OperationType OperationType;
typedef enum NumberType NumberType;
typedef struct Number Number;
typedef struct ASTNode ASTNode;

typedef struct FunctionCall FunctionCall;
typedef enum FunctionType FunctionType;
typedef struct Function Function;


// ASTNode related definitions
enum NodeType {
    NODE_INTEGER,
    NODE_DOUBLE,
    NODE_OPERATOR,
    NODE_VARIABLE,
    NODE_FUNC_CALL,
    NODE_ASSIGN_VAR,
    NODE_ASSIGN_FUNC
};


enum OperationType {
    OP_ADDITION,
    OP_SUBTRACTION,
    OP_MULTIPLICATION,
    OP_DIVISION,
    OP_EXPONTENTIATION
};


struct ASTNode {
    NodeType type;
    // Look into packing an int in here to support scientific notation

    union {
        OperationType op;
        char *identifier;
        double value; // Uses double over long double as most math.h funcs cast to double, and would leave 12 bytes of wasted space
        long long integer;
        Function *func;
        FunctionCall *call;
    };

    struct ASTNode *left;
    struct ASTNode *right;
};


// Function related definitions
struct FunctionCall {
    char *identifier;
    ASTNode **parameters;
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
        ASTNode *definition;
        double (*builtin) (double);
        ASTNode *(*transform) (ASTNode **args, int nArgs);
    };
};



ASTNode *dummyASTNode(NodeType type);


ASTNode *deepCopyAST(ASTNode *ast);


FILE *printAST(ASTNode *root);


void freeAST(ASTNode *ast);


char *astToString(ASTNode *ast);

#endif