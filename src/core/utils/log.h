#pragma once

#include <stdio.h>

#ifdef NDEBUG 

#define Debug(fileStream, stream, ...) ((void) 0)

#else

void Debug(const int fileStream, const void *stream, ...);

#endif

void printStream(FILE *stream);


void Info(const int fileStream, const void *stream, ...);
