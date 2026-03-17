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
int bindSymbol(Environment *env, SymbolType type, char *identifier, void *symbol);
Symbol* searchEnvironment(Environment *env, char *identifier);

#endif