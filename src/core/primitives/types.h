#pragma once 

#include <stdint.h>

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif


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
typedef struct BuiltinResult BuiltinResult;
typedef struct Function Function;


typedef struct Context Context;
typedef BuiltinResult (*BuiltinImplementation)(Context const *ctx, uint32_t nArgs, Expression **exprs);

// Expression related definitions
enum ExpressionType {
	EXPRESSION_OBJECT,
    EXPRESSION_VARIABLE,
    EXPRESSION_INTEGER,
    EXPRESSION_DOUBLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_OPERATOR
};

typedef enum associativity {
	ASSOC_LEFT,
	ASSOC_RIGHT,
	ASSOC_BOTH
} associativity;


struct Operation {

	associativity associativity;

    bool commutative;
    char symbol;
	
	uint8_t precedence;
    uint32_t arity; // Number of required operands


	uint32_t implementationSize;
    uint32_t nImplementations;
	BuiltinImplementation *implementations;
};

struct Expression {
    ExpressionType type;

	union {
		uint32_t nOperands;
		uint32_t nInputs;
	};

    union {
		// Operations
        struct {
            Operation const *op;
            struct Expression **operands;
        };

		// Mathematical objects
		struct {
			uint64_t const objectId;
			void *data;
		};

		// Function calls
		struct {
			struct Expression **inputs;
			Component const *cmp;
		};

        char *identifier;
        double value;
        int64_t integer;
    };
};


enum FunctionType {
    BUILTIN,
    DEFINED,
};

enum BuiltinResultType {
    BUILTIN_SUCCESS,
    BUILTIN_NEUTRAL,
    BUILTIN_ERROR
};

struct BuiltinResult {
    BuiltinResultType type;
    Expression *output;
};


struct Function {
    FunctionType type;
    uint32_t nParameters;
    char **parameters;

    union {
        Expression *definition;

		struct {
			uint32_t implementationSize;
			uint32_t nImplementations;
			BuiltinImplementation *implementations;
		};
    };
};


enum ComponentType {
    COMP_VARIABLE,
    COMP_FUNCTION
};

struct Component {
    ComponentType type;
    char *identifier;
    // Pack secondary hashing to have super fast comparisions when tokenizing
    Component *next;

    union {
        Function *func;
        Expression *value;
    };
};
