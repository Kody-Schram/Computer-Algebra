#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "context.h"
#include "core/utils/types.h" 


static void freeFunction(Function *func) {
    if (func == nullptr) return;
    freeEnvironment(func->env);
    if (func->type == DEFINED) freeExpression(func->definition);
    free(func);
} 


static void freeOperation(Operation *op) {
    if (op == nullptr) return;
    if (op->definition != nullptr) freeFunction(op->definition);
    free(op);
}


static void freeComponent(Component *cmp) {
    free(cmp->identifier);
    switch (cmp->type) {
        case COMP_FUNCTION:
            freeFunction(cmp->func);
            break;
            
        case COMP_OPERATION:
            freeOperation(cmp->operation);
            break;
            
        case COMP_VARIABLE:
            freeExpression(cmp->value);
            break;
    }
    free(cmp);
}


Environment *createEnvironment(EnvironmentType type) {
    Environment *env = calloc(1, sizeof(Environment));
    switch (type) {
        case ENV_HASH:
            printf("Hash env not implemented yet\n");
            return nullptr;
            
        default:
            break;
    }

    return env;
}


int bindComponent(Environment *env, ComponentType type, char *identifier, void *data) {
    Component *new = calloc(1, sizeof(Component));
    if (new == nullptr) {
        perror("Error in binding component");
        return 0;
    }
    
    new->type = type;
    new->identifier = strdup(identifier);
    if (new->identifier == nullptr) {
        perror("Error binding component");
        freeComponent(new);
        return 0;
    }
    
    switch (type) {
        case COMP_OPERATION:
            new->operation = (Operation *) data;
            break;
            
        case COMP_VARIABLE:
            new->value = (Expression *) data;
            break;
            
        case COMP_FUNCTION:
            new->func = (Function *) data;
            break;
            
        default:
            printf("What kinda shit you think this is?");
            freeComponent(new);
            return 0;
    }
    
    switch (env->type) {
        case ENV_LIST:
            new->next = env->compList;
            env->compList = new;
            break;
            
        case ENV_HASH:
            printf("Binding to hash env not implemented yet.\n");
            freeComponent(new);
            return 0;
    }
    
    return 1;
}


Component* searchEnvironment(Environment *env, char *identifier) {
    if (identifier == nullptr) return nullptr;
    switch (env->type) {
        case ENV_LIST:
            Component *cmp = env->compList;
            while (cmp != nullptr) {
                if (!strcmp(cmp->identifier, identifier)) return cmp;
            }
            break;
            
        case ENV_HASH:
            printf("Stop trying bro.\n");
            return nullptr;
    }

    return nullptr;
}

static void printLinkedCmpList(FILE *stream, Component *cmp) {
    while (cmp != nullptr) {
        switch (cmp->type) {
            case COMP_FUNCTION:
                Function *func = cmp->func;
                Component *localCmp = func->env->compList;
                if (localCmp == nullptr) return;
                fprintf(stream, "%s(", cmp->identifier);
                while (localCmp->next != nullptr) {
                    fprintf(stream, "%s, ", localCmp->identifier);
                    localCmp = localCmp->next;
                }
                fprintf(stream, "%s)", localCmp->identifier);
                
                char *str = expressionToString(func->definition);
                if (str == nullptr) return;
                fprintf(stream, "%s\n", str);
                free(str);
                
                break;
                
            case COMP_VARIABLE:
                fprintf(stream, "%s = ", cmp->identifier);
                
                str = expressionToString(cmp->value);
                if (str == nullptr) return;
                fprintf(stream, "%s\n", str);
                free(str);
                
                break;
                
            case COMP_OPERATION:
                if (cmp->operation->type == AXIOMATIC) break; // will only print out user defined operations (maybe add setting in config for this)
                fprintf(stream, "%s(x, y) = ", cmp->identifier);
                
                str = expressionToString(cmp->operation->definition->definition);
                if (str == nullptr) return;
                fprintf(stream, "%s\n", str);
                free(str);
                
                fprintf(stream, "  Associative: %d\n", cmp->operation->associative);
                fprintf(stream, "  Commutative: %d\n", cmp->operation->commutative);
                
                break;
                
        }
        cmp = cmp->next;
    }
}


FILE *printEnvironment(Environment *env) {
    FILE *stream = tmpfile();
    if (stream == nullptr || env == nullptr) return nullptr;
    
    switch (env->type) {
        case ENV_LIST:
            Component *cmp = env->compList;
            printLinkedCmpList(stream, cmp);
            break;
        
        case ENV_HASH:
            printf("This is a hash??\n");
            break;
    }

    return stream;
}


void freeEnvironment(Environment *env) {
    switch (env->type) {
        case ENV_LIST:
            Component *cmp = env->compList;
            while (cmp != nullptr) {
                Component *temp = cmp->next;
                freeComponent(cmp);
                cmp = temp;
            }
            free(env->compList);
            
        case ENV_HASH:
            printf("Hashing not implemented!.\n");
            break;
    }

    free(env);
}


int initOutputVariables(Environment *env) {
    int outputs = GLOBALCONTEXT->config->OUTPUTS;
    if (outputs > 0) {
        if (outputs == 1) {
            Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
            if (temp == nullptr) return 0;
            temp->value = 0;

            if (!bindComponent(env, COMP_VARIABLE, GLOBALCONTEXT->config->OUTPUT_ID, temp)) {
                free(temp);
                return 0;
            }
            return 1;
        }

        for (int i = 0; i < outputs; i ++) {
            int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;

            char *str = malloc(size);
            if (str == nullptr) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

            Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
            if (temp == nullptr) return 0;
            temp->value = 0;

            if (!bindComponent(env, COMP_VARIABLE, str, temp)) {
                free(temp);
                free(str);
                return 0;
            }

            free(str);
        }
    }

    return 1;
}