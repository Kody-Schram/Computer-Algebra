#pragma once

#include "core/primitives/types.h"


typedef enum EXECUTOR_RESULT {
	EXECUTOR_SUCCESS,
	EXECUTOR_RUNTIME_ERROR,
	EXECUTOR_ERROR
} EXECUTOR_RESULT;


EXECUTOR_RESULT execute(Expression **expr, char **output);
