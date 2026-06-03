#pragma once

#include <stdint.h>
#include <stdio.h>
#include "core/primitives/types.h"


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
    bool LAZY_CALLS;
	bool STRICT;
};


FILE *printConfig(Config const *config);


bool reloadConfig(Context *ctx);


// =======================================
//               Environment
// =======================================




Environment *createEnvironment();


bool bindComponent(Environment *env, ComponentType type, char const *identifier, void const *data);


Component *searchEnvironment(Environment const *env, char const *identifier);


void freeEnvironment(Environment *env);


FILE *printEnvironment(Environment const *env);


// ============================================
//                   Registry
// ============================================

Operation const *searchOperation(Registry const *registry, char symbol);


bool registerOperation(Registry *registry, Operation op);


bool registerObject(Registry *registry, Object obj, uint64_t id);


Object const *searchObject(Registry const *registry, uint64_t obj_id);


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t operands);


bool createObject(Object *out, uint64_t originModule,
		void (*cleanup)(void *data), int32_t (*compare)(void const *a, void const *b),
		void *(*copy)(void const *src)
);


bool addOperationImplementation(Registry *registry, char symbol, BuiltinImplementation fn);
