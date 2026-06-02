#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include "integers.h"
#include "core/context.h"
#include "core/context/registry.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"


BuiltinResult add_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	out->integer = a->integer + b->integer;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


BuiltinResult mult_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
	printf("int mult\n");
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	out->integer = a->integer * b->integer;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
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


bool initIntegers(Registry *registry) {
	if (!addOperationImplementation(registry, '+', add_int)) return false;
	if (!addOperationImplementation(registry, '*', mult_int)) return false;

	return true;
}
