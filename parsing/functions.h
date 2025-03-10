#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "parserTypes.h"

typedef struct Function {
    char *identifer;
    char **parameters;
    int nParameters;

    ASTNode *definition;
} Function;

typedef struct FunctionTable {
    Function *table[];
} FunctionTable;

#endif