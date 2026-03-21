#ifndef TYPES_H
#define TYPES_H

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif

#include "utils/context/environment.h"
typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef struct Environment Environment;

// Forward declaring types
typedef enum NodeType NodeType;
typedef struct ASTNode ASTNode;

typedef struct FunctionCall FunctionCall;
typedef enum FunctionType FunctionType;
typedef struct Function Function;


// ASTNode related definitions
enum NodeType {
    NODE_NUMBER,
    NODE_OPERATOR,
    NODE_VARIABLE,
    NODE_FUNC_CALL,
    NODE_ASSIGN_VAR,
    NODE_ASSIGN_FUNC
};

struct ASTNode {
    NodeType type;
    union {
        char *identifier;
        double value;
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


#endif