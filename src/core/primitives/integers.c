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
	long long value = (*(long long*) a->data) + (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


BuiltinResult mult_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	long long value = (*(long long*) a->data) * (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


void cleanup_int(void *data) {
	free(data);
}


int32_t compare_int(void const *a, void const *b) {
	int64_t ia = *(int64_t const *) a;
	int64_t ib = *(int64_t const *) b;

	if (a > b) return 1;
	if (a < b) return -1;
	return 0;
}

void *copy_int(void const *src) {
	long long *new = malloc(sizeof(long long));
	if (new == NULL) return NULL;

	*new = *(long long*) src;
	return new;
}


bool initIntegers(Registry *registry) {
	Object integer;

	if (!createObject(&integer, INTEGER_ID, LIB_CORE_8, cleanup_int, compare_int, copy_int)) return false;
	if (!registerObject(registry, integer)) return false;

	if (!addOperationImplementation(registry, '+', add_int)) return false;
	if (!addOperationImplementation(registry, '*', mult_int)) return false;

	return true;
}
