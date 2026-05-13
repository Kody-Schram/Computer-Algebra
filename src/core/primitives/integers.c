#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "integers.h"
#include "core/primitives/types.h"


BuiltinResult add_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    BuiltinResult result = {.type = BUILTIN_NEUTRAL, NULL};

    Expression *a = operands[0];
    Expression *b = operands[1];

 

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return result;

	return result;
}




void cleanup_int(void *integer) {
	free(integer);
}


int32_t compare_int(void const *a, void const *b) {
	int64_t ia = *(int64_t const *) a;
	int64_t ib = *(int64_t const *) b;

	if (a > b) return 1;
	if (a < b) return -1;
	return 0;
}
