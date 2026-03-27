#ifndef LOG_H
#define LOG_H

#include "utils/context/context.h"


void printStream(FILE *stream);


void Debug(const int fileStream, const void *stream, ...);


void Info(const int fileStream, const void *stream, ...);

#endif