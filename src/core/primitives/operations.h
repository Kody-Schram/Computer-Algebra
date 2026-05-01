#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "types.h"


int registerOperation(Operation *op);


int registerImplementation(Operation *op, BuiltinResult (*function) (unsigned int nArgs, Expression **exprs));


int initPrimitiveOperations();

#endif
