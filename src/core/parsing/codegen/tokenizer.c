#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"

#include "core/parsing/parser_types.h"
#include "core/parsing/parser_utils.h"

typedef struct  {
    char *op;
    TokenType type;
} DelimiterMapping;

static const DelimiterMapping DEFAULT_MAPPING[] = {
    {":", TOKEN_ASSIGNMENT},
    {"(", TOKEN_LEFT_PAREN},
    {")", TOKEN_RIGHT_PAREN},
    {",", TOKEN_SEPARATOR},
    {"->", TOKEN_MAPPING}
};

static const int N_MAPPINGS = 5;

typedef struct {
    int len;
    TokenType type;
} DelimiterReturn;

typedef struct {
    int len;
    Component *cmp;
} ComponentReturn;


static DelimiterReturn getDelimiter(const char *c) {
    DelimiterReturn result = {.type = TOKEN_SEPARATOR, .len=0};
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
static int getNumber(const char *c) {
    int i = 0;
    if (!isdigit(c[0]) && c[0] != '.') return 0;
	
	i = 1;
	while (isdigit(c[i]) || c[i] == '.') i ++;

    return i;
}


static ComponentReturn getOperatorComponent(const char *c) {
    Debug(0, "Getting operator component, '%c'\n", (char) c[0]);
    ComponentReturn result = {.len = 0, .cmp = NULL};
    if (isalnum(c[0])) return result;
    
    Debug(0, "getting cmp\n");
    
    Component *cmp = searchEnvironmentOperator(GLOBALCONTEXT->env, (char) c[0]);
    if (cmp == NULL) return result;
    
    result.len = 1;
    result.cmp = cmp;
    return result;
}


/**
 * @brief Get the length of identifiers by checking against the environment registry
 * 
 * @param c Buffer
 * @param env Environment
 * @return int Length of largest component found or length of contiuguous valid identifier characters
 */
static ComponentReturn getComponent(char *c) {
    Debug(0, "Checking component.\n");
    ComponentReturn result = {0, NULL};
    
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

        if (cmp != NULL) {
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

        if (cmp != NULL && result.len < length - i) {
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

            if (cmp != NULL && result.len < end - i) {
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

    Token *head = NULL;
    Token *prev = NULL;

    int matchLen;
    DelimiterReturn delim;
    ComponentReturn cmp;

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
        
        // Number can start with a - for a negative number (not implemented yet)
        else if ((matchLen = getNumber(buffer + i))) {
            end += matchLen;
            type = TOKEN_NUMBER;
        }

        // Checks if operator
        else if ((delim = getDelimiter(buffer + i)).len) {
            end += delim.len;
            type = delim.type;
        }
        
        else if ((cmp = getOperatorComponent(buffer + i)).len) {
            end += cmp.len;
            type = TOKEN_OPERATOR;
        }
        
        // Checks if builtin function and return the length if it is
        else if ((cmp = getComponent(buffer + i)).len) {
            //if (cRet.cmp != NULL) printf("largest component found was %s\n", cRet.cmp->identifier);
            end += cmp.len;

            if (cmp.cmp == NULL || cmp.cmp->type == COMP_VARIABLE) type = TOKEN_IDENTIFIER;
            else type = TOKEN_FUNC_CALL_PLACEHOLDER;
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
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNC_CALL_PLACEHOLDER)) {
                printf("Spacing leads to ambiguous intent.\n");
                return NULL;
            }
        }

        spaceI = -1;
        prevT = type;

        Token *newToken;
		if (type == TOKEN_OPERATOR) newToken = createTokenOperator(cmp.cmp->operation);
		else newToken = createToken(type, buffer + i, end - i);

		Debug(0, "type = %d\n", newToken->type);
		if (newToken == NULL) {
            perror("Error in tokenizer");
            return NULL;
        }

		Debug(0, "updating list\n");
        // Updates linked list
        if (prev != NULL) {
            prev->next = newToken;
        } else {
			Debug(0, "setting head token\n");
            head = newToken;
        }
        prev = newToken;

        i = end;
		Debug(1, printTokens(head));

    }
	Debug(0, "finished tokenizing\n");
    Debug(1, printTokens(head));
    
    return head;
}
