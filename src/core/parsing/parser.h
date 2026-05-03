#ifndef PARSER_H
#define PARSER_H

#include "core/primitives/types.h"
#include "parser_types.h"


typedef enum PARSER_RESULT {
    PARSER_SUCCESS,
	PARSER_SYNTAX_ERROR,
    PARSER_ERROR
} PARSER_RESULT;


/**
 * @brief Handles entire parsing process from string to AST
 * 
 * @retval NULL: Error parsing the string
 * @retval Token*: Properly parsed the string
 * 
 * @param buffer Mathematical string
 * @return Token* 
 */
PARSER_RESULT parse(char *buffer, Expression **expr);

#endif
