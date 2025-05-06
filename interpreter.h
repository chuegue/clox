#ifndef __INTERPRETER__
#define __INTERPRETER__

#include "parser.h"
#include "environment.h"

typedef struct
{
    Environment *env;
} Interpreter;

Literal *evaluate(Interpreter *interpreter, Expression *expr, int *error_code);
void interpret(Statement **statements, size_t len_statements, int *error_code_param);

#endif //__INTERPRETER__