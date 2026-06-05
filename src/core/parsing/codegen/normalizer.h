#pragma once

#include "core/parsing/parser.h"
#include "core/parsing/parser_types.h"


PARSER_RESULT handleLocalVariables(Token **ptr, char **parameters, int nParameters);


PARSER_RESULT normalize(Token **head);
