#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "tokenizer.h"

int main() {

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    Token *head = tokenize(expression);
    printTokens(head);

    free(expression);

    freeTokens(head);

    return 0;
}