#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "utils/context/context.h"
#include "utils/log.h"


ASTNode *dummyASTNode(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        printf("Error allocating for new node.\n");
        return NULL;
    }

    node->type = type;
    node->left = NULL;
    node->right = NULL;

    return node;
}


static void printASTRec(ASTNode *node, int level, FILE *stream) {
    if (node == NULL || stream == NULL) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) fprintf(stream, "  ");

    //Debug(0, "switch\n");
    // Print node info
    switch(node->type) {
        case NODE_NUMBER:
            fprintf(stream, "<type: NUMBER, value: %f>\n", node->value);
            break;

        case NODE_OPERATOR:
            char id;
            switch (node->op) {
                case OP_ADDITION:
                    id = '+';
                    break;
                case OP_SUBTRACTION:
                    id = '-';
                    break;
                case OP_MULTIPLICATION:
                    id = '*';
                    break;
                case OP_DIVISION:
                    id = '/';
                    break;
                case OP_EXPONTENTIATION:
                    id = '^';
                    break;
            }
            fprintf(stream, "<type: OPERATOR, symbol: %c>\n", id);
            break;

        case NODE_VARIABLE:
            fprintf(stream, "<type: VARIABLE, identifier: '%s'>\n", node->identifier);
            break;

        case NODE_FUNC_CALL: 
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", node->call->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            fprintf(stream, "Parameters:\n");
            for (int p = 0; p < node->call->nParams; p ++) printASTRec(node->call->parameters[p], level + 1, stream);
            break;

        case NODE_ASSIGN_FUNC:
            fprintf(stream, "<type ASSIGN_FUNC>\n");
            if (node->func != NULL) printASTRec(node->func->definition, level + 1, stream);
            break;

        default:
            fprintf(stream, "no %d\n", node->type);
    }

    // Recursively print children
    printASTRec(node->left, level + 1, stream);
    printASTRec(node->right, level + 1, stream);
}


FILE *printAST(ASTNode *root) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    //Debug(0, "printing ast recursively\n");

    printASTRec(root, 0, stream);
    return stream;
}


void freeAST(ASTNode *ast) {
    if (ast == NULL) return;

    freeAST(ast->left);
    freeAST(ast->right);

    switch (ast->type) {
        case NODE_ASSIGN_FUNC:
            Debug(0, "Free Func Assign\n");
            if (ast->func != NULL) free(ast);
            break;

        case NODE_FUNC_CALL:
            Debug(0, "Free Func Call '%s'\n");
            free(ast->call->identifier);
            free(ast->call->parameters);
            free(ast->call);

            free(ast);
            break;

        case NODE_VARIABLE:
            Debug(0, "Free Variable '%s'\n", ast->identifier);
            free(ast->identifier);
            free(ast);
            break;

        case NODE_NUMBER:
            Debug(0, "Free number %f\n", ast->value);
            free(ast);
            break;

        case NODE_OPERATOR:
            Debug(0, "Free operator\n");
            free(ast);
            break;

        default:
            Debug(0, "Free Default\n");
            free(ast);
            break;
    }
}