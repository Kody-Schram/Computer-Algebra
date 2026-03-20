#ifndef EXECUTE_H
#define EXECUTE_H

#include "utils/types.h"
#include "utils/config.h"
#include "utils/env/environment.h"


int execute(ASTNode *ast, Environment *env, Config *config);

#endif