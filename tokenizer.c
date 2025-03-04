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
int getBuiltinLength(char *c, char **builtins, int entries) {
    if (c == NULL) return 0;

    for (int i = 0; i < entries; i++) {
        int j = 0;
        
        while (c[j] != '\0' && builtins[i][j] != '\0') {
            if (c[j] == builtins[i][j]) {
                j ++;
            }
            else break;
        }

        if (builtins[i][j] == '\0') return j;

    }

    return 0;
}

int getOperatorLength(char *c) {
    char *operators[] = {"<=", ">=", "int", "drv","**", "+", "-", "*", "/", "^", "=", "<", ">"};

    return getBuiltinLength(c, operators, sizeof(operators)/sizeof(operators[0]));

}

int getFunctionLength(char *c) {
    char *functions[] = {"sin", "cos", "tan", "csc", "sec", "cot", "log", "ln"};

    return getBuiltinLength(c, functions, sizeof(functions)/sizeof(functions[0]));

}

// Returns:
// 0: unknown character
// 1: valid symbol
int getSimpleSymbols(char c, TokenType *type) {
    switch(c) {
        case '(':
        case '{':
        case '[':
            *type = TOKEN_LEFT_BRACKET;
            break;
 
        case ')':
        case '}':
        case ']':
            *type = TOKEN_RIGHT_BRACKET;
            break;
        case ':':
            *type = TOKEN_FUNC_DEF;
            break;
        case ',':
            *type = TOKEN_SEPERATOR;
            break;
        default:
            return 0;
    }

    return 1;
}

Token *tokenize(char *buffer) {
    Token *head = NULL;
    Token *prev = NULL;

    int builtinln;

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
        else if ((builtinln = getOperatorLength(buffer + i))) {
            // Switches ** into ^ for exponents
            if (builtinln == 2) {
                if (buffer[i] == '*' && buffer[i+1] == '*') {
                    i ++;
                    buffer[i] = '^';
                }
            }

            end += builtinln;
            type = TOKEN_OPERATOR;
        }
        // Checks if builtin function and return the length if it is
        else if ((builtinln = getFunctionLength(buffer + i))) {
            end += builtinln;
            type = TOKEN_FUNCTION;
        }
        // Reads identifiers/variables
        else if (isalpha(buffer[i])) {
            type = TOKEN_IDENTIFIER;
            while (isalpha(buffer[end])) end ++;
        }

        else {
            int l = getSimpleSymbols(buffer[i], &type);
            if (l > 0) {
                end ++;
            } else {
                printf("Unknown character: %c\n", buffer[i]);
                printf("%s\n", buffer);

                char *pointer = malloc((i + 1) * sizeof(char));
                if (pointer == NULL) {
                    return NULL;
                }

                memset(pointer, ' ', i);
                pointer[i] = '^';
                printf("%s\n", pointer);

                return NULL;
            }
        }

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
