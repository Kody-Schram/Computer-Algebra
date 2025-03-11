#include <stdlib.h>

#include "functions.h"
#include "builtins.h"

FunctionTable functionTable;

void addFunction(Function* function) {

}

void addBuiltins() {
    for (int i = 0; i < nBuiltins; i ++) {
        addFunction((Function*) &builtins[i]);
    }
}

void initFunctionTable() {
    functionTable = *(FunctionTable*) malloc(sizeof(int) + 8 * sizeof(Function*));
}