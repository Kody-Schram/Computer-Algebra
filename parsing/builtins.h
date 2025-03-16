#ifndef BUILTINS_H
#define BUILTINS_H

#include "tables/functions.h"

// Function builtins
extern const char *builtin_identifiers[];
extern const int nBuiltins;

extern const Function builtins[];

typedef struct Constant {
    char *identifier;
    double value;
} Constant;

extern const Constant constants[];
extern const int nConstants;

#endif