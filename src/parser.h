#ifndef __PARSER__
#define __PARSER__

#include "scanner.h"

typedef struct Expression_ Expression;

typedef enum
{
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_UNARY,
    EXPR_VARIABLE,
} ExpressionType;

struct Expression_
{
    ExpressionType type;
    union
    {
        struct
        {
            Expression *left;
            Token *operator;
            Expression *right;
        } binary;

        Literal *literal;

        struct
        {
            Token *name;
        } variable;
    } as;
};

typedef struct
{
    Token **tokens; // array of Token*
    int current;
} Parser;

typedef enum
{
    STMT_EXPR,
    STMT_PRINT,
    STMT_VAR,
} StatementType;

typedef struct
{
    StatementType type;
    union
    {
        struct
        {
            Expression *expression;
        } expr;
        struct
        {
            Expression *expression;
        } print;
        struct
        {
            Token *name;
            Expression *initializer;
        } var;
    } data;

} Statement;

Parser *init_parser(Token **tokens, size_t len_tokens);
Statement **parse(Parser *parser, size_t *len_statements, int *error_return);
void print_expression(Expression *expr);
void print_literal(Literal *literal);
void print_statement(Statement *stmt);
Token *init_token(char *lexeme, Literal *literal, int line);
void free_statements(Statement **stmts, size_t len_statements);
void free_parser(Parser *parser);
#endif //__PARSER__
