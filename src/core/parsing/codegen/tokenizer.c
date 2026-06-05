#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"
#include "core/common.h"
#include "core/context.h"
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


#define N_MAPPINGS 5


typedef struct {
    int len;
    TokenType type;
} DelimiterReturn;


typedef struct {
    int len;
    Component *cmp;
} ComponentReturn;


static DelimiterReturn getDelimiter(char const *c) {
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
static int getNumber(char const *c) {
    int i = 0;
    if (!isdigit(c[0]) && c[0] != '.') return 0;
	
	i = 1;
	while (isdigit(c[i]) || c[i] == '.') i ++;

    return i;
}


static inline Operation const *getOperator(char const *c) {
    if (isalnum(c[0])) return NULL;
    
    Operation const *op = searchOperation(GLOBALCONTEXT->registry, (char) c[0]);
    if (op == NULL) return NULL;
    
    return op;
}


/**
 * @brief Get the length of identifiers by checking against the environment registry
 * 
 * @param c Buffer
 * @param env Environment
 * @return int Length of largest component found or length of contiuguous valid identifier characters
 */
static uint32_t getComponent(char *c, Component const **out) {
    // If not valid character type for variable name, automatically skip check
    if (!isalpha(c[0])) return 0;
    Environment *env = GLOBALCONTEXT->env;

    // Gets length of valid identifer characters
	uint32_t result = 0;
    uint32_t length = 0;
    while (isalnum(c[length]) || c[length] == '_') length ++;

    // Check left
    for (uint32_t i = 0; i < length; i ++) {
        char temp = c[length - i];
        c[length - i] = '\0';
        Component const *cmp = searchEnvironment(env, c);
        c[length - i] = temp;

        if (cmp != NULL) {
			Debug(0, "found left cmp %s\n", cmp->identifier);
			*out = cmp;
			result = length - i;
        }
    }

	if (result == length) return result;

    // Check right
    for (uint32_t i = 1; i < length; i ++) {
        char temp = c[length];
        c[length] = '\0';
        Component const *cmp = searchEnvironment(env, c + i);
        c[length] = temp;

        if (cmp != NULL && result < length - i) {
			Debug(0, "found right cmp %s\n", cmp->identifier);
			*out = cmp;
            result = i;
        }
    }

    for (int i = 1; i < length; i ++) {
        for (int end = i + 1; end < length; end ++) {
            char temp = c[end];
            c[end] = '\0';
            Component const *cmp = searchEnvironment(env, c + i);
            c[end] = temp;

            if (cmp != NULL && result < end - i) {
				Debug(0, "found center cmp %s\n", cmp->identifier);
				*out = NULL;
                result = i;
            }
        }
    }

    if (result == 0) result = length;
    return result;
}


Token *tokenize(char *buffer) {
    Config *config = GLOBALCONTEXT->config;

    Debug(0, "\nTokenizing '%s'\n", buffer);

    Token *head = NULL;
    Token *prev = NULL;

    int matchLen;
    DelimiterReturn delim;
    Component const *cmp = NULL;
	Operation const *op = NULL;

    int spaceI = -1;
    TokenType prevT = -1;

	bool hasCalls = false;
	bool hasVarAssignment = false;
	bool hasFuncAssignment = false;

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
        
        else if ((op = getOperator(buffer + i)) != NULL) {
            end ++;
            type = TOKEN_OPERATOR;
        }
        
        // Checks if builtin function and return the length if it is
        else if ((matchLen = getComponent(buffer + i, &cmp))) {
            //if (cRet.cmp != NULL) printf("largest component found was %s\n", cRet.cmp->identifier);
            end += matchLen;

            if (cmp == NULL || cmp->type == COMP_VARIABLE) {
				type = TOKEN_IDENTIFIER;
			}
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

			deepFreeTokens(head);
            return NULL;
        }

        // Prevents ambiguous syntax due to spaces
        if (spaceI != -1) {
            if ((prevT == TOKEN_NUMBER || prevT == TOKEN_IDENTIFIER) && (type == TOKEN_IDENTIFIER || type == TOKEN_NUMBER || type == TOKEN_FUNC_CALL_PLACEHOLDER)) {
                printf("Spacing leads to ambiguous intent.\n");
				deepFreeTokens(head);
                return NULL;
            }
        }

        spaceI = -1;
        prevT = type;

        Token *newToken;
		if (type == TOKEN_OPERATOR) newToken = createOperatorToken(op);
		else if (type == TOKEN_FUNC_CALL_PLACEHOLDER) {
			newToken = createFuncCallToken(cmp);
			hasCalls = true;
		}
		else newToken = createToken(type, buffer + i, end - i);

		if (newToken->type == TOKEN_ASSIGNMENT) {
			if (hasVarAssignment || hasFuncAssignment) {
				printf("Cannot have an assignment within an assignment\n");
				deepFreeTokens(head);
				return NULL;
			}
			hasVarAssignment = true;
		} else if (newToken->type == TOKEN_MAPPING) {
			if (hasFuncAssignment) {
				printf("Cannot have an assignment within an assignment\n");
				deepFreeTokens(head);
				return NULL;
			}
			hasFuncAssignment = true;
		}

		cmp = NULL;

		if (newToken == NULL) {
            perror("Error in tokenizer");
			deepFreeTokens(head);
            return NULL;
        }

        // Updates linked list
        if (prev != NULL) {
            prev->next = newToken;
        } else {
            head = newToken;
        }
        prev = newToken;

        i = end;
    }
	Debug(1, printTokens(head));

    head->hasFuncAssignment = hasFuncAssignment;
	head->hasVarAssignment = hasVarAssignment;
	head->hasFunctionCalls = hasCalls;
	return head;
}
