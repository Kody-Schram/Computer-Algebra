#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "parsing/tables/functions.h"

#include "parsing/parser.h"
#include "parsing/parserTypes.h"

int main() {

    if (!initFunctionTable()) {
        return 0;
    }

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    ASTNode *head = parse(expression, 0);
    if (head == NULL) {
        printf("Error parsing input.\n");
    }

    return 0;
}