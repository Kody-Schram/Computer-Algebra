#include <stdio.h>
#include <yaml.h>
#include <string.h>

#include "config.h"
#include "types.h"

typedef enum {
    STATE_START,
    STATE_STREAM,
    STATE_DOCUMENT,
    STATE_SECTION,

    STATE_LOG_LEVEL,
    STATE_LOG_LOCATION,

    STATE_STOP
} State;


static int consumeEvent(State *state, yaml_event_t *event, Config *config) {
    printf("State: %d\n", *state);
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
                if (!strcmp(value, "log_level")) *state = STATE_LOG_LEVEL;
                else if (!strcmp(value, "log_location")) *state = STATE_LOG_LOCATION;
                else {
                    printf("Unexpected scalar: %s\n", value);
                    return 0;
                }
                break;
            case YAML_DOCUMENT_END_EVENT:
                *state = STATE_STREAM;
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
        }
        break;
    
    case STATE_LOG_LEVEL:
        switch (event->type) {
            case YAML_MAPPING_END_EVENT:
                *state = STATE_SECTION;
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
                }
                break;
            default:
                printf("Unexpected event %d in state %d.\n", event->type, *state);
                return 0;
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
}


Config *loadConfig() {
    Config *config = malloc(sizeof(config));
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

    return config;
}

void printConfig(Config *config) {
    fprintf(config->LOG_STREAM, "\nConfig\n");
    fprintf(config->LOG_STREAM, "Log Level: %d\n", config->LOG_LEVEL);
}

void freeConfig(Config *config) {
    free(config);
    // will need more later on
}