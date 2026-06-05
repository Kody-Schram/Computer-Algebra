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
#define SET_BIT_TRUE(flags, MASK) (flags |= MASK)
#define SET_BIT_FALSE(flags, MASK) (flags &= ~MASK)
#define GET_BIT(flags, MASK) ((flags & MASK) != 0)

#define NEGATIVE_MASK (1 << 0)
#define SET_NEGATIVE_TRUE(flags) (SET_BIT_TRUE(flags, NEGATIVE_MASK))
#define SET_NEGATIVE_FALSE(flags) (SET_BIT_FALSE(flags, NEGATIVE_MASK))
#define NEGATIVE(flags) (GET_BIT(flags, NEGATIVE_MASK))

#define INLINE_INTEGER_MASK (1 << 1)
#define SET_INLINE_INTEGER_TRUE(flags) (SET_BIT_TRUE(flags, INLINE_INTEGER_MASK))
#define SET_INLINE_INTEGER_FALSE(flags) (SET_BIT_FALSE(flags, INLINE_INTEGER_MASK))
#define INLINE_INTEGER(flags) (GET_BIT(flags, INLINE_INTEGER_MASK))

#define GMP_NUMBER_MASK (1 << 2)
#define SET_GMP_NUMBER_TRUE(flags) (SET_BIT_TRUE(flags, GMP_NUMBER_MASK))
#define SET_GMP_NUMBER_FALSE(flags) (SET_BIT_FALSE(flags, GMP_NUMBER_MASK))
#define GMP_NUMBER(flags) (GET_BIT(flags, GMP_NUMBER_MASK))


// Defines the bounds for core flags vs module flags
#define MODULE_FLAG_MASK(bit_index) (1 << (8 + (bit_index)))


typedef enum ComponentType ComponentType;
typedef struct Component Component;

// Forward declaring types
typedef struct Object Object; 

typedef enum OperationType OperationType;
typedef struct Operation Operation;

typedef enum ExpressionType ExpressionType;
typedef struct Expression Expression;

typedef enum FunctionType FunctionType;
typedef enum BuiltinResultType BuiltinResultType;
typedef enum BuiltinResult BuiltinResult;
typedef struct Function Function;


typedef struct Context Context;
typedef BuiltinResult (*BuiltinImplementation)(Context const *ctx, Expression **operands, uint32_t nArgs, Expression **out);

// Expression related definitions
enum ExpressionType { EXPRESSION_OBJECT,
    EXPRESSION_VARIABLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_OPERATOR
};

typedef enum Associativity {
	ASSOC_LEFT,
	ASSOC_RIGHT,
	ASSOC_BOTH
} Associativity;


struct Operation {

	Associativity associativity;

    bool commutative;
    char symbol;
	
	uint8_t precedence;
    uint32_t arity; // Number of required operands


	uint32_t implementationSize;
    uint32_t nImplementations;
	BuiltinImplementation *implementations;
};


typedef union {
	void *data;
	int64_t integer;
	double floating;
} ObjectValue;


struct Object {
	uint64_t module;
	void (*cleanup)(ObjectValue value, uint32_t flags);
	int32_t (*compare)(ObjectValue const a, uint32_t aFlags, ObjectValue const b, uint32_t bFlags);
	bool (*copy)(ObjectValue const src, ObjectValue *dest, uint32_t flags);
	char *(*print)(ObjectValue const value, uint32_t flags);
};




struct Expression {
    ExpressionType type;

	union {
		uint32_t nOperands;
		uint32_t nInputs;
		struct {
			uint32_t flags;
		};
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
		BuiltinImplementation implementation;
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
