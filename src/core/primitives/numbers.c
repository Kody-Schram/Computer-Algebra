#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "numbers.h"
#include "core/common.h"
#include "core/context.h"


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
	if (!INLINE_INTEGER(aFlags) && !INLINE_INTEGER(bFlags)) return a.floating - b.floating;

	if (!INLINE_INTEGER(aFlags)) return a.floating- b.integer;
	else if (!INLINE_INTEGER(bFlags)) return a.integer - b.floating;
	return a.integer - b.integer;
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
}


bool initNumbers(Registry *registry) {
	Object number;
	if (!createObject(&number, LIB_CORE_8, cleanup_number, compare_number, copy_number, print_number)) return true;
	if (!registerObject(registry, number, NUMBER_ID)) return false;

	return true;
}
