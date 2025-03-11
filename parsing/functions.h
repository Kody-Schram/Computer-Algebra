#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "parserTypes.h"

typedef enum FunctionType {
    BUILTIN,
    DEFINED
} FunctionType;

typedef struct Function {
    char *identifier;
    char **parameters;
    int nParameters;

    FunctionType type;

    union 
    {
        ASTNode *definition;
        double (*builtin) (double);
    };
} Function;

typedef struct FunctionTable {
    int size;
    Function *table[];
} FunctionTable;

#endif