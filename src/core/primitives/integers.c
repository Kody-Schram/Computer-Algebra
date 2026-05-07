#include <stdint.h>
#include <stddef.h>

#include "integers.h"
#include "core/primitives/types.h"

BuiltinResult add_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

	uint32_t intId = getObjectId(ctx->registry, "integer");

	if (a->objectId != intId || b->objectId != intId) return result;

	return result;
}

bool initIntegers() {
	
}
