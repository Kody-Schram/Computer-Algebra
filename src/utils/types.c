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
    if (node == NULL) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) {
        fprintf(stream, "  ");
        //printf("  ");
    }

    switch(node->type) {
        case NODE_NUMBER:
            fprintf(stream, "<type: NUMBER, value: %f>\n", node->value);
            //printf("<type: NUMBER, value: %f>\n", node->value);
            break;

        case NODE_OPERATOR:
            //printf("operator\n");
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
                default:
                    printf("How'd we get here?\n");
                    id = '?';
            }
            fprintf(stream, "<type: OPERATOR, symbol: %c>\n", id);
            printASTRec(node->left, level + 1, stream);
            printASTRec(node->right, level + 1, stream);
            //printf("<type: OPERATOR, symbol: %c>\n", id);
            break;

        case NODE_VARIABLE:
            fprintf(stream, "<type: VARIABLE, identifier: '%s'>\n", node->identifier);
            //printf("<type: VARIABLE, identifier: '%s'>\n", node->identifier);
            break;

        case NODE_FUNC_CALL: 
            printf("printing call\n");
            if (node->call == NULL) {
                Debug(0, "Function call was null\n");
                return;
            }
            if (node->call->identifier == NULL) {
                Debug(0, "Call identifer was null\n");
                return;
            }
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", node->call->identifier);
            //printf("<type: FUNC_CALL, value: '%s'>\n", node->call->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            printf("printing parameters\n");
            //printf("parameters\n");
            fprintf(stream, "Parameters:\n");
            // //printf("Parameters:\n");
            for (int p = 0; p < node->call->nParams; p ++) printASTRec(node->call->parameters[p], level + 1, stream);
            //printf("finished printing call\n");
            break;

        case NODE_ASSIGN_FUNC:
            fprintf(stream, "<type ASSIGN_FUNC '%s'>\n", (char *) node->left);
            // printf("<type ASSIGN_FUNC '%s'>\n", (char *) node->left);
            // printf("Definition:\n");
            fprintf(stream, "Definition:\n");
            printASTRec(node->func->definition, level + 1, stream);
            break;

        default:
            fprintf(stream, "no %d\n", node->type);
    }
}


FILE *printAST(ASTNode *root) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    printASTRec(root, 0, stream);
    return stream;
}


void freeAST(ASTNode *ast) {
    if (ast == NULL) return;
    Debug(0, "Freeing ast\n");

    freeAST(ast->left);
    freeAST(ast->right);

    switch (ast->type) {
        case NODE_ASSIGN_FUNC:
            Debug(0, "Free Func Assign\n");
            free(ast->left);
            free(ast);
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