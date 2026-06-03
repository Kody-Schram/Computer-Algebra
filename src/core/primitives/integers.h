#pragma once

#include "core/common.h"

#define INTEGER "core_int"
#define INTEGER_ID CREATE_REF_8(INTEGER) 


typedef struct Registry Registry;


bool initIntegers(Registry *registry);
