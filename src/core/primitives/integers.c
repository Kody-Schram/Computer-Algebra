#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include "integers.h"
#include "core/context.h"
#include "core/context/registry.h"
#include "core/utils/expr_utils.h"


BuiltinResult add_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	if (out == NULL) return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};

	long long value = (*(long long*) a->data) + (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	if (out->data == NULL) {
		free(out);
		return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};
	}

	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


BuiltinResult sub_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	if (out == NULL) return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};

	long long value = (*(long long*) a->data) - (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	if (out->data == NULL) {
		free(out);
		return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};
	}

	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


BuiltinResult mult_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	if (out == NULL) return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};

	long long value = (*(long long*) a->data) * (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	if (out->data == NULL) {
		free(out);
		return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};
	}

	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


BuiltinResult div_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	if (*(long long*) b->data == 0) {
		printf("Division by zero.\n");
		return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = NULL};
	}

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	if (out == NULL) return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};

	long long value = (*(long long*) a->data) / (*(long long*) b->data);
	out->data = malloc(sizeof(long long));
	if (out->data == NULL) {
		free(out);
		return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};
	}

	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


static inline long long _powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


BuiltinResult exp_int(Context const *ctx, uint32_t nArgs, Expression **operands) {
    Expression *a = operands[0];
    Expression *b = operands[1];

	if (a->objectId != INTEGER_ID || b->objectId != INTEGER_ID) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output = NULL};

	Expression *out = dummyExpression(EXPRESSION_OBJECT);
	if (out == NULL) return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};

	long long value = _powi(*(long long*) a->data, *(long long*)b->data); 
	out->data = malloc(sizeof(long long));
	if (out->data == NULL) {
		free(out);
		return (BuiltinResult) {.type = BUILTIN_ERROR, .output = NULL};
	}
	
	*(long long*)out->data = (long long) value;

	return (BuiltinResult) {.type = BUILTIN_SUCCESS, .output = out};
}


void cleanup_int(void *data) {
	free(data);
}


int32_t compare_int(void const *a, void const *b) {
	int64_t ia = *(int64_t const *) a;
	int64_t ib = *(int64_t const *) b;

	if (ia > ib) return 1;
	if (ia < ib) return -1;
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

	if (!createObject(&integer, LIB_CORE_8, cleanup_int, compare_int, copy_int)) return false;
	if (!registerObject(registry, integer, INTEGER_ID)) return false;

	if (!addOperationImplementation(registry, '+', add_int)) return false;
	if (!addOperationImplementation(registry, '-', sub_int)) return false;
	if (!addOperationImplementation(registry, '*', mult_int)) return false;
	if (!addOperationImplementation(registry, '/', div_int)) return false;
	if (!addOperationImplementation(registry, '^', exp_int)) return false;

	return true;
}
