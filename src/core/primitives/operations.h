#pragma once

#include <stdint.h>

#include "types.h"


Operation *createOperation(const char symbol, associativity a, bool c, uint32_t operands);


void freeOperation(Operation *op);


int initPrimitiveOperations();
