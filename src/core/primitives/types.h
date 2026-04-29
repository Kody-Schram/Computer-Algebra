#ifndef TYPES_H
#define TYPES_H

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

    unsigned int arity; // Number of required operands
	unsigned int precedence;
    unsigned int nDefs;

    Function **definitions;
};

struct Expression {
    ExpressionType type;

    union {
        struct {
            const Operation *op;
            unsigned int nOperands;
            struct Expression **operands;
        };

        char *identifier;
        double value;
        long long integer;
        FunctionCall *call;
    };
};


// Function related definitions
struct FunctionCall {
    char *identifier;
    Expression **parameters;
    unsigned int nParams;
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
    unsigned int nParameters;
    char **parameters;

    union {
        Expression *definition;
        BuiltinResult (*builtin) (unsigned int nArgs, Expression ** exprs);
    };
};

#endif
