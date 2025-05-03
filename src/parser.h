#ifndef __PARSER__
#define __PARSER__

#include "scanner.h"

typedef struct Expression_ Expression;

// typedef union {
//     int bool_val;
//     int null;
//     char *string;
//     double *number;
// } Literal_;

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
    EXPR_BINARY
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

Parser *init_parser(Token *tokens, size_t len_tokens);
Expression *parse(Parser *parser, int *error_return);
void print_expression(Expression *expr);

#endif //__PARSER__
