#ifndef BUILTINS
#define BUILTINS

#include <math.h>
#include "functions.h"

const char *identifiers[] = {"sin", "cos", "tan", "csc", "sec", "cot", "ln"};
const int builtins = 7;

Function sin_function = {
    .identifier = "sin",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = sin
};

Function cos_function = {
    .identifier = "cos",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = cos
};

Function tan_function = {
        .identifier = "tan",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = tan
};

double csc(double x) {
    return 1/sin(x);
}

Function csc_function = {
    .identifier = "csc",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = csc
};

double sec(double x) {
    return 1/cos(x);
}

Function cos_function = {
    .identifier = "sec",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = sec
};

double cot(double x) {
    return 1/tan(x);
}

Function tan_function = {
    .identifier = "cot",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = cot
};

Function ln_function = {
    .identifier = "ln",
    .parameters = (char *[]) {"x"},
    .nParameters = 1,
    .type = BUILTIN,
    .builtin = log
};

#endif