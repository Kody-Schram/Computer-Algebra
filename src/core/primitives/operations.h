#pragma once

#include <stdint.h>

#include "types.h"


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t operands);


void freeOperation(Operation *op);


bool initPrimitiveOperations();
