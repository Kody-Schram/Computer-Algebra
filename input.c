#include <stdio.h>
#include <stdlib.h>

const int DEFAULT_INPUT_SIZE = 50;

char* terminalEntry() {

    char* expression = (char *) malloc(DEFAULT_INPUT_SIZE * sizeof(char));
    if (expression == NULL) {
        printf("Cannot allocate space for expression.");
        free(expression);
        return NULL;
    }
    
    printf("Enter an expression: ");
    fflush(stdout);

    int size = DEFAULT_INPUT_SIZE;
    int length = 0;

    char c;
    while ((c = getchar()) != '\n' && c!= EOF) {
        if (length >= size - 1) {
            size *= 2;
            char* temp = realloc(expression, size);
            if (temp == NULL) {
                printf("Cannot allocate more space for expression.");
                free(expression);

                return NULL;
            }

            expression = temp;
        }
        expression[length++] = c;
    }
    expression[length] = '\0';

    return expression;
}