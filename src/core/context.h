#pragma once

#include <stdint.h>
#include <stdio.h>
#include "core/common.h"


typedef enum LOG_LEVEL LOG_LEVEL;
typedef struct KeywordMapping KeywordMapping;
typedef struct Config Config;


typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef struct Environment Environment;


typedef struct Registry Registry;


// ============================================
//                  Context
// ============================================

struct Context {
    Environment *env;
    Config *config;
	Registry *registry;
};


extern Context *GLOBALCONTEXT;


bool initContext(char const *cpath);


void freeContext(Context *context);


bool updateOutputVariables(Context *ctx, Expression *output);


// =========================================
//              Configuration
// =========================================

enum LOG_LEVEL {
    NONE,
    INFO,
    DEBUG
};


typedef enum {
    K_QUIT,
    K_ENV,
    K_RELOAD
} KeywordCMD;


struct KeywordMapping {
    KeywordCMD cmd;
    char *keyword;
};


struct Config {
    LOG_LEVEL LOG_LEVEL;
    int OUTPUTS;
    
    char *CONFIG_FILE_PATH;
    
    FILE *LOG_STREAM;
    char *STARTUP;
    char *OUTPUT_ID;
    
    KeywordMapping MAPPING[3];

    bool PRESERVE_FRACS;
    bool LAZY_RESOLUTIONS;
	bool STRICT;
};


FILE *printConfig(Config const *config);


bool reloadConfig(Context *ctx);


// =======================================
//               Environment
// =======================================




Environment *createEnvironment();


bool bindComponent(Environment *env, ComponentType type, char *identifier, void const *data);


Component const *searchEnvironment(Environment const *env, char const *identifier);


void freeEnvironment(Environment *env);


FILE *printEnvironment(Environment const *env);


// ============================================
//                   Registry
// ============================================

Operation const *searchOperation(Registry const *registry, char symbol);


bool registerOperation(Registry *registry, Operation op);


bool registerObject(
		Registry *registry, uint64_t originModule, uint64_t id,
		void (*cleanup)(ObjectData data),
		int32_t (*compare)(ObjectData const a, ObjectData const b),
		bool (*copy)(ObjectValue const src, ObjectValue *dest, ExpressionMeta meta, uint32_t flags),
		char *(*print)(ObjectData const data)
);


Object const *searchObject(Registry const *registry, uint64_t obj_id);


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t operands);


bool addOperationImplementation(
		Registry *registry, char symbol,
		OperationImplementation implementation, uint64_t ids[2]
);


BuiltinResult dispatchOperation(Operation const *op, 
		uint64_t id_a, ObjectData a, uint64_t id_b, ObjectData b, 
		ObjectData *out, uint64_t *id_out
);

