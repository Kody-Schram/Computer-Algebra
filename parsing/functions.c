#include <stdio.h>
#include <stdlib.h>


#include "functions.h"
#include "builtins.h"

const int DEFAULT_TABLE_SIZE = 10;
const int DEFAULT_INCREASE_SIZE = 5;

FunctionTable functionTable;

int addFunction(Function* function) {
    functionTable.n ++;

    // Increases table size if needed
    if (functionTable.n == functionTable.size) {
        functionTable.size += DEFAULT_INCREASE_SIZE;
        functionTable.table = realloc(functionTable.table, functionTable.size);
        if (functionTable.table == NULL) {
            printf("Error reallocating function table.");
            return 0;
        }
    }

    functionTable.table[functionTable.n] = function;

    return 1;
}

int addBuiltins() {
    for (int i = 0; i < nBuiltins; i ++) {
        if (!addFunction((Function*) &builtins[i])) {
            return 0; // Error handling
        }
    }

    return 1;
}

int searchTable(Function* function) {
    for (int i = 0; i < functionTable.n; i ++) {
        if (functionTable.table[i]->identifier == function->identifier) {
            return 1;
        }
    }

    return 0;
}

int initFunctionTable() {
    functionTable.table = (Function **) malloc(DEFAULT_TABLE_SIZE * sizeof(Function *));
    if (functionTable.table == NULL) {
        printf("Error allocating for function table.");
        return 0;
    }

    functionTable.size = 0;
    functionTable.n = 0;

    if (!addBuiltins()) return 0;

    return 1;
}