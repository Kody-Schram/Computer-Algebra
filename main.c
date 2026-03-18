#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "parsing/env/environment.h"

#include "parsing/parser.h"
#include "parsing/parserTypes.h"

int main() {

    Environment *global_env = createEnvironment();
    if (global_env == NULL) return 0;

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    ASTNode *head = parse(expression, global_env, 1);
    if (head == NULL) return 0;

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

    return 0;
}