#ifndef AST_H
#define AST_H

#include "core/primitives/types.h"
#include "core/parsing/parser_types.h"


/**
 * @brief Generates intermediate data type, RPN, for use in AST generation
 * 
 * @param head Start of Token linked list
 * @return RPNList* 
 */
RPNList *shuntingYard(Token *head);

/**
 * @brief Generates AST from RPN list
 * 
 * @param rpn 
 * @return Expression* Root of AST
 */
Expression *expressionFromRPN(RPNList *rpn);

#endif