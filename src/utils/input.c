#include <stdio.h>
#include <stdlib.h>

const int DEFAULT_INPUT_SIZE = 50;

char* terminalEntry(int line) {

    char* expression = (char *) malloc(DEFAULT_INPUT_SIZE * sizeof(char));
    if (expression == nullptr) {
        printf("Cannot allocate space for expression.");
        free(expression);
        return nullptr;
    }
    
    printf("%d > ", line);
    fflush(stdout);

    int size = DEFAULT_INPUT_SIZE;
    int length = 0;

    char c;
    while ((c = getchar()) != '\n' && c!= EOF) {
        if (length >= size - 1) {
            size *= 2;
            char* temp = realloc(expression, size);
            if (temp == nullptr) {
                printf("Cannot allocate more space for expression.");
                free(temp);

                return nullptr;
            }

            expression = temp;
        }
        expression[length++] = c;
    }
    expression[length] = '\0';

    return expression;
}