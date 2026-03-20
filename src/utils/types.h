#ifndef TYPES_H
#define TYPES_H

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif

// Forward declaring types
typedef enum NodeType NodeType;
typedef struct ASTNode ASTNode;

typedef struct FunctionCall FunctionCall;
typedef enum FunctionType FunctionType;
typedef struct Function Function;

typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef struct Environment Environment;

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
        void *definition;
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
    char **parameters;
    int nParameters;
    FunctionType type;

    union {
        ASTNode *definition;
        double (*builtin) (double);
        ASTNode* (*transform) (ASTNode **args, int nArgs);
    };
};


// Environment related definitions
enum ComponentType{
    VARIABLE,
    FUNCTION
};

struct Component {
    ComponentType type;
    char *identifier;

    union {
        Function *function;
        double value;
    };

};

struct Environment {
    int entries;
    int size;
    Component *components;
};

#endif