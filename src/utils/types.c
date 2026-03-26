#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "utils/context/context.h"
#include "utils/log.h"


static const int DEFUALT_STRING_SIZE = 64;


ASTNode *dummyASTNode(NodeType type) {
    ASTNode *node = calloc(1, sizeof(ASTNode));

    if (node == NULL) {
        printf("Error allocating for new node.\n");
        return NULL;
    }

    node->type = type;

    return node;
}


static ASTNode *deepCopyASTRecur(ASTNode *ast) {
    ASTNode *new = dummyASTNode(ast->type);
    if (ast == NULL || new == NULL) return NULL;

    switch (new->type) {
        case NODE_VARIABLE:
            new->identifier = strdup(ast->identifier);
            if (new->identifier == NULL) {
                printf("Error copying identifier.\n");
                goto error;
            }
            break;

        case NODE_NUMBER:
            new->value = ast->value;
            break;
        
        case NODE_OPERATOR:
            new->op = ast->op;
            new->left = deepCopyASTRecur(ast->left);
            new->right = deepCopyASTRecur(ast->right);
            
            if (new->left == NULL || new->right == NULL) goto error;
            break;

        case NODE_FUNC_CALL:
            FunctionCall *call = malloc(sizeof(FunctionCall));
            if (call == NULL) {
                printf("Error allocating memory for function call.\n");
                goto error;
            }
            call->identifier = strdup(ast->call->identifier);
            if (call->identifier == NULL) {
                printf("Error copying call identifier.\n");
                goto error;
            }
            call->nParams = ast->call->nParams;
            
            call->parameters = malloc(sizeof(ASTNode *) * call->nParams);
            if (call->parameters == NULL) {
                free(call->identifier);
                free(call);
                
                goto error;
            }

            for (int i = 0; i < call->nParams; i ++) {
                call->parameters[i] = deepCopyAST(ast->call->parameters[i]);
                if (call->parameters[i] == NULL) {
                    free(call->identifier);
                    free(call->parameters);
                    free(call);

                    goto error;
                }
            }

            new->call = call;
            break;

    }

    return new;

    error:
        free(new);
        return NULL;
}


ASTNode *deepCopyAST(ASTNode *ast) {
    if (ast == NULL) return NULL;

    return deepCopyASTRecur(ast);
}


static void printASTRec(ASTNode *node, int level, FILE *stream) {
    if (node == NULL || stream == NULL) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) fprintf(stream, "  ");

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
                default:
                    printf("How'd we get here?\n");
                    id = '?';
            }
            fprintf(stream, "<type: OPERATOR, symbol: %c>\n", id);
            printASTRec(node->left, level + 1, stream);
            printASTRec(node->right, level + 1, stream);
            break;

        case NODE_VARIABLE:
            fprintf(stream, "<type: VARIABLE, identifier: '%s'>\n", node->identifier);
            break;

        case NODE_FUNC_CALL: 
            if (node->call == NULL) {
                Debug(0, "Function call was null\n");
                return;
            }
            if (node->call->identifier == NULL) {
                Debug(0, "Call identifer was null\n");
                return;
            }
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", node->call->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            fprintf(stream, "Parameters:\n");
            for (int p = 0; p < node->call->nParams; p ++) printASTRec(node->call->parameters[p], level + 1, stream);
            break;

        case NODE_ASSIGN_FUNC:
            fprintf(stream, "<type ASSIGN_FUNC '%s'>\n", node->left->identifier);
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
    Debug(0, "\nRunning deep free of ast\n");
    Debug(1, printAST(ast));

    switch (ast->type) {
        case NODE_ASSIGN_FUNC:
            Debug(0, "Free Func Assign\n");
            free(ast->left);
            break;

        case NODE_FUNC_CALL:
            Debug(0, "Free Func Call\n");
            if (ast->call != NULL) {
                free(ast->call->identifier);
                for (int i = 0; i < ast->call->nParams; i ++) {
                    freeAST(ast->call->parameters[i]);
                }
                free(ast->call->parameters);
            }
            free(ast->call);
            break;

        case NODE_VARIABLE:
            Debug(0, "Free Variable '%s'\n", ast->identifier);
            free(ast->identifier);
            break;

        case NODE_NUMBER:
            Debug(0, "Free number %f\n", ast->value);
            break;

        case NODE_OPERATOR:
            Debug(0, "Free operator\n");
            freeAST(ast->left);
            freeAST(ast->right);
            break;

        default:
            Debug(0, "Free Default\n");
            break;
    }

    free(ast);
}


static void astToStringRecur(ASTNode *ast, FILE *stream) {
    if (ast == NULL) return;

    switch (ast->type) {
        case NODE_OPERATOR:
            if (ast->left != NULL && ast->left->type == NODE_OPERATOR) {
                fprintf(stream, "(");
                astToStringRecur(ast->left, stream);
                fprintf(stream, ")");
            } else {
                astToStringRecur(ast->left, stream);
            }

            switch (ast->op) {
                case OP_ADDITION:
                    fprintf(stream, " + ");
                    break;
                case OP_SUBTRACTION:
                    fprintf(stream, " - ");
                    break;
                case OP_MULTIPLICATION:
                    fprintf(stream, " * ");
                    break;
                case OP_DIVISION:
                    fprintf(stream, " / ");
                    break;
                case OP_EXPONTENTIATION:
                    fprintf(stream, " ^ ");
                    break;
                default:
                    printf("How'd we get here?\n");
            }

            if (ast->right != NULL && ast->right->type == NODE_OPERATOR) {
                fprintf(stream, "(");
                astToStringRecur(ast->right, stream);
                fprintf(stream, ")");
            } else {
                astToStringRecur(ast->right, stream);
            }
            break;
        case NODE_NUMBER:
            fprintf(stream, "%f", ast->value);
            break;
        case NODE_VARIABLE:
            fprintf(stream, "%s", ast->identifier);
            break;
    }
}


char *astToString(ASTNode *ast) {
    FILE *stream = tmpfile();
    char *string = NULL;
    if (ast == NULL || stream == NULL) {
        fclose(stream);
        return string;
    }
    astToStringRecur(ast, stream);

    long size = ftell(stream);
    if (size < 0) {
        fclose(stream);
        return NULL;
    }

    string = malloc(size + 1);
    if (string == NULL) {
        fclose(stream);
        return NULL;
    }

    rewind(stream);

    size_t ptr = fread(string, 1, size, stream);
    string[ptr] = '\0';

    fclose(stream);
    return string;
}