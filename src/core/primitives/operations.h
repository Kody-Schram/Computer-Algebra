#pragma once

#include <stdint.h>

#include "types.h"


Operation *createOperation(const char symbol, bool lA, bool rA, bool c, uint32_t operands);


void freeOperation(Operation *op);


int registerImplementation(
		Operation *op,
		BuiltinResult (*function) (uint32_t nArgs, Expression **exprs)
);


int initPrimitiveOperations();
