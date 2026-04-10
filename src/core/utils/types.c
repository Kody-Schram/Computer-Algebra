#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/utils/log.h"


static const int DEFUALT_STRING_SIZE = 64;


ASTNode *dummyASTNode(NodeType type) {
    ASTNode *node = calloc(1, sizeof(ASTNode));

    if (node == nullptr) {
        printf("Error allocating for new node.\n");
        return nullptr;
    }

    node->type = type;

    return node;
}


static ASTNode *deepCopyASTRecur(ASTNode *ast) {
    ASTNode *new = dummyASTNode(ast->type);
    if (ast == nullptr || new == nullptr) return nullptr;

    switch (new->type) {
        case NODE_VARIABLE:
            new->identifier = strdup(ast->identifier);
            if (new->identifier == nullptr) {
                printf("Error copying identifier.\n");
                goto error;
            }
            break;

        case NODE_INTEGER:
            new->integer = ast->integer;
            break;

        case NODE_DOUBLE:
            new->value = ast->value;
            break;
        
        case NODE_OPERATOR:
            new->op = ast->op;
            new->left = deepCopyASTRecur(ast->left);
            new->right = deepCopyASTRecur(ast->right);
            
            if (new->left == nullptr || new->right == nullptr) goto error;
            break;

        case NODE_FUNC_CALL:
            FunctionCall *call = malloc(sizeof(FunctionCall));
            if (call == nullptr) {
                printf("Error allocating memory for function call.\n");
                goto error;
            }
            call->identifier = strdup(ast->call->identifier);
            if (call->identifier == nullptr) {
                printf("Error copying call identifier.\n");
                goto error;
            }
            call->nParams = ast->call->nParams;
            
            call->parameters = malloc(sizeof(ASTNode *) * call->nParams);
            if (call->parameters == nullptr) {
                free(call->identifier);
                free(call);
                
                goto error;
            }

            for (int i = 0; i < call->nParams; i ++) {
                call->parameters[i] = deepCopyAST(ast->call->parameters[i]);
                if (call->parameters[i] == nullptr) {
                    free(call->identifier);
                    free(call->parameters);
                    free(call);

                    goto error;
                }
            }

            new->call = call;
            break;

        default:
            break;
            
    }

    return new;

    error:
        free(new);
        return nullptr;
}


ASTNode *deepCopyAST(ASTNode *ast) {
    if (ast == nullptr) return nullptr;

    return deepCopyASTRecur(ast);
}


static void printASTRec(ASTNode *node, int level, FILE *stream) {
    if (node == nullptr || stream == nullptr) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) fprintf(stream, "  ");

    switch(node->type) {
        case NODE_INTEGER:
            fprintf(stream, "<type: INTEGER, value: %lld>\n", node->integer);
            break;

        case NODE_DOUBLE:
            fprintf(stream, "<type: DOUBLE, value: %g>\n", node->value);
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
            if (node->call == nullptr) {
                Debug(0, "Function call was nullptr\n");
                return;
            }
            if (node->call->identifier == nullptr) {
                Debug(0, "Call identifer was nullptr\n");
                return;
            }
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", node->call->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            fprintf(stream, "Parameters:\n");
            for (int p = 0; p < node->call->nParams; p ++) printASTRec(node->call->parameters[p], level + 1, stream);
            break;

        case NODE_ASSIGN_FUNC:
            fprintf(stream, "<type ASSIGN_FUNC '%s'>\n", node->left->identifier);
            fprintf(stream, "Definition:\n");
            printASTRec(node->func->definition, level + 1, stream);
            break;

        case NODE_ASSIGN_VAR:
            fprintf(stream, "<type ASSIGN_VAR '%s'>\n", node->left->identifier);
            fprintf(stream, "Definition:\n");
            printASTRec(node->right, level + 1, stream);
            break;

        default:
            fprintf(stream, "no %d\n", node->type);
    }
}


FILE *printAST(ASTNode *root) {
    FILE *stream = tmpfile();
    if (stream == nullptr) return nullptr;

    printASTRec(root, 0, stream);
    return stream;
}


void freeAST(ASTNode *ast) {
    if (ast == nullptr) return;

    switch (ast->type) {
        case NODE_ASSIGN_VAR:
            if (ast->left != nullptr) free(ast->left->identifier);
            free(ast->left);
            freeAST(ast->right);
            free(ast);
            break;

        case NODE_ASSIGN_FUNC:
            if (ast->left != nullptr) free(ast->left->identifier);
            free(ast->left);
            if (ast->func != nullptr) {
                freeEnvironment(ast->func->env);
                if (ast->func->type == DEFINED) freeAST(ast->func->definition);
            }
            free(ast->func);
            break; 

        case NODE_FUNC_CALL:
            if (ast->call != nullptr) {
                free(ast->call->identifier);
                for (int i = 0; i < ast->call->nParams; i ++) {
                    freeAST(ast->call->parameters[i]);
                }
                free(ast->call->parameters);
            }
            free(ast->call);
            break;

        case NODE_VARIABLE:
            free(ast->identifier);
            break;

        case NODE_OPERATOR:
            freeAST(ast->left);
            freeAST(ast->right);
            break;
            
        default:
            break;
        
    }

    free(ast);
}


static void astToStringRecur(ASTNode *ast, FILE *stream) {
    if (ast == nullptr) return;

    switch (ast->type) {
        case NODE_OPERATOR:
            if (ast->left != nullptr && ast->left->type == NODE_OPERATOR) {
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

            if (ast->right != nullptr && ast->right->type == NODE_OPERATOR) {
                fprintf(stream, "(");
                astToStringRecur(ast->right, stream);
                fprintf(stream, ")");
            } else {
                astToStringRecur(ast->right, stream);
            }
            break;

        case NODE_DOUBLE:
            fprintf(stream, "%g", ast->value);
            break;

        case NODE_INTEGER:
            fprintf(stream, "%lld", ast->integer);
            break;

        case NODE_VARIABLE:
            fprintf(stream, "%s", ast->identifier);
            break;

        case NODE_FUNC_CALL:
            fprintf(stream, "%s(", ast->call->identifier);
            for (int i = 0; i < ast->call->nParams - 1; i ++) {
                astToStringRecur(ast->call->parameters[i], stream);
                fprintf(stream, ", ");
            }
            astToStringRecur(ast->call->parameters[ast->call->nParams - 1], stream);
            fprintf(stream, ")");
            
        default:
            fprintf(stream, "How'd we get here?\n");
            break;
    }
}


char *astToString(ASTNode *ast) {
    FILE *stream = tmpfile();
    char *string = nullptr;
    if (ast == nullptr || stream == nullptr) {
        fclose(stream);
        return string;
    }
    astToStringRecur(ast, stream);
    
    long size = ftell(stream);
    if (size < 0) {
        fclose(stream);
        return nullptr;
    }

    string = malloc(size + 1);
    if (string == nullptr) {
        fclose(stream);
        return nullptr;
    }

    rewind(stream);

    size_t ptr = fread(string, 1, size, stream);
    string[ptr] = '\0';

    fclose(stream);
    return string;
}