#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

typedef struct ASTNode ASTNode;

typedef enum {
    BUILTIN,
    DEFINED,
    TRANSFORM
} FunctionType;

typedef struct {
    char **parameters;
    int nParameters;
    FunctionType type;

    union {
        ASTNode *definition;
        double (*builtin) (double);
        ASTNode* (*transform) (ASTNode **args, int nArgs);
    };
} Function;


typedef struct {
    char *identifier;
    ASTNode **parameters;
    int nParams;
} FunctionCall;


typedef enum {
    VARIABLE,
    FUNCTION
} ComponentType;


typedef struct {
    ComponentType type;
    char *identifier;

    union {
        Function *function;
        double value;
    };

} Component;


typedef struct {
    int entries;
    int size;
    Component *components;
} Environment;


Environment *createEnvironment();


int bindComponent(Environment *env, ComponentType type, char *identifier, void *component);


Component* searchEnvironment(Environment *env, char *identifier);


void printEnvironment(Environment *env);

#endif