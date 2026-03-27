#ifndef CONFIG_H
#define CONFIG_H

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
    FILE *LOG_STREAM;

    char *STARTUP;

    KeywordMapping MAPPING[3];

    int OUTPUTS;
    char *OUTPUT_ID;

    int PRESERVE_FRACS;
    int LAZY_CALLS;
};


Config *loadConfig();


FILE *printConfig(Config *config);


void freeConfig(Config *config);


#endif