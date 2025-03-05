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
    memcpy(token->value, value, l);
    token->value[l] = '\0';
    token->next = NULL;

    return token;
}

/**
 * @brief Get the Builtin Length object
 * 
 * @retval 0: No match found
 * @retval >0: Length of match found
 * 
 * @param c Start of string to check against
 * @param builtins List of strings
 * @param entries Number of entries in the list
 * @return int 
 */
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

/**
 * @brief Gets the length of any operators found
 * 
 * @retval 0: No operator found
 * @retval >0: Length of operator found
 * 
 * @param c Start of string to check against
 * @return int 
 */
int getOperatorLength(char *c) {
    char *operators[] = {"<=", ">=", "int", "drv", "+", "-", "*", "/", "^", "=", "<", ">"};

    return getBuiltinLength(c, operators, sizeof(operators)/sizeof(operators[0]));

}

/**
 * @brief Gets the length any builtin function found
 * 
 * @retval 0: No builtin function found
 * @retval >0: Length of builtin function found
 * 
 * @param c Start of string to check against
 * @return int 
 */
int getFunctionLength(char *c) {
    char *functions[] = {"sin", "cos", "tan", "csc", "sec", "cot", "log", "ln"};

    return getBuiltinLength(c, functions, sizeof(functions)/sizeof(functions[0]));

}

/**
 * @brief Check a few single character symbols
 * 
 * @retval 0: No symbol recognized
 * @retval 1: Properly found symbol
 * 
 * @param c Character being checked
 * @param type Pointer to type variable in tokenizer
 * @return int 
 */
int getSimpleSymbols(char c, TokenType *type) {
    switch(c) {
        case '(':
            *type = TOKEN_LEFT_PAREN;
            break;
 
        case ')':
            *type = TOKEN_RIGHT_PAREN;
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

    int spaceI = -1;
    TokenType prevT = -1;

    int i = 0;
    while(buffer[i] != '\0') {
        int end = i;
        TokenType type = -1;

        // Skip spaces
        if (isspace(buffer[i])) {
            spaceI = i;
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
            while (buffer[end] == '_' || (isalnum(buffer[end]) && !getFunctionLength(buffer+end))) end ++;
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

        // Prevents ambiguous syntax due to spaces
        if (spaceI != -1) {
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNCTION)) {
                printf("Spacing lead to ambiguous intent.\n");
                printf("%s\n", buffer);

                char *pointer = malloc((i) * sizeof(char));
                if (pointer == NULL) {
                    return NULL;
                }

                memset(pointer, ' ', i-1);
                pointer[i-1] = '^';
                printf("%s\n", pointer);

                return NULL;
            }
        }

        spaceI = -1;
        prevT = type;

        Token *newToken = createToken(type, buffer + i, end - i);
        if (newToken == NULL) return NULL;

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