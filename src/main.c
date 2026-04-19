#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/types.h"
#include "core/axioms.h"
#include "core/utils/log.h"

#include "core/utils/input.h"
#include "core/parsing/parser.h"

#include "core/execution/execute.h"


static int handleKeywords(char *buffer) {
    for (int i = 0; i < sizeof(GLOBALCONTEXT->config->MAPPING) / sizeof(KeywordMapping); i ++) {

        if (strncmp(buffer, GLOBALCONTEXT->config->MAPPING[i].keyword, fmin(strlen(buffer), strlen(GLOBALCONTEXT->config->MAPPING[i].keyword)))) continue;
        Debug(0, "Found keyword '%s'\n", buffer);
        switch (GLOBALCONTEXT->config->MAPPING[i].cmd) {
            case K_QUIT:
                return -1;
            case K_ENV:
                printf("\nEnvironment\n\n");
                printStream(printEnvironment(GLOBALCONTEXT->env));
                printf("\n");
                return 1;
            case K_RELOAD:
                printf("Reloading config.\n");

                freeConfig(GLOBALCONTEXT->config);
                GLOBALCONTEXT->config = loadConfig(nullptr);
                if (GLOBALCONTEXT->config == nullptr) return -1;

                Info(1, printConfig(GLOBALCONTEXT->config));
                return 1;
        }
    }

    return 0;
}


static int process(char *buffer) {
    Debug(0, "\nProcessing '%s'\n", buffer);
    int keyword = handleKeywords(buffer);
    if (keyword == -1) return 0;
    else if (keyword == 1) return 1;

    ParserResult result = parse(buffer);
    if (result.type == PARSER_ERROR) return 0;
    if (result.expr == nullptr) return 1;

    if (!execute(&result.expr)) {
        freeExpression(result.expr);
        return 1;
    }

    char *str = expressionToString(result.expr);
    if (str != nullptr) printf("%s\n\n", str);
    free(str);

    // Updates output variables
    if (GLOBALCONTEXT->config->OUTPUTS > 0) {
        Debug(0, "Updating output variable(s).\n");
        if (GLOBALCONTEXT->config->OUTPUTS == 1) {
            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, GLOBALCONTEXT->config->OUTPUT_ID);
            if (cmp == nullptr) return 0;

            freeExpression(cmp->value);
            cmp->value = result.expr;
            return 1;
        } else {
            int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
            char *str = malloc(size);
            if (str == nullptr) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, GLOBALCONTEXT->config->OUTPUTS - 1);

            Component *last = searchEnvironment(GLOBALCONTEXT->env, str);
            if (last == nullptr) {
                free(str);
                return 0;
            }
            free(str);
            freeExpression(last->value);

            for (int i = GLOBALCONTEXT->config->OUTPUTS - 2; i >= 0; i --) {
                size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
                str = malloc(size);
                if (str == nullptr) return 0;
                snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

                Component *cmp = searchEnvironment(GLOBALCONTEXT->env, str);
                if (cmp == nullptr) {
                    free(str);
                    return 0;
                }
                free(str);

                last->value = cmp->value;
                last = cmp;
            }

            size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
            str = malloc(size);
            if (str == nullptr) return 0;
            snprintf(str, size, "%s_0", GLOBALCONTEXT->config->OUTPUT_ID);

            Component *first = searchEnvironment(GLOBALCONTEXT->env, str);
            if (first == nullptr) {
                free(str);
                return 0;
            }
            free(str);

            first->value = result.expr;

            return 1;
        }

        return 1;
    } else freeExpression(result.expr);

    return 1;
}


static int runStartup() {
    // Load startup script
    if (GLOBALCONTEXT->config->STARTUP != nullptr) {
        Debug(0, "\nRunning Startup Script\n");
        char *line = strtok(GLOBALCONTEXT->config->STARTUP, "\n");

        while (line != nullptr) {
            if (!strcmp(line, "FILE")) {
                line = strtok(nullptr, "\n");
                Debug(0, "Running startup script from file: '%s'.\n", line);
                FILE *file = fopen(line, "r");
                if (file == nullptr) {
                    perror("Couldn't load startup script");
                    return 0;
                }

                char buffer[128];
                while (fgets(buffer, 128, file)) {
                    printf("S > %s\n", buffer);
                    if (!process(buffer)) {
                        fclose(file);
                        return 0;
                    }
                }
                fclose(file);
                break;
            } else {
                printf("S > %s\n", line);
                if (!process(line)) return 0;
            }
            line = strtok(nullptr, "\n");
        }

        free(GLOBALCONTEXT->config->STARTUP);
        GLOBALCONTEXT->config->STARTUP = nullptr;
    }

    return 1;
}


int main(int argc, char *argv[]) {
    char *cpath = nullptr;
    if (argc > 1) {
        cpath = argv[1];
        printf("Loading config from '%s'\n", cpath);
    }

    printf("Loading context\n");
    if (!initContext(cpath))  {
        freeContext(GLOBALCONTEXT);
        return 1;
    }
    
    if (!initAxioms()) {
        freeContext(GLOBALCONTEXT);
        return 1;
    }
    
    Debug(0, "Context created.\n");
    Info(1, printConfig(GLOBALCONTEXT->config));
    Info(1, printEnvironment(GLOBALCONTEXT->env));

    if (!runStartup()) {
        freeContext(GLOBALCONTEXT);
        return 0;
    }

    int line = 1;
    
    // System loop
    while (1) {
        char *input = terminalEntry(line);
        line ++;

        if (!process(input)) {
            free(input);
            freeContext(GLOBALCONTEXT);
            return 0;
        }

        free(input);
    }

    return 0;
}
