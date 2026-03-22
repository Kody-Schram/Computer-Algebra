#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"
#include "utils/context/context.h"

static const int DEFAULT_BUFFER = 128;


void printStream(FILE *stream) {
    rewind(stream);
    char buffer[DEFAULT_BUFFER];
    while (fgets(buffer, DEFAULT_BUFFER, stream)) {
        printf("%s", buffer);
    }
    fclose(stream);
}

void Debug(const int fileStream, const void *stream, ...) {
    if (GLOBALCONTEXT->config->LOG_LEVEL < DEBUG) return;

    va_list args;
    va_start(args, stream);

    if (fileStream) {
        FILE *file = (FILE *) stream;
        rewind(file);
        char buffer[DEFAULT_BUFFER];
        while (fgets(buffer, DEFAULT_BUFFER, file)) {
            Debug(0, "%s", buffer);
        }
        fclose(file);
    } else  {
        vfprintf(GLOBALCONTEXT->config->LOG_STREAM, (char *) stream, args);
        fflush(GLOBALCONTEXT->config->LOG_STREAM);
    }

    va_end(args);
}


void Info(const int fileStream, const void *stream, ...) {
    if (GLOBALCONTEXT->config->LOG_LEVEL < INFO) return;

    va_list args;
    va_start(args, stream);

    if (fileStream) {
        FILE *file = (FILE *) stream;
        rewind(file);
        char buffer[DEFAULT_BUFFER];
        while (fgets(buffer, DEFAULT_BUFFER, file)) {
            Debug(0, 0, "%s", buffer);
        }
        fclose(file);
    } else {
        vfprintf(GLOBALCONTEXT->config->LOG_STREAM, (char *) stream, args);
        fflush(GLOBALCONTEXT->config->LOG_STREAM);
    }

    va_end(args);
}