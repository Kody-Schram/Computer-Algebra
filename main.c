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

    ASTNode *head = parse(expression, global_env, 0, 1);
    if (head == NULL) {
        printf("Error parsing input.\n");
    }

    return 0;
}