#ifndef AST_H
#define AST_H

#include "../utils/types.h"
#include "./parserTypes.h"


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
 * @return ASTNode* Root of AST
 */
ASTNode *astFromRPN(RPNList *rpn);

#endif