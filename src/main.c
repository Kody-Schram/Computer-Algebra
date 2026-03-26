#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils/context/context.h"
#include "utils/types.h"
#include "utils/log.h"

#include "utils/input.h"
#include "parsing/parser.h"
#include "parsing/parserTypes.h"

#include "execute.h"


static int handleKeywords(char *buffer) {
    for (int i = 0; i < sizeof(GLOBALCONTEXT->config->MAPPING) / sizeof(KeywordMapping); i ++) {

        if (strncmp(buffer, GLOBALCONTEXT->config->MAPPING[i].keyword, fmin(strlen(buffer), strlen(GLOBALCONTEXT->config->MAPPING[i].keyword)))) continue;
        Debug(0, "Found keyword '%s'\n", buffer);
        switch (GLOBALCONTEXT->config->MAPPING[i].cmd) {
            case K_QUIT:
                return -1;
            case K_ENV:
                printf("\n");
                printStream(printEnvironment(GLOBALCONTEXT->env));
                printf("\n");
                return 1;
            case K_RELOAD:
                freeConfig(GLOBALCONTEXT->config);
                GLOBALCONTEXT->config = loadConfig();
                printf("Reloaded config.\n");
                return 1;
        }
    }

    return 0;
}


static int process(char *buffer) {
    Debug(0, "\nProcessing '%s'\n", buffer);
    int result = handleKeywords(buffer);
    if (result == -1) return 0;
    else if (result == 1) return 1;

    ASTNode *head = parse(buffer);
    if (head != NULL) execute(&head);

    if (head == NULL) return 1;

    char *str = astToString(head);
    if (str != NULL) printf("%s\n", str);
    free(str);

    if (GLOBALCONTEXT->config->OUTPUTS > 0) {
        Debug(0, "Updating output variable(s).\n");
        if (GLOBALCONTEXT->config->OUTPUTS == 1) {
            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, GLOBALCONTEXT->config->OUTPUT_ID);
            if (cmp == NULL) return 0;

            Debug(0, "Replacing\n");
            freeAST(cmp->value);
            cmp->value = head;
            return 1;
        } else {
            int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
            char *str = malloc(size);
            if (str == NULL) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, GLOBALCONTEXT->config->OUTPUTS - 1);

            Component *last = searchEnvironment(GLOBALCONTEXT->env, str);
            if (last == NULL) {
                free(str);
                return 0;
            }
            free(str);
            freeAST(last->value);

            for (int i = GLOBALCONTEXT->config->OUTPUTS - 2; i >= 0; i --) {
                size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
                str = malloc(size);
                if (str == NULL) return 0;
                snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

                Component *cmp = searchEnvironment(GLOBALCONTEXT->env, str);
                if (cmp == NULL) {
                    free(str);
                    return 0;
                }
                free(str);

                Debug(0, "Replacing\n");
                last->value = cmp->value;
                last = cmp;
            }

            size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
            str = malloc(size);
            if (str == NULL) return 0;
            snprintf(str, size, "%s_0", GLOBALCONTEXT->config->OUTPUT_ID);

            Component *first = searchEnvironment(GLOBALCONTEXT->env, str);
            if (first == NULL) {
                free(str);
                return 0;
            }
            free(str);

            first->value = head;

            return 1;
        }

        return 1;
    } else freeAST(head);

    return 1;
}


static int runStartup() {
    // Load startup script
    if (GLOBALCONTEXT->config->STARTUP != NULL) {
        Debug(0, "\nRunning Startup Script\n");
        char *line = strtok(GLOBALCONTEXT->config->STARTUP, "\n");

        while (line != NULL) {
            if (!strcmp(line, "FILE")) {
                Debug(0, "Running startup script from file.\n");
                FILE *file = fopen(strtok(line, "\n"), "r");
                if (file == NULL) {
                    printf("Couldn't load startup script.\n");
                    return 0;
                }

                char buffer[128];
                while (fgets(buffer, 128, file)) {
                    printf("S > %s\n", line);
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
            line = strtok(NULL, "\n");
        }

        free(GLOBALCONTEXT->config->STARTUP);
        GLOBALCONTEXT->config->STARTUP = NULL;
    }

    return 1;
}


static int initOutputVariables() {
    int outputs = GLOBALCONTEXT->config->OUTPUTS;
    if (outputs > 0) {
        if (outputs == 1) {
            ASTNode *temp = dummyASTNode(NODE_NUMBER);
            if (temp == NULL) return 0;
            temp->value = 0;

            if (!bindComponent(GLOBALCONTEXT->env, VARIABLE, GLOBALCONTEXT->config->OUTPUT_ID, temp)) {
                free(temp);
                return 0;
            }
            return 1;
        }

        for (int i = 0; i < outputs; i ++) {
            int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;

            char *str = malloc(size);
            if (str == NULL) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

            ASTNode *temp = dummyASTNode(NODE_NUMBER);
            if (temp == NULL) return 0;
            temp->value = 0;

            if (!bindComponent(GLOBALCONTEXT->env, VARIABLE, str, temp)) {
                free(temp);
                free(str);
                return 0;
            }

            free(str);
        }
    }

    return 1;
}


int main() {
    if (!initContext())  {
        freeContext(GLOBALCONTEXT);
        return 1;
    }
    Debug(0, "Context created.\n");
    Info(1, printConfig(GLOBALCONTEXT->config));

    if (!initOutputVariables()) {
        freeContext(GLOBALCONTEXT);
        return 1;
    }

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