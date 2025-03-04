#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "tokenizer.h"
#include "lexer.h"

int main() {

    char *expression = terminalEntry();
    printf("you entered %s\n", expression);

    Token *raw = tokenize(expression);
    free(expression);

    Token *head = lex(raw);
    printTokens(head);

    freeTokens(head);

    return 0;
}