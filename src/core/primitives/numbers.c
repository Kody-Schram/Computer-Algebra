#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "numbers.h"
#include "core/common.h"
#include "core/context.h"
#include "core/utils/log.h"


bool defaultNumberParser(char const *input, ObjectValue *value, ExpressionMeta *meta) {
	Debug(0, "Creating number from '%s'\n", input);
	char *end;
	double number = strtod(input, &end);

	if (end == input) return false;
	
	if (number == (int64_t) number) {
		value->integer = number;
		SET_INLINE_INTEGER_TRUE((*meta).coreFlags);

		return true;
	}

	value->floating = number;

	return true;
}


void cleanup_number(ObjectData data) {
	if (!GMP_NUMBER(data.meta.coreFlags) || INLINE_INTEGER(data.meta.coreFlags)) return;
}


int32_t compare_number(ObjectData const a, ObjectData const b) {

	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return 0;
	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) return a.value.integer- b.value.integer;

	if (!INLINE_INTEGER(a.meta.coreFlags)) return a.value.floating - b.value.integer;
	else if (!INLINE_INTEGER(b.meta.coreFlags)) return a.value.integer - b.value.floating;
	return a.value.floating - b.value.floating;
}


bool copy_number(ObjectValue const src, ObjectValue *dest, ExpressionMeta meta, uint32_t flags) {
	if (GMP_NUMBER(flags)) return false;
	else if (INLINE_INTEGER(flags)) dest->integer = src.integer;
	else dest->floating = src.floating;

	return true;
}


char *print_number(ObjectData data) {
	if (GMP_NUMBER(data.meta.coreFlags)) return NULL;

    char *result = NULL;

	if (INLINE_INTEGER(data.meta.coreFlags)) {
		if (asprintf(&result, "%" PRId64, data.value.integer) < 0) return NULL; 
	} else {
		if (asprintf(&result, "%f", data.value.floating) < 0) return NULL; 
	}

	return result;
}


BuiltinResult add_number(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out) {
	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return BUILTIN_ERROR; // not implemented yet

	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) {
		(*out).value.integer = a.value.integer + b.value.integer;
		SET_INLINE_INTEGER_TRUE((*out).meta.coreFlags);
	}
	else if (!INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.floating + b.value.integer;
	else if (INLINE_INTEGER(a.meta.coreFlags) && !INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.integer + b.value.floating;
	else (*out).value.floating = a.value.floating + b.value.floating;

	*id_out = NUMBER_ID;

	return BUILTIN_SUCCESS;
}


BuiltinResult sub_number(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out) {
	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return BUILTIN_ERROR; // not implemented yet

	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) {
		(*out).value.integer = a.value.integer - b.value.integer;
		SET_INLINE_INTEGER_TRUE((*out).meta.coreFlags);
	}
	else if (!INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.floating - b.value.integer;
	else if (INLINE_INTEGER(a.meta.coreFlags) && !INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.integer - b.value.floating;
	else (*out).value.floating = a.value.floating - b.value.floating;

	*id_out = NUMBER_ID;

	return BUILTIN_SUCCESS;
}


BuiltinResult mult_number(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out) {
	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return BUILTIN_ERROR; // not implemented yet

	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) {
		(*out).value.integer = a.value.integer * b.value.integer;
		SET_INLINE_INTEGER_TRUE((*out).meta.coreFlags);
	}
	else if (!INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.floating * b.value.integer;
	else if (INLINE_INTEGER(a.meta.coreFlags) && !INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = a.value.integer * b.value.floating;
	else (*out).value.floating = a.value.floating * b.value.floating;

	*id_out = NUMBER_ID;

	return BUILTIN_SUCCESS;
}


BuiltinResult div_number(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out) {
	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return BUILTIN_ERROR; // not implemented yet

	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) {
		if (b.value.integer == 0) goto div_by_zero;
		(*out).value.floating = ((double) a.value.integer) / ((double) b.value.integer);
	}
	else if (!INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) {
		if (b.value.integer == 0) goto div_by_zero;
		(*out).value.floating = a.value.floating / b.value.integer;
	}
	else if (INLINE_INTEGER(a.meta.coreFlags) && !INLINE_INTEGER(b.meta.coreFlags)) {
		if (b.value.floating == 0) goto div_by_zero;
		(*out).value.floating = a.value.integer / b.value.floating;
	}
	else {
		if (b.value.floating == 0) goto div_by_zero;
		(*out).value.floating = a.value.floating / b.value.floating;
	}

	*id_out = NUMBER_ID;

	return BUILTIN_SUCCESS;

	div_by_zero:
		printf("Division by zero.\n");
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


BuiltinResult exp_number(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out) {
	if (GMP_NUMBER(a.meta.coreFlags) || GMP_NUMBER(b.meta.coreFlags)) return BUILTIN_ERROR; // not implemented yet

	if (INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags)) 
		(*out).value.integer = _powi(a.value.integer, b.value.integer);
	else if (!INLINE_INTEGER(a.meta.coreFlags) && INLINE_INTEGER(b.meta.coreFlags))
		(*out).value.floating = powf(a.value.floating, b.value.integer); 
	else if (INLINE_INTEGER(a.meta.coreFlags) && !INLINE_INTEGER(b.meta.coreFlags)) 
		(*out).value.floating = powf(a.value.integer, b.value.floating); 
	else (*out).value.floating = powf(a.value.floating, b.value.floating); 

	*id_out = NUMBER_ID;

	return BUILTIN_SUCCESS;
}


bool initNumbers(Registry *registry) {
	Info(0, "Initializing numbers\n");
	if (!registerObject(registry, LIB_CORE_8, NUMBER_ID, cleanup_number, compare_number, copy_number, print_number)) return false;

	if (!addOperationImplementation(registry, '+', add_number, (uint64_t[]) {NUMBER_ID, NUMBER_ID})) return false;
	if (!addOperationImplementation(registry, '-', sub_number, (uint64_t[]) {NUMBER_ID, NUMBER_ID})) return false;
	if (!addOperationImplementation(registry, '*', mult_number, (uint64_t[]) {NUMBER_ID, NUMBER_ID})) return false;
	if (!addOperationImplementation(registry, '/', div_number, (uint64_t[]) {NUMBER_ID, NUMBER_ID})) return false;
	if (!addOperationImplementation(registry, '^', exp_number, (uint64_t[]) {NUMBER_ID, NUMBER_ID})) return false;

	return true;
}
