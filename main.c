#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "parsing/functions.h"

#include "parsing/parser.h"
#include "parsing/parserTypes.h"

int main() {

    if (!initFunctionTable()) {
        return 0;
    }

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    Token *head = parse(expression);
    printf("\nFinal Token List\n");
    printTokens(head);
    
    freeTokens(head);

    return 0;
}