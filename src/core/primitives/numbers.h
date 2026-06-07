#pragma once

#include "core/common.h"

#define NUMBER "core_num"
#define NUMBER_ID CREATE_REF_8(NUMBER) 

typedef struct Registry Registry;


bool defaultNumberParser(char const *input, ObjectValue *value, ExpressionMeta *meta);


bool initNumbers(Registry *registry);
