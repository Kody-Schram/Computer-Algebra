#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/types.h"
#include "utils/config.h"

#include "utils/input.h"
#include "utils/env/environment.h"
#include "parsing/parser.h"
#include "parsing/parserTypes.h"

#include "execute.h"

int main() {
    Config *config = loadConfig();
    if (config == NULL) return 0;

    if (config->LOG_LEVEL >= INFO) printConfig(config);

    Environment *global_env = createEnvironment();
    if (global_env == NULL) return 0;

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
                ASTNode *head = parse(line, global_env, config);
                if (head) {
                    execute(head, global_env, config);
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

        ASTNode *head = parse(input, global_env, config);
        if (head == NULL) continue;

        if (!execute(head, global_env, config)) continue;
    }

    freeConfig(config);

    return 0;
}