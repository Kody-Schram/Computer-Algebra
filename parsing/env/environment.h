#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

typedef struct ASTNode ASTNode;

typedef enum {
    BUILTIN,
    DEFINED
} FunctionType;

typedef struct {
    char **parameters;
    int nParameters;
    FunctionType type;

    union {
        ASTNode *definition;
        double (*builtin) (double);
    };
} Function;


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

#endif