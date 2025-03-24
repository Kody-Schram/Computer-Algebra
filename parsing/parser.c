#include <stdlib.h>

#include "parserTypes.h"
#include "codegen/tokenizer.h"
#include "codegen/lexer.h"

void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}


Token *parse(char *buffer) {
    Token *raw = tokenize(buffer);
    if (raw == NULL) return NULL;

    // printf("\nRaw Tokens\n");
    // printTokens(raw);

    Token *head = lex(raw);

    if (head == NULL) return NULL;

    return head;
}