#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"
#include "../builtins.h"


static const OperatorMapping DEFAULT_MAPPING[] = {
    {"+", TOKEN_OPERATOR},
    {"-", TOKEN_OPERATOR},
    {"*", TOKEN_OPERATOR},
    {"/", TOKEN_OPERATOR},
    {"^", TOKEN_OPERATOR},
    {"->", TOKEN_ASSIGNMENT},
    {"(", TOKEN_RIGHT_PAREN},
    {")", TOKEN_LEFT_PAREN},
    {",", TOKEN_SEPERATOR},
    {":", TOKEN_FUNC_DEF}
};

static const int N_MAPPINGS = 10;

/**
 * @brief Gets the length of an operator
 * 
 * @param c Buffer
 * @return int Length of found operator
 */
static int getOperatorLength(char *c) {
    if (isalnum(c[0])) return 0;
    int max = 0;
    for (int i = 0; i < N_MAPPINGS; i ++) {
        if (!strncmp(c, DEFAULT_MAPPING[i].op, strlen(DEFAULT_MAPPING[i].op))) {
            max = strlen(DEFAULT_MAPPING[i].op);
            printf("Match against #%d\n", i);
        }
    }

    return max;
}

/**
 * @brief Gets the length of number found
 * 
 * @param c Buffer
 * @return int Length of found number
 */
static int getNumber(char *c) {
    int i = 0;
    if (isdigit(c[0]) || c[0] == '.') {
        while (isdigit(c[i]) || c[i] == '.') i ++;
    }

    return i;
}

/**
 * @brief Get the length of identifiers by checking against the environment registry
 * 
 * @param c Buffer
 * @param env Environment
 * @return int Length of largest component found or length of contiuguous valid identifier characters
 */
static int getComponentLength(char *c, Environment *env) {
    // If not valid character type for variable name, automatically skip check
    if (!isalpha(c[0])) return 0;

    int i = 0;
    int max = 0;
    while (isalnum(c[i]) || c[i] == '_') {
        char temp = c[i + 1];
        
        // Adds the end of string char to only select a part of the buffer
        c[i + 1] = '\0';
        Component *cmp = searchEnvironment(env, c);
        c[i + 1] = temp;

        if (cmp != NULL) max = i + 1;

        i++;
    }

    if (max > 0) return max;
    return i;
}


Token *tokenize(char *buffer, Environment *env) {
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

        // Checks if operator and returns the length if it is
        else if ((matchLen = getOperatorLength(buffer + i))) {
            end += matchLen;
            type = TOKEN_OPERATOR;
        }

        // Number can start with a - for a negative number (not implemented yet)
        else if ((matchLen = getNumber(buffer + i))) {
            end += matchLen;
            type = TOKEN_NUMBER;
        }
        
        // Checks if builtin function and return the length if it is
        else if ((matchLen = getComponentLength(buffer + i, env))) {
            end += matchLen;

            char temp = buffer[end];
            buffer[end] = '\0';
            Component *cmp = searchEnvironment(env, buffer + i);
            buffer[end] = temp;

            if (cmp == NULL || cmp->type == VARIABLE) type = TOKEN_IDENTIFIER;
            else type = TOKEN_FUNC_CALL;
        }
        
        else {
            // Unknown character found, returns error
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

        // Prevents ambiguous syntax due to spaces
        if (spaceI != -1) {
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNC_CALL)) {
                printf("Spacing lead to ambiguous intent.\n");

                // Prints input and pointer to problematic char
                printf("%s\n", buffer);
                char *pointer = malloc((i) * sizeof(char));
                if (pointer == NULL) {
                    return NULL;
                }

                // Formatting error printout
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
            prev->next = newToken;
        } else {
            head = newToken;
        }
        prev = newToken;

        i = end;

    }

    return head;
}