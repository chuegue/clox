#include <string.h>

#include "environment.h"

EnvironmentNode *init_environment_node(char *name, Literal *value)
{
    EnvironmentNode *new = calloc(1, sizeof(EnvironmentNode));
    new->key = name;
    new->value = value;
    new->next = NULL;
    return new;
}

void free_environment_node(EnvironmentNode *node)
{
    free(node->key);
    node->key = NULL;
    Literal *lit = node->value;
    if (lit->token_type == STRING)
    {
        free(lit->data.string);
        lit->data.string = NULL;
    }
    if (lit->token_type == NUMBER)
    {
        free(lit->data.number);
        lit->data.number = NULL;
    }
    free(node);
    node = NULL;
}

Environment *init_environment(Environment *enclosing)
{
    Environment *env = calloc(1, sizeof(Environment *));
    env->nodes = calloc(ENVIRONMENT_SIZE, sizeof(EnvironmentNode *));
    env->enclosing = enclosing;
    return env;
}

void free_environment(Environment *env)
{
    for (size_t i = 0; i < ENVIRONMENT_SIZE; i++)
    {
        if (env->nodes[i] != NULL)
        {
            EnvironmentNode *tmp, *head = env->nodes[i];
            while (head != NULL)
            {
                tmp = head;
                head = head->next;
                free_environment_node(tmp);
            }
        }
    }
    free(env->nodes);
    env->nodes = NULL;
    free(env);
    env = NULL;
}

size_t hash(char *str)
{
    size_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

EnvironmentNode *get_node_environment(Environment *env, size_t idx)
{
    return env->nodes[idx];
}

void put_environment(Environment *env, char *name, Literal *value)
{
    size_t idx = hash(name) % ENVIRONMENT_SIZE;
    EnvironmentNode *current = env->nodes[idx];
    while (current != NULL)
    {
        if (strcmp(current->key, name) == 0)
        {
            current->value = value;
            return;
        }
        current = current->next;
    }
    // append at the end
    EnvironmentNode *new = init_environment_node(name, value);
    new->key = strdup(name);
    new->value = value;
    new->next = env->nodes[idx];
    env->nodes[idx] = new;
}

Literal *get_environment(Environment *env, char *name, int *error_code)
{
    size_t idx = hash(name) % ENVIRONMENT_SIZE;
    EnvironmentNode *current = env->nodes[idx];
    while (current != NULL)
    {
        if (strcmp(current->key, name) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    if (env->enclosing != NULL)
    {
        return get_environment(env->enclosing, name, error_code);
    }
    *error_code = 70;
    fprintf(stderr, "Undefined variable '%s'.\n", name);
    return NULL;
}

void assign_environment(Environment *env, Token *name, Literal *value, int *error_code)
{
    size_t idx = hash(name->lexeme) % ENVIRONMENT_SIZE;
    EnvironmentNode *current = env->nodes[idx];
    while (current != NULL)
    {
        if (strcmp(current->key, name->lexeme) == 0)
        {
            current->value = value;
            return;
        }
    }
    if (env->enclosing != NULL)
    {
        assign_environment(env->enclosing, name, value, error_code);
        return;
    }
    *error_code = 70;
    fprintf(stderr, "Undefined variable '%s'.\n", name->lexeme);
}

void define_environment(Environment *env, char *name, Literal *value)
{
    put_environment(env, name, value);
}