#include <stdio.h>
#include <stdarg.h>

#include "log.h"
#include "utils/context/context.h"

void Debug(const char *c, ...) {
    if (GLOBALCONTEXT->config->LOG_LEVEL >= DEBUG) {
        va_list args;
        va_start(args, c);
        vfprintf(GLOBALCONTEXT->config->LOG_STREAM, c, args);
        va_end(args);

        fflush(GLOBALCONTEXT->config->LOG_STREAM);
    }
}