#ifndef LOG_H
#define LOG_H

#include "core/utils/context/context.h"

#ifdef Release

#define Debug(fileStream, stream, ...) ((void) 0)

#else

void Debug(const int fileStream, const void *stream, ...);

#endif

void printStream(FILE *stream);


void Info(const int fileStream, const void *stream, ...);

#endif