#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../parserTypes.h"

typedef enum {
    BUILTIN,
    DEFINED
} FunctionType;

typedef struct {
    char **parameters;
    int nParameters;
    FunctionType type;

    union 
    {
        ASTNode *definition;
        double (*builtin) (double);
    };
} Function;


typedef enum {
    VARAIBLE,
    FUNCTION
} SymbolType;


typedef struct {
    SymbolType type;
    char *identifier;

    union {
        Function *function;
        double value;
    };

} Symbol;


typedef struct {
    int entries;
    int size;
    Symbol *symbols;
} Environment;


Environment *createEnvironment();
int bindSymbol(Environment env, Symbol *symbol);

#endif