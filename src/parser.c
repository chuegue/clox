#include "scanner.h"
#include "parser.h"

int error_return_global = 0;

Expression *init_expression_binary(Expression *left, Token *operator, Expression *right, ExpressionType expression_type)
{
    Expression *expression = calloc(1, sizeof(Expression));

    expression->as.binary.left = left;
    expression->as.binary.operator = operator;
    expression->as.binary.right = right;
    expression->type = expression_type;
    return expression;
}

Expression *init_expression_literal(Literal *literal, ExpressionType expression_type)
{
    Expression *expression = calloc(1, sizeof(Expression));

    memcpy(&(expression->as.literal), literal, sizeof(Literal));
    expression->type = expression_type;
    return expression;
}

Parser *init_parser(Token *tokens, size_t len_tokens)
{
    Parser *parser = calloc(1, sizeof(Parser));
    parser->tokens = calloc(len_tokens, sizeof(Token));
    memcpy(parser->tokens, tokens, len_tokens * sizeof(Token));
    parser->current = 0;
    return parser;
}

Token *peek_parser(Parser *parser)
{
    return &parser->tokens[parser->current];
}

int isAtEnd_parser(Parser *parser)
{
    Token *t = peek_parser(parser);
    return t->type == EOF_LOX;
}

Token *previous(Parser *parser)
{
    return &parser->tokens[parser->current - 1];
}

Token *advance_parser(Parser *parser)
{
    if (!isAtEnd_parser(parser))
    {
        parser->current++;
    }
    return previous(parser);
}

int check(Parser *parser, TokenType type)
{
    if (isAtEnd_parser(parser))
    {
        return 0;
    }
    return parser->tokens[parser->current].type == type;
}

int match_parser(Parser *parser, TokenType types[], size_t len_types)
{
    for (size_t i = 0; i < len_types; i++)
    {
        if (check(parser, types[i]))
        {
            Token *t = advance_parser(parser);
            return 1;
        }
    }
    return 0;
}

Token *consume(Parser *parser, TokenType type, char message[])
{
    error_return_global = 65;
    if (check(parser, type))
    {
        return advance_parser(parser);
    }
    fprintf(stderr, "%s", message);
    if (type == EOF_LOX)
    {
        fprintf(stderr, "Line %d at end. %s", peek_parser(parser)->line, message);
    }
    else
    {
        fprintf(stderr, "Line %d at '%s'. %s", peek_parser(parser)->line, peek_parser(parser)->lexeme, message);
    }
}

void synchronize(Parser *parser)
{
    advance_parser(parser);
    while (!isAtEnd_parser(parser))
    {
        if (previous(parser)->type == SEMICOLON)
        {
            return;
        }
        switch (peek_parser(parser)->type)
        {
        case CLASS:
        case FUN:
        case VAR:
        case FOR:
        case IF:
        case WHILE:
        case PRINT:
        case RETURN:
            return;
        }
        advance_parser(parser);
    }
}

