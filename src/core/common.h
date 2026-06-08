#pragma once 

#include <stdint.h>

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif

#define LIB_CORE_8 CREATE_REF_8("lib_core")

#define REFERENCE_LENGTH 8
#define CREATE_REF_8(s) \
	((uint64_t)(s[0]) | ((uint64_t)(s[1]) << 8) | \
	 ((uint64_t)(s[2]) << 16) | ((uint64_t)(s[3]) << 24) | \
	 ((uint64_t)(s[4]) << 32) | ((uint64_t)(s[5]) << 40) | \
	 ((uint64_t)(s[6]) << 48) | ((uint64_t)(s[7]) << 56))


// Bit operations for core flags
#define SET_BIT_TRUE(flags, MASK) ((flags) |= (MASK))
#define SET_BIT_FALSE(flags, MASK) ((flags) &= ~(MASK))
#define GET_BIT(flags, MASK) (((flags) & (MASK)) != 0)


#define NEGATIVE_MASK (1 << 0)
#define SET_NEGATIVE_TRUE(flags) (SET_BIT_TRUE((flags), NEGATIVE_MASK))
#define SET_NEGATIVE_FALSE(flags) (SET_BIT_FALSE((flags), NEGATIVE_MASK))
#define NEGATIVE(flags) (GET_BIT((flags), NEGATIVE_MASK))


#define INLINE_INTEGER_MASK (1 << 1)
#define SET_INLINE_INTEGER_TRUE(flags) (SET_BIT_TRUE((flags), INLINE_INTEGER_MASK))
#define SET_INLINE_INTEGER_FALSE(flags) (SET_BIT_FALSE((flags), INLINE_INTEGER_MASK))
#define INLINE_INTEGER(flags) (GET_BIT((flags), INLINE_INTEGER_MASK))


#define GMP_NUMBER_MASK (1 << 2)
#define SET_GMP_NUMBER_TRUE(flags) (SET_BIT_TRUE((flags), GMP_NUMBER_MASK))
#define SET_GMP_NUMBER_FALSE(flags) (SET_BIT_FALSE((flags), GMP_NUMBER_MASK))
#define GMP_NUMBER(flags) (GET_BIT((flags), GMP_NUMBER_MASK))


#define CONSTANT_MASK (1 << 3)
#define SET_CONSTANT_TRUE(flags) (SET_BIT_TRUE((flags), CONSTANT_MASK))
#define SET_CONSTANT_FALSE(flags) (SET_BIT_FALSE((flags), CONSTANT_MASK))
#define IS_CONSTANT(flags) (GET_BIT((flags), CONSTANT_MASK))


// Defines the bounds for core flags vs module flags
#define MODULE_FLAG_MASK(bit_index) (1 << (8 + (bit_index)))


typedef struct Context Context;


typedef enum BuiltinResultType BuiltinResultType;
typedef enum BuiltinResult BuiltinResult;

typedef enum ComponentType ComponentType;

typedef struct Component Component;


//typedef enum uint8_t ExpressionType;
typedef struct Expression Expression;

typedef struct Object Object; 
typedef struct ObjectData ObjectData;


typedef enum OperationType OperationType;
typedef struct Operation Operation;
typedef BuiltinResult (*OperationImplementation)(Context const *ctx, ObjectData a, ObjectData b, ObjectData *out, uint64_t *id_out);


typedef enum FunctionType FunctionType;
typedef struct Function Function;
typedef BuiltinResult (*FunctionImplementation)(Context const *ctx, Expression **inputs, uint32_t nInputs, Expression **out);


typedef enum Associativity {
	ASSOC_LEFT,
	ASSOC_RIGHT,
	ASSOC_BOTH
} Associativity;


typedef struct uint128_t {
	uint64_t bits[2];
} uint128_t; 


#define COMPARE_UINT128_T(a, b) \
	(((a).bits[0] == (b).bits[0]) & ((a).bits[1] == (b).bits[0]))


struct Operation {

	Associativity associativity;

    bool commutative;
    char symbol;
	
	uint8_t precedence;

	uint32_t implementationSize;
    uint32_t nImplementations;
	uint128_t *implementation_map;
	OperationImplementation *implementations;
};


typedef union {
	void *data;
	int64_t integer;
	double floating;
} ObjectValue;


typedef struct ExpressionMeta {
	uint8_t coreFlags;
	uint8_t weight;
	uint8_t computeWeight;
} ExpressionMeta;


struct ObjectData {
	ObjectValue value;
	uint32_t flags;
	ExpressionMeta meta;
};


#define BUILD_OBJECT_DATA(expr_ptr) \
	(ObjectData) {.meta = (expr)->meta, .flags = (expr)->flags, .value = (expr)->value}


struct Object {
	uint64_t module;
	void (*cleanup)(ObjectData data);
	int32_t (*compare)(ObjectData const a, ObjectData const b);
	bool (*copy)(ObjectValue const src, ObjectValue *dest, ExpressionMeta meta, uint32_t flags);
	char *(*print)(ObjectData const data);
};


#define EXPRESSION_OPERATOR 0
#define EXPRESSION_FUNCTION_CALL 1
#define EXPRESSION_VARIABLE 2
#define EXPRESSION_OBJECT 3




struct Expression {
    uint8_t type;
	ExpressionMeta meta;

	union {
		uint32_t nOperands;
		uint32_t nInputs;
		uint32_t flags;
	};

    union {
		// Operations
        struct {
            Operation const *op;
            struct Expression **operands;
        };

		// Mathematical objects
		struct {
			uint64_t objectId;
			ObjectValue value;
		};

		// Function calls
		struct {
			struct Expression **inputs;
			Component const *cmp;
		};

        char *identifier;
    };
};


enum FunctionType {
    BUILTIN,
    DEFINED,
};

enum BuiltinResult {
    BUILTIN_SUCCESS,
    BUILTIN_NEUTRAL,
    BUILTIN_ERROR
};


struct Function {
    FunctionType type;
    uint32_t nParameters;
    char **parameters;

    union {
        Expression *definition;
		FunctionImplementation implementation;
    };
};


enum ComponentType {
    COMP_VARIABLE,
    COMP_FUNCTION
};

struct Component {
    char *identifier;
    // Pack secondary hashing to have super fast comparisions when tokenizing
    Component *next;

    union {
        Function *func;
        Expression *value;
    };
    ComponentType type;
};
