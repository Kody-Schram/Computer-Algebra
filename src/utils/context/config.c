#include <stdio.h>
#include <yaml.h>
#include <string.h>

#include "config.h"
#include "utils/types.h"

static const int DEFAULT_MAPPING_SIZE = 5;

typedef enum {
    STATE_START,
    STATE_STREAM,
    STATE_DOCUMENT,
    STATE_SECTION,

    STATE_LOG,
    STATE_LOG_LEVEL,
    STATE_LOG_LOCATION,

    STATE_STARTUP,

    STATE_KEYWORDS,
    STATE_K_QUIT,
    STATE_K_ENV,
    STATE_K_RELOAD,

    STATE_STOP
} State;


static int consumeEvent(State *state, yaml_event_t *event, Config *config) {
    char *value;

    switch (*state) {
    case STATE_START:
        switch (event->type) {
            case YAML_STREAM_START_EVENT:
                *state = STATE_STREAM;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
            }
        break;

    case STATE_STREAM:
        switch (event->type) {
            case YAML_DOCUMENT_START_EVENT:
                *state = STATE_DOCUMENT;
                break;
            case YAML_STREAM_END_EVENT:
                *state = STATE_STOP;
                break;
        
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
            }
        break;

    case STATE_DOCUMENT:
        switch (event->type) {
            case YAML_MAPPING_START_EVENT:
                *state = STATE_SECTION;
                break;
            case YAML_DOCUMENT_END_EVENT:
                *state = STATE_STREAM;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
        }
        break;

    case STATE_SECTION:
        switch (event->type) {
            case YAML_SCALAR_EVENT:
                value = event->data.scalar.value;
                if (!strcmp(value, "log")) *state = STATE_LOG;
                else if (!strcmp(value, "startup")) *state = STATE_STARTUP;
                else if (!strcmp(value, "keywords")) *state = STATE_KEYWORDS;
                else {
                    printf("Unexpected scalar: %s\n", value);
                    return 0;
                }
                break;
            case YAML_DOCUMENT_END_EVENT:
                *state = STATE_STREAM;
                break;
            case YAML_MAPPING_END_EVENT:
                *state = STATE_DOCUMENT;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
        }
        break;

    case STATE_LOG:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_SECTION;
                break;
            case YAML_SCALAR_EVENT:
                value = event->data.scalar.value;
                if (!strcmp(value, "level")) *state = STATE_LOG_LEVEL;
                else if (!strcmp(value, "location")) *state = STATE_LOG_LOCATION;
                else {
                    printf("Unexpected scalar: %s\n", value);
                    return 0;
                }
                break;
        }
        break;
    
    case STATE_LOG_LEVEL:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_LOG;
                break;
            case YAML_SCALAR_EVENT:
                value = (char *) event->data.scalar.value;

                if (!strcmp(value, "None") || !strcmp(value, "0")) {
                    config->LOG_LEVEL = NONE;
                } else if (!strcmp(value, "Info") || !strcmp(value, "1")) {
                    config->LOG_LEVEL = INFO;
                } else if (!strcmp(value, "Debug") || !strcmp(value, "2")) {
                    config->LOG_LEVEL = DEBUG;
                } else {
                    printf("Unexpected log level: %s.\n", value);
                    return 0;
                }
                *state = STATE_LOG;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
        }
        break;

    case STATE_LOG_LOCATION:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_LOG;
                break;
            case YAML_SCALAR_EVENT:
                value = (char *) event->data.scalar.value;

                FILE *location = fopen(value, "wb+");
                config->LOG_STREAM = location;

                *state = STATE_LOG;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
        }
        break;

    case STATE_STARTUP:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_SECTION;
                break;
            case YAML_SCALAR_EVENT:
                config->STARTUP = strdup((char *) event->data.scalar.value);
                *state = STATE_SECTION;
                break;
        }
        break;

    case STATE_KEYWORDS:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_SECTION;
                break;

            case YAML_SCALAR_EVENT:
                char *value = event->data.scalar.value;
                if (!strcmp(value, "QUIT")) *state = STATE_K_QUIT;
                else if (!strcmp(value, "ENV")) *state = STATE_K_ENV;
                else if (!strcmp(value, "RELOAD")) *state = STATE_K_RELOAD;
                else {
                    printf("Unexpected keyword: %s.\n", value);
                    return 0;
                }
                break;
        }
        break;

    case STATE_K_QUIT:
    case STATE_K_ENV:
    case STATE_K_RELOAD:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_KEYWORDS;
                break;
            
            case YAML_SCALAR_EVENT:
                char *value = event->data.scalar.value;
                printf("key word %s\n", value);

                KeywordCMD cmd;
                switch (*state) {
                    case STATE_K_QUIT:
                        cmd = K_QUIT;
                        break;
                    case STATE_K_ENV:
                        cmd = K_ENV;
                        break;
                    case STATE_K_RELOAD:
                        cmd = K_RELOAD;
                        break;
                }

                // Replaces default setting
                for (int i = 0; i < sizeof(config->mapping) / sizeof(KeywordMapping); i ++) {
                    if (config->mapping[i].cmd == cmd) {
                        printf("replacing old mapping with '%s'\n", value);
                        free(config->mapping[i].keyword);

                        config->mapping[i].keyword = strdup(value);
                        *state = STATE_KEYWORDS;
                        return 1;
                    }
                }
            break;
        }
        break;
    
    case STATE_STOP:
        break;
    }

    return 1;
}


