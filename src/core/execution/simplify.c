#include <stdlib.h>
#include <string.h>

#include "simplify.h"
#include "core/utils/data_types.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"
#include "core/utils/log.h"

#define DEFAULT_FLATTEN_SIZE 2
#define DEFAULT_FLATTEN_INCREASE 5


static int flattenOpsRecur(Expression *expr, const Operation *op, Expression ***list, int *elts, int *size) {
    if (expr->type == EXPRESSION_OPERATOR && expr->op == op) {
        for (int i = 0; i < expr->arity; i ++) {
            if (!flattenOpsRecur(expr->operands[i], op, list, elts, size)) return 0;
        }
        
        // Ensures main flatten function doesnt free expressions moved to the flattened list
        expr->arity = 0;
        freeExpression(expr);
        
        return 1;
    }
    
    if (elts >= size) {
        *size += DEFAULT_FLATTEN_SIZE;
        Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
        if (temp == NULL) {
            perror("Error flattening operation");
            return 0;
        }
        
        *list = temp;
    }
    
    Debug(0, "\nAdding to list\n");
    Debug(1, printExpression(expr));
    
    (*list)[*elts] = expr;
    (*elts) ++;
    
    return 1;
}


static int flattenOps(Expression *expr) {
    if (expr == NULL) return 1;
    
    int size = DEFAULT_FLATTEN_SIZE;
    int elts = 0;
    Expression **list = malloc(sizeof(Expression *) * size);
    if (list == NULL) {
        perror("Error flattening operation");
        return 0;
    }
    
    Debug(0, "Recursively flattening\n");
    for (int i = 0; i < expr->arity; i ++) {
        if (!flattenOpsRecur(expr->operands[i], expr->op, &list, &elts, &size)) {
            free(list);
            return 0;
        }
    }
    
    free(expr->operands);
    
    Debug(0, "\nFlattened list %d\n", elts);
    for (int i = 0; i < elts; i ++) {
        Debug(1, printExpression(list[i]));
    }
    
    expr->operands = list;
    expr->arity = elts;
    
    return 1;
}


static int combineLikeTerms(Expression **ptr) {
    if (*ptr == NULL || (*ptr)->type != EXPRESSION_OPERATOR) return 1;
    Expression *expr = *ptr;
    
    eval:
    if (!expr->op->commutative && expr->arity >= expr->op->definition->nParameters) {
        for (int i = expr->op->definition->nParameters; i < expr->arity; i ++) {
            
        }
        
        return 1;
    }
    
    int parts = 0;
    Expression ***partitions = NULL;
    
    // Copies pointer list (when comparing will 0 out entries in list to show that expression has been handled)
    Expression **copy = NULL;
    memcpy(copy, expr->operands, sizeof(Expression *) * expr->arity);
    if (copy == NULL) goto error;
    
    partitions = array(Expression **, expr->arity);
    if (partitions == NULL) goto error;
    
    for (int i = 0; i < expr->arity; i ++) {
        if (expr->operands[i] == NULL) continue;
        partitions[parts] = array(Expression *, expr->arity - i);
        if (partitions[parts] == NULL) goto error;
        
        for (int j = i + 1; j < expr->arity; j ++) {
            if (expr->operands[j] == NULL) continue;
            
        }
        
    }
    
    // Handle commutative combining, otherwise just simple 
    return 1;
    
    error:
        perror("Error combining like terms");
        free(copy);
        if (partitions != NULL) {
            for (int i = 0; i < parts; i ++) free(partitions[i]);
        }
        free(partitions);
        return 0;
}


static int simplifyRecur(Expression **ptr) {
    if (ptr == NULL || *ptr == NULL) return 0;
    if ((*ptr)->type != EXPRESSION_OPERATOR) return 1; // No simplification to be done on non-operators
    Expression *expr = *ptr;
    
    Debug(0, "Flattening operation\n");
    if (expr->op->associative && !flattenOps(expr)) return 0;
    Debug(0, "\nafter\n");
    Debug(1, printExpression(expr));
    
    for (int i = 0; i < expr->arity; i ++) {
        if (!simplifyRecur(&(expr->operands[i]))) return 0;
    }
    
    Debug(0, "Combining like terms\n");
    if (!combineLikeTerms(ptr)) return 0;
    
    return 1;
}


int simplify(Expression **expr) {
    Debug(0, "\nSimplifying\n");
    Debug(1, printExpression(*expr));
    
    if (!simplifyRecur(expr)) return 0;
    
    return 1;
}