#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "types.h"


Operation *createOperation(const char symbol, bool lA, bool rA, bool c, unsigned int operands);


void freeOperation(Operation *op);


int registerImplementation(Operation *op, BuiltinResult (*function) (unsigned int nArgs, Expression **exprs));


int initPrimitiveOperations();

#endif
