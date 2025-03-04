#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"

Token *createToken(TokenType type, char *value, int l) {
    Token *token = (Token*) malloc(sizeof(Token));
    if (token == NULL) {
        printf("Error allocating space for token.");
        return NULL;
    }

    // Populates newToken attributes
    token->type = type;
    token->value = (char*) malloc(l+1);
    if (token->value == NULL) {
        printf("Error allocating space for token value.");
        return NULL;
    }
    strncpy(token->value, value, l);
    token->value[l] = '\0';
    token->next = NULL;

    return token;
}

void printTokens(Token *head) {
    Token *cur = head;

    while (cur != NULL) {
        printf("<type: %d, value: %s>\n", cur->type, cur->value);
        cur = cur->next;
    }
}

// Returns:
//  0: not an builtin
// >0: length of builtin
int getBuiltinLength(char *c, char *builtins[]) {
    if (c == NULL) return 0;

    for (int i = 0; i < (int) (sizeof(builtins)/sizeof(builtins[0])); i++) {
        int j = 0;
        
        while (c[j] != '\0' && builtins[i][j] != '\0') {
            if (c[j] == builtins[i][j]) {
                j ++;
            }
            else if (j > 0) return j;
            else break;
        }

        if (j>0) return j;

    }

    return 0;
}

int getOperatorLength(char *c) {
    char *operators[] = {"<=", ">=", "int", "drv", "+", "-", "*", "/", "^", "=", "<", ">"};

    return getBuiltinLength(c, operators);

}

int getFunctionLength(char *c) {
    char *functions[] = {"sin", "cos", "tan", "csc", "sec", "cot"};

    return getBuiltinLength(c, functions);

}

Token *tokenize(char *buffer) {
    Token *head = NULL;
    Token *prev = NULL;

    int opl;

    int i = 0;
    while(buffer[i] != '\0') {
        int end = i;
        TokenType type = -1;

        printf("%c\n", buffer[i]);

        // Skip spaces
        if (isspace(buffer[i])) {
            i++;
            continue;
        }
        
        // Number can start with a - for a negative number
        else if (isdigit(buffer[i])) {
            type = TOKEN_NUMBER;
            while (isdigit(buffer[end])) end ++;
        }
        // Checks if operator and returns the length if it is
        else if ((opl = getOperatorLength(buffer + i))) {
            end += opl;
            type = TOKEN_OPERATOR;
        }

        else if (isalpha(buffer[i])) {
            type = TOKEN_IDENTIFIER;
            while (isalpha(buffer[end])) end ++;
        }

        else {
            printf("Unknown character: %c\n", buffer[i]);
            printf("%s\n", buffer);
            char *pointer[] = malloc((i + 1) * sizeof(char));
            if (pointer == NULL) {
                return NULL;
            }

            memset(pointer, ' ', i);
            pointer[i] = '^';

            printf("%s", pointer);
            return NULL;
        }

        printf("creating token\n");

        Token *newToken = createToken(type, buffer + i, end - i);

        if (prev != NULL) {
            prev-> next = newToken;
        } else {
            head = newToken;
        }
        prev = newToken;

        i = end;

    }

    return head;
}

void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}