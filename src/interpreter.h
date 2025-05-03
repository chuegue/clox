#ifndef __INTERPRETER__
#define __INTERPRETER__

#include "parser.h"

Literal *evaluate(Expression *expr, int *error_code);

#endif //__INTERPRETER__