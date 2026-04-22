#ifndef PARSER_H
#define PARSER_H

#include "core/utils/types.h"
#include "parser_types.h"

typedef enum ParserResultType ParserResultType;
typedef struct ParserResult ParserResult;


enum ParserResultType {
    PARSER_SUCCESS,
    PARSER_ERROR
};


struct ParserResult {
    ParserResultType type;
    Expression *expr;
};


int parseFunction(Token *head);

/**
 * @brief Handles entire parsing process from string to AST
 * 
 * @retval NULL: Error parsing the string
 * @retval Token*: Properly parsed the string
 * 
 * @param buffer Mathematical string
 * @return Token* 
 */
ParserResult parse(char *buffer);

#endif