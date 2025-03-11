#include <stdlib.h>

#include "functions.h"

FunctionTable functionTable;

void addFunction(Function* function) {

}

void builtins() {
    //Function sin = {"sin", "x"};
}

void initFunctionTable() {
    functionTable = *(FunctionTable*) malloc(sizeof(int) + 8 * sizeof(Function*));
}