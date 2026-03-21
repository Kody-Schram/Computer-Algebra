#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "utils/types.h"
typedef struct Function Function;
typedef struct ASTNode ASTNode;

typedef enum ComponentType ComponentType;
typedef struct ComponentData ComponentData;
typedef struct Component Component;
typedef struct Environment Environment;


// Environment related definitions
enum ComponentType {
    VARIABLE,
    FUNCTION
};

struct Component {
    ComponentType type;
    char *identifier;

    union {
        Function *func;
        ASTNode *value;
    };
};

struct Environment {
    int entries;
    int size;
    Component *components;
};



Environment *createEnvironment();


int bindComponent(Environment *env, ComponentType type, char *identifier, void *data);


Component* searchEnvironment(Environment *env, char *identifier);


void printEnvironment(Environment *env);

#endif