static void initConfig(Config *config) {
    config->LOG_LEVEL = 0;
    config->LOG_STREAM = stdout;
    config->STARTUP = NULL;

    config->mapping[0] = (KeywordMapping) {.cmd=K_QUIT, .keyword=strdup("quit")};
    config->mapping[1] = (KeywordMapping) {.cmd=K_ENV, .keyword=strdup("env")};
    config->mapping[2] = (KeywordMapping) {.cmd=K_RELOAD, .keyword=strdup("reload")};
}


Config *loadConfig() {
    Config *config = malloc(sizeof(Config));
    if (config == NULL) {
        fprintf(stdout, "Error allocating for config.\n");
        return NULL;
    }

    initConfig(config);

    char *paths[] = {
        "config.yaml",
        "../config.yaml",
        "~/.config/" PROJECT_NAME "/config.yaml"
    };
    int npaths = sizeof(paths) / sizeof(char *);
    int path = 0;

    FILE *cfile = fopen(paths[path], "rb");
    while (cfile == NULL) {
        path ++;
        if (path < npaths) cfile = fopen(paths[path], "rb");
        else break;
    }

    if (cfile == NULL) {
        printf("Error loading config.\n");
        return NULL;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, cfile);

    State parserState = STATE_START;

    int done = 0;

    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("YAML parser error %s\n", parser.problem);
            return NULL;
        }

        if (!consumeEvent(&parserState, &event, config)) return NULL;

        done = (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);

    if (config->LOG_STREAM != stdout) {
        setvbuf(config->LOG_STREAM, NULL, _IONBF, 0); 
    }

    return config;
}


FILE *printConfig(Config *config) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    fprintf(stream, "\nConfig\n");
    char *level;
    switch(config->LOG_LEVEL) {
        case NONE:
            level = "None";
            break;

        case INFO:
            level = "Info";
            break;

        case DEBUG:
            level = "Debug";
            break;

        default:
            level = "GRIFFITH!";
    }
    fprintf(stream, "Log Level: %s\n", level);
    fprintf(stream, "Startup: \n%s\n\n", config->STARTUP);

    for (int i = 0; i < sizeof(config->mapping) / sizeof(KeywordMapping); i ++) {
        switch (config->mapping[i].cmd) {
            case K_QUIT:
                fprintf(stream, "QUIT: ");
                break;

            case K_ENV:
                fprintf(stream, "ENV: ");
                break;

            case K_RELOAD:
                fprintf(stream, "RELOAD: ");
                break;
            
            default:
                fprintf(stream, "Guts....");
        }

        fprintf(stream, "%s\n", config->mapping[i].keyword);
    }

    return stream;
}


void freeConfig(Config *config) {
    fclose(config->LOG_STREAM);

    // Frees keyword identifiers
    for (int i = 0; i < sizeof(config->mapping) / sizeof(KeywordMapping); i ++) {
        free(config->mapping[i].keyword);
    }

    free(config);
}