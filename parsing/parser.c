#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "codegen/tokenizer.h"
#include "codegen/lexer.h"
#include "codegen/ast.h"

void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}

void printRPN(RPNList list) {
    for (int i = 0; i < list.length; i ++) {
        printf("%s\n", list.rpn[i]->value);
    }
}


Token *parse(char *buffer) {
    Token *raw = tokenize(buffer);
    if (raw == NULL) return NULL;

    // printf("\nRaw Tokens\n");
    // printTokens(raw);

    Token *head = lex(raw);

    if (head == NULL) return NULL;

    RPNList RPN = shuntingYard(head);
    printRPN(RPN);

    return head;
}