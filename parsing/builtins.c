#include <math.h>
#include "builtins.h"

const char *builtin_identifiers[] = {"sin", "cos", "tan", "csc", "sec", "cot", "ln"};
const int nBuiltins = 7;

double csc(double x) {
    return 1/sin(x);
}

double sec(double x) {
    return 1/cos(x);
}

double cot(double x) {
    return 1/tan(x);
}

const Function builtins[] = {
    {
        .identifier = "sin",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = sin
    }, 
    {
        .identifier = "cos",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = cos
    },
     {
        .identifier = "tan",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = tan
    }, 
    {
        .identifier = "csc",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = csc
    },
    {
        .identifier = "sec",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = sec
    },
    {
        .identifier = "cot",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = cot
    },
    {
        .identifier = "ln",
        .parameters = (char *[]) {"x"},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = log
    },
    {
        .identifier = "int",
        .parameters = (char *[]) {},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = 0
    },
    {
        .identifier = "der",
        .parameters = (char *[]) {},
        .nParameters = 1,
        .type = BUILTIN,
        .builtin = 0
    }
};

const Constant constants[] = {
    {
        .identifier = "pi",
        .value = M_PI
    }
};

const int nConstants = 1;