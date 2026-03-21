#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/context/context.h"
#include "utils/types.h"

#include "utils/input.h"
#include "parsing/parser.h"
#include "parsing/parserTypes.h"

#include "execute.h"

int main() {
    if (!initContext()) return 0;
    Config *config = GLOBALCONTEXT->config;

    if (config->LOG_LEVEL >= INFO) {
        fprintf(config->LOG_STREAM, "Context created.\n");
        printConfig(config);
    }

    int line = 1;

    // Load startup script
    if (config->STARTUP != NULL) {
        if (config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "\nRunning Startup Script\n");
        char *line = strtok(config->STARTUP, "\n");

        while (line != NULL) {
            if (!strcmp(line, "FILE")) {
                
                line = strtok(config->STARTUP, "\n");
                if (line) {
                    // load startup script from file and leave loop
                }
                break;
            } else {
                printf("S > %s\n", line);
                ASTNode *head = parse(line);
                if (head) {
                    if (execute(head) && config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "Successfully Executed.\n");
                }
            }
            line = strtok(NULL, "\n");
        }

        free(config->STARTUP);
        config->STARTUP = NULL;
    }

    // System loop
    while (1) {
        char *input = terminalEntry(line);
        line ++;

        if (!strcmp(input, "quit")) break;

        ASTNode *head = parse(input);
        if (head == NULL) {

            if (execute(head) && config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "Successfully Executed.\n");
        }
    }

    freeConfig(config);

    return 0;
}