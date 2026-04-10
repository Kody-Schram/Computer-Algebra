#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"
#include "core/utils/context/context.h"
#include "core/utils/log.h"
#include "parsing/parserUtils.h"

typedef struct  {
    char *op;
    TokenType type;
} OperatorMapping;

static const OperatorMapping DEFAULT_MAPPING[] = {
    {"+", TOKEN_OPERATOR},
    {"-", TOKEN_OPERATOR},
    {"*", TOKEN_OPERATOR},
    {"/", TOKEN_OPERATOR},
    {"^", TOKEN_OPERATOR},
    {":", TOKEN_ASSIGNMENT},
    {"(", TOKEN_LEFT_PAREN},
    {")", TOKEN_RIGHT_PAREN},
    {",", TOKEN_SEPARATOR},
    {"->", TOKEN_MAPPING}
};

static const int N_MAPPINGS = 10;

typedef struct {
    int len;
    TokenType type;
} SymbolReturn;

typedef struct {
    int len;
    Component *cmp;
} ComponentReturn;

/**
 * @brief Gets the length of an operator
 * 
 * @param c Buffer
 * @return int Length of found operator
 */
static SymbolReturn getSymbolLength(char *c) {
    SymbolReturn result = {0, TOKEN_OPERATOR};
    if (isalnum(c[0])) return result;
    for (int i = 0; i < N_MAPPINGS; i ++) {
        int len = strlen(DEFAULT_MAPPING[i].op);
        if (!strncmp(c, DEFAULT_MAPPING[i].op, len)) {
            if (len < result.len) continue;

            result.len = len;
            result.type = DEFAULT_MAPPING[i].type;
        }
    }

    return result;
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
static ComponentReturn getComponentLength(char *c) {
    Debug(0, "Checking component.\n");
    ComponentReturn result = {0, nullptr};
    // If not valid character type for variable name, automatically skip check
    if (!isalpha(c[0])) return result;
    Environment *env = GLOBALCONTEXT->env;

    // Gets length of valid identifer characters
    int length = 0;
    while (isalnum(c[length]) || c[length] == '_') length ++;

    // Check left
    for (int i = 0; i < length; i ++) {
        char temp = c[length - i];
        c[length - i] = '\0';
        Component *cmp = searchEnvironment(env, c);
        c[length - i] = temp;

        if (cmp != nullptr) {
            Debug(0, "found left side component: %s\n", cmp->identifier);
            result.len = length - i;
            result.cmp = cmp;
        }
    }

    if (result.len == length) return result;
    
    // Check right
    for (int i = 1; i < length; i ++) {
        char temp = c[length];
        c[length] = '\0';
        Component *cmp = searchEnvironment(env, c + i);
        c[length] = temp;

        if (cmp != nullptr && result.len < length - i) {
            Debug(0, "found right side component: %s, %d characters in.\n", cmp->identifier, i);
            result.len = i;
        }
    }

    if (result.len == length) return result;

    for (int i = 1; i < length; i ++) {
        for (int end = i + 1; end < length; end ++) {
            char temp = c[end];
            c[end] = '\0';
            Component *cmp = searchEnvironment(env, c + i);
            c[end] = temp;

            if (cmp != nullptr && result.len < end - i) {
                Debug(0, "found nested component: %s\n", cmp->identifier);
                result.len = i;
                return result;
            }
        }
    }

    if (result.len == 0) result.len = length;
    return result;
}


Token *tokenize(char *buffer) {
    Config *config = GLOBALCONTEXT->config;

    Debug(0, "\nTokenizing '%s'\n", buffer);

    Token *head = nullptr;
    Token *prev = nullptr;

    int matchLen;
    SymbolReturn ret;
    ComponentReturn cRet;

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
        else if ((ret = getSymbolLength(buffer + i)).len) {
            end += ret.len;
            type = ret.type;
        }

        // Number can start with a - for a negative number (not implemented yet)
        else if ((matchLen = getNumber(buffer + i))) {
            end += matchLen;
            type = TOKEN_NUMBER;
        }
        
        // Checks if builtin function and return the length if it is
        else if ((cRet = getComponentLength(buffer + i)).len) {
            //if (cRet.cmp != nullptr) printf("largest component found was %s\n", cRet.cmp->identifier);
            end += cRet.len;

            if (cRet.cmp == nullptr || cRet.cmp->type == VARIABLE) type = TOKEN_IDENTIFIER;
            else type = TOKEN_FUNC_CALL_PLACEHOLDER;
        }
        
        else {
            // Unknown character found, returns error
            printf("Unknown character: %c\n", buffer[i]);

            // Prints input with pointer to unknown character
            printf("%s\n", buffer);
            char *pointer = malloc((i + 1) * sizeof(char));
            if (pointer == nullptr) {
                return nullptr;
            }

            memset(pointer, ' ', i);
            pointer[i] = '^';
            printf("%s\n", pointer);

            return nullptr;
        }

        // Prevents ambiguous syntax due to spaces
        if (spaceI != -1) {
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNC_CALL_PLACEHOLDER)) {
                printf("Spacing lead to ambiguous intent.\n");
                return nullptr;
            }
        }

        spaceI = -1;
        prevT = type;

        Token *newToken = createToken(type, buffer + i, end - i);
        if (newToken == nullptr) {
            perror("Error in tokenizer");
            return nullptr;
        }

        // Updates linked list
        if (prev != nullptr) {
            prev->next = newToken;
        } else {
            head = newToken;
        }
        prev = newToken;

        i = end;

    }

    Debug(1, printTokens(head));
    
    return head;
}