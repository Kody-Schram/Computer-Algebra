#include <stdlib.h>

#include "execution_utils.h"
#include "core/primitives/types.h" 


BuiltinResult callImplementations(
		uint32_t nImplementations, BuiltinImplementation *implementations,
		uint32_t nArgs, Expression **exprs
) {
	if (nImplementations == 0) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output=NULL };

	BuiltinResult result;
	for (uint32_t i = 0; i < nImplementations; i ++) {
		result = implementations[i](nArgs, exprs);
		if (result.type == BUILTIN_NEUTRAL) continue;
		break;
	}
	
	return result;
}

