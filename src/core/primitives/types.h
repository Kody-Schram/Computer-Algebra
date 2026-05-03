#pragma once 

#include <stdint.h>

#ifndef PROJECT_NAME
    #define PROJECT_NAME "project"
#endif


typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef struct Environment Environment;

// Forward declaring types
typedef enum OperationType OperationType;
typedef struct Operation Operation;
typedef enum ExpressionType ExpressionType;
typedef struct Expression Expression;

typedef struct FunctionCall FunctionCall;
typedef enum FunctionType FunctionType;
typedef enum BuiltinResultType BuiltinResultType;
typedef struct BuiltinResult BuiltinResult;
typedef struct Function Function;

typedef BuiltinResult (*BuiltinImplementation) (uint32_t nArgs, Expression **exprs);

// Expression related definitions
enum ExpressionType {
    EXPRESSION_VARIABLE,
    EXPRESSION_INTEGER,
    EXPRESSION_DOUBLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_OPERATOR
};


struct Operation {
    bool commutative;
	bool rightAssociative;
	bool leftAssociative;
    char symbol;

    uint32_t arity; // Number of required operands
	uint8_t precedence;

	uint32_t implementationSize;
    uint32_t nImplementations;
	BuiltinImplementation *implementations;
};

struct Expression {
    ExpressionType type;
	uint32_t nOperands;

    union {
        struct {
            const Operation *op;
            struct Expression **operands;
        };

        char *identifier;
        double value;
        int64_t integer;
        FunctionCall *call;
    };
};


// Function related definitions
struct FunctionCall {
    char *identifier;
    Expression **parameters;
    uint32_t nParams;
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
