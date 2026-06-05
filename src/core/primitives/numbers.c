#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "numbers.h"
#include "core/common.h"
#include "core/context.h"
#include "core/utils/expr_utils.h"


bool defaultNumberParser(char const *input, ObjectValue *value, uint32_t *flags) {
	char *end;
	double number = strtod(input, &end);

	if (end == input) return false;
	
	if (number == (int64_t) number) {
		value->integer = number;
		SET_INLINE_INTEGER_TRUE(*flags);

		return true;
	}

	value->floating = number;

	return true;
}


void cleanup_number(ObjectValue value, uint32_t flags) {
	if (!GMP_NUMBER(flags) || INLINE_INTEGER(flags)) return;
}


int32_t compare_number(ObjectValue const a, uint32_t aFlags, ObjectValue const b, uint32_t bFlags) {

	if (GMP_NUMBER(aFlags) || GMP_NUMBER(bFlags)) return 0;
	if (INLINE_INTEGER(aFlags) && INLINE_INTEGER(bFlags)) return a.integer - b.integer;

	if (!INLINE_INTEGER(aFlags)) return a.floating- b.integer;
	else if (!INLINE_INTEGER(bFlags)) return a.integer - b.floating;
	return a.floating - b.floating;
}


bool copy_number(ObjectValue const src, ObjectValue *dest, uint32_t flags) {
	if (GMP_NUMBER(flags)) return 0;
	else if (INLINE_INTEGER(flags)) dest->integer = src.integer;
	else dest->floating = src.floating;

	return true;
}


char *print_number(ObjectValue const value, uint32_t flags) {
	if (GMP_NUMBER(flags)) return NULL;

    char *result = NULL;

	if (INLINE_INTEGER(flags)) {
		if (asprintf(&result, "%" PRId64, value.integer) < 0) return NULL; 
	} else {
		if (asprintf(&result, "%f", value.floating) < 0) return NULL; 
	}

	return result;
}


BuiltinResult add_number(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out) {
	Expression *a = operands[0];
	Expression *b = operands[1];

	if (a->objectId != NUMBER_ID || b->objectId != NUMBER_ID) {
		*out = NULL;
		return BUILTIN_NEUTRAL;
	}

	if (GMP_NUMBER(a->flags) || GMP_NUMBER(b->flags)) return BUILTIN_ERROR; // not implemented yet

	Expression *result = dummyExpression(EXPRESSION_OBJECT);
	result->objectId = NUMBER_ID;

	if (INLINE_INTEGER(a->flags) && INLINE_INTEGER(b->flags)) {
		result->value.integer = a->value.integer + b->value.integer;
		SET_INLINE_INTEGER_TRUE(result->flags);
	}
	else if (!INLINE_INTEGER(a->flags)) result->value.floating = a->value.floating + b->value.integer;
	else if (INLINE_INTEGER(b->flags)) result->value.floating = a->value.integer + b->value.floating;
	else result->value.floating = a->value.floating + b->value.floating;

	return BUILTIN_SUCCESS;
}


BuiltinResult sub_number(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out) {
	Expression *a = operands[0];
	Expression *b = operands[1];

	if (a->objectId != NUMBER_ID || b->objectId != NUMBER_ID) {
		*out = NULL;
		return BUILTIN_NEUTRAL;
	}

	if (GMP_NUMBER(a->flags) || GMP_NUMBER(b->flags)) return BUILTIN_ERROR; // not implemented yet

	Expression *result = dummyExpression(EXPRESSION_OBJECT);
	result->objectId = NUMBER_ID;

	if (INLINE_INTEGER(a->flags) && INLINE_INTEGER(b->flags)) {
		result->value.integer = a->value.integer - b->value.integer;
		SET_INLINE_INTEGER_TRUE(result->flags);
	}
	else if (!INLINE_INTEGER(a->flags)) result->value.floating = a->value.floating - b->value.integer;
	else if (INLINE_INTEGER(b->flags)) result->value.floating = a->value.integer - b->value.floating;
	else result->value.floating = a->value.floating - b->value.floating;

	return BUILTIN_SUCCESS;
}


