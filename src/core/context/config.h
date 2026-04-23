#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

typedef enum LOG_LEVEL LOG_LEVEL;
typedef struct KeywordMapping KeywordMapping;
typedef struct Config Config;


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
    bool PRINT_AXIOMATIC_OPS;
};


Config *loadConfig(char *cpath);


FILE *printConfig(Config *config);


void freeConfig(Config *config);


#endif