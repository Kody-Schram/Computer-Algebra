#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"
#include "../builtins.h"

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
int getBuiltinLength(char *c, const char **builtins, int entries) {
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
    const char *operators[] = {"<=", ">=", "int", "drv", "+", "-", "*", "/", "^", "<", ">"};

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
    return getBuiltinLength(c, builtin_identifiers, nBuiltins);
}

/**
 * @brief Gets length of number found
 * 
 * @retval 0: No number found
 * @retval >0: Length of number found
 * 
 * @param c Start of string to check against
 * @return int Length of found number
 */
int getNumber(char *c) {
    int i = 0;
    if (isdigit(c[0]) || c[0] == '.') {
        while (isdigit(c[i]) || c[i] == '.') i ++;
    }

    return i;
}

/**
 * @brief Gets length of identifier found
 * 
 * @retval 0: No identifier found
 * @retval >0: Length of identifier found
 * 
 * @param c Start of  string to check against
 * @return int Length of found identifier
 */
int getIdentifier(char *c) {
    int i = 0;
    if (isalpha(c[0])) {
        while (c[i] == '_' || (isalnum(c[i]) && !getFunctionLength(c+i))) i ++;
    }
    return i;
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
        case '=':
            *type = TOKEN_ASSIGNMENT;
            break;
        default:
            return 0;
    }

    return 1;
}

Token *createToken(TokenType type, char *value, int l) {
    // Allocates new token
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

Token *tokenize(char *buffer) {
    Token *head = NULL;
    Token *prev = NULL;

    int matchLen;

    int spaceI = -1;
    TokenType prevT = -1;

    int i = 0;
    // Loops through till end of string
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
        else if ((matchLen = getNumber(buffer + i))) {
            end += matchLen;
            type = TOKEN_NUMBER;
        }
        // Checks if operator and returns the length if it is
        else if ((matchLen = getOperatorLength(buffer + i))) {
            end += matchLen;
            type = TOKEN_OPERATOR;
        }
        // Checks if builtin function and return the length if it is
        else if ((matchLen = getFunctionLength(buffer + i))) {
            end += matchLen;
            type = TOKEN_FUNC_CALL;
        }
        // Reads identifiers/variables
        else if ((matchLen = getIdentifier(buffer + i))) {
            end += matchLen;
            type = TOKEN_IDENTIFIER;
        }

        else {
            // Checks remaining symbols
            int l = getSimpleSymbols(buffer[i], &type);
            if (l > 0) {
                end ++;
            }
            // Unknown character found, returns error
            else {
                printf("Unknown character: %c\n", buffer[i]);

                // Prints input with pointer to unknown character
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
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNC_CALL)) {
                printf("Spacing lead to ambiguous intent.\n");

                // Pritns input and pointer to problematic char
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

        // Updates linked list
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