BuiltinResult mult_number(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out) {
	Expression *a = operands[0];
	Expression *b = operands[1];

	if (a->objectId != NUMBER_ID || b->objectId != NUMBER_ID) {
		*out = NULL;
		return BUILTIN_NEUTRAL;
	}

	if (GMP_NUMBER(a->flags) || GMP_NUMBER(b->flags)) return BUILTIN_ERROR; // not implemented yet

	Expression *result = dummyExpression(EXPRESSION_OBJECT);
	result->objectId = NUMBER_ID;

	if (INLINE_INTEGER(a->flags) && INLINE_INTEGER(b->flags)) {
		result->value.integer = a->value.integer * b->value.integer;
		SET_INLINE_INTEGER_TRUE(result->flags);
	}
	else if (!INLINE_INTEGER(a->flags)) result->value.floating = a->value.floating * b->value.integer;
	else if (INLINE_INTEGER(b->flags)) result->value.floating = a->value.integer * b->value.floating;
	else result->value.floating = a->value.floating * b->value.floating;

	return BUILTIN_SUCCESS;
}


BuiltinResult div_number(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out) {
	Expression *a = operands[0];
	Expression *b = operands[1];

	if (a->objectId != NUMBER_ID || b->objectId != NUMBER_ID) {
		*out = NULL;
		return BUILTIN_NEUTRAL;
	}

	if (GMP_NUMBER(a->flags) || GMP_NUMBER(b->flags)) return BUILTIN_ERROR; // not implemented yet


	Expression *result = dummyExpression(EXPRESSION_OBJECT);
	result->objectId = NUMBER_ID;

	if (INLINE_INTEGER(a->flags) && INLINE_INTEGER(b->flags)) {
		if (b->value.integer == 0) goto div_by_zero;
		result->value.floating = ((double) a->value.integer) / ((double) b->value.integer);
		SET_INLINE_INTEGER_TRUE(result->flags);
	}
	else if (!INLINE_INTEGER(a->flags)) {
		if (b->value.integer == 0) goto div_by_zero;
		result->value.floating = a->value.floating / b->value.integer;
	}
	else if (INLINE_INTEGER(b->flags)) {
		if (b->value.floating == 0) goto div_by_zero;
		result->value.floating = a->value.integer / b->value.floating;
	}
	else {
		if (b->value.floating == 0) goto div_by_zero;
		result->value.floating = a->value.floating / b->value.floating;
	}

	return BUILTIN_SUCCESS;

	div_by_zero:
		printf("Division by zero.\n");
		*out = NULL;
		return BUILTIN_ERROR;
}


static inline int64_t _powi(int64_t a, int64_t e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


BuiltinResult exp_number(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out) {
	Expression *a = operands[0];
	Expression *b = operands[1];

	if (a->objectId != NUMBER_ID || b->objectId != NUMBER_ID) {
		*out = NULL;
		return BUILTIN_NEUTRAL;
	}

	if (GMP_NUMBER(a->flags) || GMP_NUMBER(b->flags)) return BUILTIN_ERROR; // not implemented yet

	Expression *result = dummyExpression(EXPRESSION_OBJECT);
	result->objectId = NUMBER_ID;

	if (INLINE_INTEGER(a->flags) && INLINE_INTEGER(b->flags)) result->value.integer = _powi(a->value.integer, b->value.integer); 
	else if (!INLINE_INTEGER(a->flags)) result->value.floating = powf(a->value.floating, b->value.integer); 
	else if (INLINE_INTEGER(b->flags)) result->value.floating = powf(a->value.integer, b->value.floating); 
	else result->value.floating = powf(a->value.floating, b->value.floating); 

	return BUILTIN_SUCCESS;
}


bool initNumbers(Registry *registry) {
	Object number;
	if (!createObject(&number, LIB_CORE_8, cleanup_number, compare_number, copy_number, print_number)) return true;
	if (!registerObject(registry, number, NUMBER_ID)) return false;

	if (!addOperationImplementation(registry, '+', add_number)) return false;
	if (!addOperationImplementation(registry, '-', sub_number)) return false;
	if (!addOperationImplementation(registry, '*', mult_number)) return false;
	if (!addOperationImplementation(registry, '/', div_number)) return false;
	if (!addOperationImplementation(registry, '^', exp_number)) return false;

	return true;
}
