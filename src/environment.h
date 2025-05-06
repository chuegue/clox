#ifndef __ENVIRONMENT__
#define __ENVIRONMENT__

#include "parser.h"

#define ENVIRONMENT_SIZE 128

typedef struct EnvironmentNode_
{
    char *key;
    Literal *value;
    struct EnvironmentNode_ *next;
} EnvironmentNode;

typedef struct Environment_
{
    struct Environment_ *enclosing;
    EnvironmentNode **nodes; // array of EnvironmentNode*
} Environment;

Environment *init_environment(Environment *enclosing);
void define_environment(Environment *env, char *name, Literal *value);
Literal *get_environment(Environment *env, char *name, int *error_code);
void free_environment(Environment *env);
void assign_environment(Environment *env, Token *name, Literal *value, int *error_code);

#endif //__ENVIRONMENT__