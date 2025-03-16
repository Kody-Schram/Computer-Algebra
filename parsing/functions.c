#include <stdio.h>
#include <stdlib.h>


#include "functions.h"
#include "builtins.h"

const int DEFAULT_TABLE_SIZE = 10;
const int DEFAULT_INCREASE_SIZE = 5;

FunctionTable functionTable;

void printTable() {
    printf("<\n");
    for (int i = 0; i < functionTable.n; i ++) {
        if (functionTable.table[i] != NULL) {
        printf("identifier: %s,\n", functionTable.table[i]->identifier);
        }
    }
    printf(">\n");
}

int addFunction(Function* function) {
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
    functionTable.n ++;

    return 1;
}

int addBuiltins() {
    for (int i = 0; i < nBuiltins; i ++) {
        if (addFunction((Function*) &builtins[i]) == 0) {
            printf("Error adding function to table.\n");
            return 0; // Error handling
        }
    }

    return 1;
}

Function *searchTable(char *identifier) {
    for (int i = 0; i < functionTable.n; i ++) {
        if (functionTable.table[i]->identifier == identifier) {
            return functionTable.table[i];
        }
    }

    return NULL;
}

int initFunctionTable() {
    functionTable.table = (Function **) malloc(DEFAULT_TABLE_SIZE * sizeof(Function *));
    if (functionTable.table == NULL) {
        printf("Error allocating for function table.");
        return 0;
    }

    functionTable.size = DEFAULT_TABLE_SIZE;
    functionTable.n = 0;

    if (!addBuiltins()) return 0;
    
    return 1;
}