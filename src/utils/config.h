#ifndef CONFIG_H
#define CONFIG_H

typedef struct Config Config;
typedef enum LOG_LEVEL LOG_LEVEL;

enum LOG_LEVEL {
    NONE,
    INFO,
    DEBUG
};


struct Config {
    LOG_LEVEL LOG_LEVEL;
    FILE *LOG_STREAM;
    char *STARTUP;
};


Config *loadConfig();


void printConfig(Config *config);


void freeConfig(Config *config);

#endif