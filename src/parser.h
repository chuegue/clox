#ifndef __PARSER__
#define __PARSER__

#include "scanner.h"

typedef struct Expression_ Expression;

typedef struct {
    TokenType token_type;
    union {
        int bool_val;
        int null;
        char *string;
        double *number;
    } data;
} Literal;

typedef enum {
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_UNARY
} ExpressionType;

struct Expression_
{
    ExpressionType type;
    union {
        struct {
            Expression *left;
            Token *operator;
            Expression *right;
        } binary;

        Literal literal;
    } as;
};

typedef struct
{
    Token *tokens;
    int current;
} Parser;

typedef enum {
    STMT_EXPR,
    STMT_PRINT,
    STMT_VAR,
} StatementType;

typedef struct 
{
    StatementType type;
    Expression *expression1;
    Expression *expression2;
} Statement;



Parser *init_parser(Token *tokens, size_t len_tokens);
Statement **parse(Parser *parser, size_t *len_statements, int *error_return);
void print_expression(Expression *expr);
void print_literal(Literal *literal);

#endif //__PARSER__
