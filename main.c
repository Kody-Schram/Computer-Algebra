#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "parsing/parser.h"
#include "parsing/parserTypes.h"

int main() {

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    Token *head = parse(expression);
    printTokens(head);
    // freeTokens(head);

    return 0;
}