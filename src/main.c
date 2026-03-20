#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/types.h"
#include "utils/config/config.h"

#include "utils/input.h"
#include "utils/env/environment.h"
#include "parsing/parser.h"
#include "parsing/parserTypes.h"

int main() {

    Config *config = loadConfig();
    if (config == NULL) return 0;

    Environment *global_env = createEnvironment();
    if (global_env == NULL) return 0;

    int line = 1;

    int first = 0;
    char entry[] = "f:x->2x";

    // System loop
    while (1) {
        char *input;
        if (first) {
            input = entry;
            first = 0;
        } else input = terminalEntry(line);

        line ++;

        if (!strcmp(input, "quit")) break;

        ASTNode *head = parse(input, global_env, 1);
        if (head == NULL) return 0;

        printf("\nInput Parsed\n");

        // Updates environment if an assignment is returned
        if (head->type == NODE_ASSIGN_FUNC || head->type == NODE_ASSIGN_VAR) {
            printf("Updating environment.\n");
            
            ComponentType type;
            switch(head->type) {
                case NODE_ASSIGN_FUNC:
                    type = FUNCTION;
                    break;
                default:
                    type = VARIABLE;
            }

            bindComponent(global_env, type, head->left->identifier, head->right);

            // Free assignment nodes after assignment
        }

        printEnvironment(global_env);
    }

    return 0;
}