Expression *expression(Parser *parser);
// primary        → NUMBER | STRING | "true" | "false" | "nil"
// | "(" expression ")" ;
Expression *primary(Parser *parser)
{
    Expression *expression_literal = calloc(1, sizeof(Expression));
    TokenType allowed_type = FALSE;
    if (match_parser(parser, &allowed_type, 1))
    {
        expression_literal->as.literal.token_type = FALSE;
        expression_literal->as.literal.data.bool_val = 0;
        expression_literal->type = EXPR_LITERAL;
        return expression_literal;
    }
    allowed_type = TRUE;
    if (match_parser(parser, &allowed_type, 1))
    {
        expression_literal->as.literal.token_type = TRUE;
        expression_literal->as.literal.data.bool_val = 1;
        expression_literal->type = EXPR_LITERAL;
        return expression_literal;
    }
    allowed_type = NIL;
    if (match_parser(parser, &allowed_type, 1))
    {
        expression_literal->as.literal.token_type = NIL;
        expression_literal->as.literal.data.null = 1;
        expression_literal->type = EXPR_LITERAL;
        return expression_literal;
    }
    TokenType allowed_tokens[] = {NUMBER};
    if (match_parser(parser, allowed_tokens, 1))
    {
        Token *prev = previous(parser);
        double *prev_literal = (double *)prev->literal;
        expression_literal->as.literal.data.number = prev_literal;
        expression_literal->type = EXPR_LITERAL;
        return expression_literal;
    }
    allowed_tokens[0] = STRING;
    if (match_parser(parser, allowed_tokens, 1))
    {
        Token *prev = previous(parser);
        char *prev_literal = (char *)prev->literal;
        expression_literal->as.literal.data.string = prev_literal;
        expression_literal->type = EXPR_LITERAL;
        return expression_literal;
    }
    allowed_tokens[0] = LEFT_PAREN;
    if (match_parser(parser, allowed_tokens, 1))
    {
        Expression *expr = expression(parser);
        consume(parser, RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }
    consume(parser, EOF_LOX, "Expect expression.");
}

// unary          → ( "!" | "-" ) unary
// | primary ;
Expression *unary(Parser *parser)
{
    TokenType allowed_Types[] = {BANG, MINUS};
    if (match_parser(parser, allowed_Types, 2))
    {
        Token *operator = previous(parser);
        Expression *right = unary(parser);
        return init_expression_binary(NULL, operator, right, EXPR_BINARY);
    }
    return primary(parser);
}

Expression *factor(Parser *parser)
{
    Expression *expr = unary(parser);
    TokenType allowed_types[] = {SLASH, STAR};
    while (match_parser(parser, allowed_types, 2))
    {
        Token *operator = previous(parser);
        Expression *right = unary(parser);
        expr = init_expression_binary(expr, operator, right, EXPR_BINARY);
    }
    return expr;
}

Expression *term(Parser *parser)
{
    Expression *expr = factor(parser);
    TokenType allowed_types[] = {MINUS, PLUS};
    while (match_parser(parser, allowed_types, 2))
    {
        Token *operator = previous(parser);
        Expression *right = factor(parser);
        expr = init_expression_binary(expr, operator, right, EXPR_BINARY);
    }
    return expr;
}

Expression *comparison(Parser *parser)
{
    Expression *expr = term(parser);
    TokenType allowed_types[] = {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL};
    while (match_parser(parser, allowed_types, 4))
    {
        Token *operator = previous(parser);
        Expression *right = term(parser);
        expr = init_expression_binary(expr, operator, right, EXPR_BINARY);
    }
    return expr;
}

// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
Expression *equality(Parser *parser)
{
    Expression *expr = comparison(parser);
    TokenType allowed_types[] = {BANG_EQUAL, EQUAL_EQUAL};
    while (match_parser(parser, allowed_types, 2))
    {
        Token *operator = previous(parser);
        Expression *right = comparison(parser);
        expr = init_expression_binary(expr, operator, right, EXPR_BINARY);
    }
    return expr;
}

// expression     → equality ;
Expression *expression(Parser *parser)
{
    return equality(parser);
}

Expression *parse(Parser *parser, int *error_return)
{
    Expression *tree = expression(parser);
    *error_return = error_return_global;
    return tree;
}

void print_expression(Expression *expr)
{
    if (expr == NULL)
    {
        return;
    }

    switch (expr->type)
    {
    case EXPR_BINARY:
    {
        print_expression(expr->as.binary.left);
        print_expression(expr->as.binary.right);
        printf("%s ", expr->as.binary.operator->lexeme);
        break;
    }

    case EXPR_LITERAL:
    {
        TokenType type = expr->as.literal.token_type;
        switch (type)
        {
        case NUMBER:
            double number = *expr->as.literal.data.number;
            if (floor(number) == number)
            { // integer
                printf("%.1lf ", number);
            }
            else
            { // floatvoid print_expression(Expression *expr)
                printf("%.15g ", number);
            }
            break;
        case STRING:
            if (expr->as.literal.data.string != NULL)
            {
                printf("\"%s\" ", expr->as.literal.data.string);
            }
            else
            {
                printf("\"\" ");
            }
            break;
        case TRUE:
            printf("true");
            break;
        case FALSE:
            printf("false");
            break;
        case NIL:
            printf("nil");
            break;
        default:
            printf("<unknown literal> ");
            break;
        }
        break;
    }

    default:
        printf("<unknown expr> ");
        break;
    }
}