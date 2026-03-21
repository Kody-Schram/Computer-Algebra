#include <stdio.h>
#include <stdarg.h>

#include "log.h"
#include "utils/context/context.h"

void Debug(const char *c, ...) {
    va_list args;
    va_start(args, c);
    if (GLOBALCONTEXT->config->LOG_LEVEL >= DEBUG) fprintf(GLOBALCONTEXT->config->LOG_STREAM, c, args);
}