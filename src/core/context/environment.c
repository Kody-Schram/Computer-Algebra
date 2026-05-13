#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "context.h"
#include "core/utils/log.h"
#include "core/primitives/types.h" 
#include "core/utils/type_utils.h"


static void freeFunction(Function *func) {
    if (func == NULL) return;
    if (func->parameters != NULL) {
        for (int i = 0; i < func->nParameters; i ++) free(func->parameters[i]);
    }
    free(func->parameters);
    if (func->type == DEFINED) freeExpression(func->definition);
    free(func);
} 


static void freeComponent(Component *cmp) {
    switch (cmp->type) {
        case COMP_FUNCTION:
            Debug(0, "Freeing func '%s'\n", cmp->identifier);
            freeFunction(cmp->func);
            break;
            
        case COMP_VARIABLE:
            Debug(0, "Freeing variable '%s'\n", cmp->identifier);
            freeExpression(cmp->value);
            break;
    }
    free(cmp->identifier);
    free(cmp);
}


Environment *createEnvironment(EnvironmentType type) {
    Environment *env = calloc(1, sizeof(Environment));
    switch (type) {
        case ENV_HASH:
            printf("Hash env not implemented yet\n");
            return NULL;
            
        default:
            break;
    }

    return env;
}


bool bindComponent(Environment *env, ComponentType type, char const *identifier, void const *data) {
    Component *new = calloc(1, sizeof(Component));
    if (new == NULL) {
        perror("Error in binding component");
        return false;
    }
    
    new->type = type;
    new->identifier = strdup(identifier);
    if (new->identifier == NULL) {
        perror("Error binding component");
        freeComponent(new);
        return false;
    }
    
    switch (type) {
        case COMP_VARIABLE:
            new->value = (Expression *) data;
            break;
            
        case COMP_FUNCTION:
            new->func = (Function *) data;
            break;
            
        default:
            printf("What kinda shit you think this is?");
            freeComponent(new);
            return false;
    }
    
    switch (env->type) {
        case ENV_LIST:
            new->next = env->compList;
            env->compList = new;
            break;
            
        case ENV_HASH:
            printf("Binding to hash env not implemented yet.\n");
            freeComponent(new);
            return false;
    }
    
    return true;
}


Component *searchEnvironment(Environment const *env, char const *identifier) {
    if (identifier == NULL) return NULL;
    //printf("searching environment for '%s'\n", identifier);
    
    switch (env->type) {
        case ENV_LIST:
            //("Checking linked list env\n");
            Component *cmp = env->compList;
            while (cmp != NULL) {
                if (!strcmp(cmp->identifier, identifier)) return cmp;
                cmp = cmp->next;
            }
            
            return NULL;
            
        case ENV_HASH:
            printf("Stop trying bro.\n");
            return NULL;
    }

    return NULL;
}


static void printLinkedCmpList(FILE *stream, Component const *cmp) {
    while (cmp != NULL) {
        switch (cmp->type) {
            case COMP_FUNCTION:
                Function *func = cmp->func;
                fprintf(stream, "%s(", cmp->identifier);
                for (int i = 0; i < func->nParameters - 1; i ++) {
                    fprintf(stream, "%s, ", func->parameters[i]);
                }
                fprintf(stream, "%s) = ", func->parameters[func->nParameters-1]);
                
                char *str = expressionToString(func->definition);
                if (str == NULL) return;
                fprintf(stream, "%s\n", str);
                free(str);
                
                break;
                
            case COMP_VARIABLE:
                fprintf(stream, "%s = ", cmp->identifier);
                
                str = expressionToString(cmp->value);
                if (str == NULL) return;
                fprintf(stream, "%s\n", str);
                free(str);
                
                break;
        }
        cmp = cmp->next;
    }
}


FILE *printEnvironment(Environment const *env) {
    FILE *stream = tmpfile();
    if (stream == NULL || env == NULL) return NULL;
    
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
    Debug(0, "Freeing environment\n");
    switch (env->type) {
        case ENV_LIST:
            Component *cmp = env->compList;
            while (cmp != NULL) {
                Component *temp = cmp->next;
                freeComponent(cmp);
                cmp = temp;
            }
            break;
            
        case ENV_HASH:
            printf("Hashing not implemented!.\n");
            break;
    }

    free(env);
    Debug(0, "\n");
}


int initOutputVariables(Environment *env) {
    int outputs = GLOBALCONTEXT->config->OUTPUTS;
    if (outputs > 0) {
        if (outputs == 1) {
            Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
            if (temp == NULL) return 0;
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
            if (str == NULL) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

            Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
            if (temp == NULL) return 0;
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


int updateOutputVariables(Environment *env, Expression *output) {
    Debug(0, "Updating output variable(s).\n");
    if (GLOBALCONTEXT->config->OUTPUTS == 1) {
        Component *cmp = searchEnvironment(GLOBALCONTEXT->env, GLOBALCONTEXT->config->OUTPUT_ID);
        if (cmp == NULL) return 0;

        freeExpression(cmp->value);
        cmp->value = output;
        return 1;
    } else {
        int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
        char *str = malloc(size);
        if (str == NULL) return 0;
        snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, GLOBALCONTEXT->config->OUTPUTS - 1);

        Component *last = searchEnvironment(GLOBALCONTEXT->env, str);
        if (last == NULL) {
            free(str);
            return 0;
        }
        free(str);
        freeExpression(last->value);

        for (int i = GLOBALCONTEXT->config->OUTPUTS - 2; i >= 0; i --) {
            size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
            str = malloc(size);
            if (str == NULL) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, str);
            if (cmp == NULL) {
                free(str);
                return 0;
            }
            free(str);

            last->value = cmp->value;
            last = cmp;
        }

        size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;
        str = malloc(size);
        if (str == NULL) return 0;
        snprintf(str, size, "%s_0", GLOBALCONTEXT->config->OUTPUT_ID);

        Component *first = searchEnvironment(GLOBALCONTEXT->env, str);
        if (first == NULL) {
            free(str);
            return 0;
        }
        free(str);

        first->value = output;

        return 1;
    }

    return 1;
}
