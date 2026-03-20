#include <stdio.h>
#include <yaml.h>

#include "config.h"

Config *loadConfig() {
    printf("\nLoading Config\n");

    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize(&parser);

    char *paths[] = {
        "config.yaml",
        "../config.yaml",
        "~/.config/" APP_NAME "/config.yaml"
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
}