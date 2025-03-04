#ifndef AST_H
#define AST_H

typedef enum Node_Type {
    NODE_NUMBER,
    NODE_OPERATOR,
    NODE_VARIABLE,
    NODE_FUNC_CALL,
    NODE_FUNC_DEF
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char *value;

    ASTNode *left;
    ASTNode *right;
} ASTNode;

#endif