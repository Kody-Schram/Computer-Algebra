#ifndef NORMALIZER_H 
#define NORMALIZER_H

#include "core/parsing/parser_types.h"

typedef enum NORMALIZER_RESULT {
	NORM_SUCCESS,
	NORM_ERROR,
	NORM_SYNTAX_ERROR
} NORMALIZER_RESULT;


NORMALIZER_RESULT handleLocalVariables(Token **ptr, char **parameters, int nParameters);


NORMALIZER_RESULT normalize(Token **head);

#endif
