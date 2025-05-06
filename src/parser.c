#include <math.h>

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

    expression->as.literal = literal;
    expression->type = expression_type;
    return expression;
}

Parser *init_parser(Token **tokens, size_t len_tokens)
{
    Parser *parser = calloc(1, sizeof(Parser));
    parser->tokens = calloc(len_tokens, sizeof(Token *));
    memcpy(parser->tokens, tokens, len_tokens * sizeof(Token *));
    parser->current = 0;
    return parser;
}

void free_parser(Parser *parser)
{
    free(parser->tokens);
    parser->tokens = NULL;
    free(parser);
}

void free_expression(Expression *expr)
{
    if (!expr)
    {
        return;
    }
    switch (expr->type)
    {
    case EXPR_BINARY:
    case EXPR_GROUPING:
    case EXPR_UNARY:
        free_expression(expr->as.binary.left);
        expr->as.binary.left = NULL;
        free_expression(expr->as.binary.right);
        expr->as.binary.right = NULL;
        // free_token(expr->as.binary.operator);
        // expr->as.binary.operator = NULL;
        break;
    case EXPR_LITERAL:
        if (expr->as.literal && expr->as.literal->token_type == NIL)
        {
            free_literal(expr->as.literal);
            expr->as.literal = NULL;
        }
        break;
    case EXPR_VARIABLE:
        // free_token(expr->as.variable.name);
        // expr->as.variable.name = NULL;
        break;
    default:
        break;
    }
    free(expr);
}

void free_statement(Statement *stmt)
{
    if (!stmt)
    {
        return;
    }
    free_expression(stmt->expression1);
    stmt->expression1 = NULL;
    free_expression(stmt->expression2);
    stmt->expression1 = NULL;
    free(stmt);
}

void free_statements(Statement **stmts, size_t len_statements)
{
    for (size_t i = 0; i < len_statements; i++)
    {
        free_statement(stmts[i]);
        stmts[i] = NULL;
    }
    free(stmts);
}

int isAtEnd_parser(Parser *parser)
{
    return parser->tokens[parser->current]->literal->token_type == EOF_LOX;
}

Token *peek_parser(Parser *parser)
{
    return parser->tokens[parser->current];
}

Token *previous(Parser *parser)
{
    if (isAtEnd_parser(parser))
    {
        return NULL;
    }

    return parser->tokens[parser->current - 1];
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
    return parser->tokens[parser->current]->literal->token_type == type;
}

int match_parser(Parser *parser, TokenType types[], size_t len_types)
{
    for (size_t i = 0; i < len_types; i++)
    {
        if (check(parser, types[i]))
        {
            return 1;
        }
    }
    return 0;
}

Token *consume(Parser *parser, TokenType type, char message[])
{
    if (check(parser, type))
    {
        return advance_parser(parser);
    }
    // fprintf(stderr, "[ERROR] consume() triggered: %s\n", message);
    error_return_global = 65;
    if (type == EOF_LOX)
    {
        fprintf(stderr, "Line %d at end. %s", peek_parser(parser)->line, message);
    }
    else
    {
        fprintf(stderr, "Line %d at '%s'. %s", peek_parser(parser)->line, peek_parser(parser)->lexeme, message);
    }
    printf("\n");
    advance_parser(parser); // Force advance to next token
    return NULL;
}

void synchronize(Parser *parser)
{
    advance_parser(parser);
    while (!isAtEnd_parser(parser))
    {
        if (previous(parser)->literal->token_type == SEMICOLON)
        {
            return;
        }
        switch (peek_parser(parser)->literal->token_type)
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
    Expression *expression_literal = NULL;
    Literal *lit = NULL;
    TokenType allowed_type = FALSE;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser);
        lit = init_literal(FALSE, NULL, 0);
        expression_literal = init_expression_literal(lit, EXPR_LITERAL);
        return expression_literal;
    }
    allowed_type = TRUE;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser);
        lit = init_literal(TRUE, NULL, 1);
        expression_literal = init_expression_literal(lit, EXPR_LITERAL);
        return expression_literal;
    }
    allowed_type = NIL;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser);
        lit = init_literal(NIL, NULL, 1);
        expression_literal = init_expression_literal(lit, EXPR_LITERAL);
        return expression_literal;
    }
    allowed_type = NUMBER;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser);
        Token *prev = previous(parser);
        // lit = init_literal(NUMBER, prev->literal->data.number, 0);
        lit = prev->literal;
        expression_literal = init_expression_literal(lit, EXPR_LITERAL);
        return expression_literal;
    }
    allowed_type = STRING;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser);
        Token *prev = previous(parser); // Get consumed STRING token
        lit = prev->literal;
        // lit = init_literal(STRING, prev->literal->data.string, 0);
        expression_literal = init_expression_literal(lit, EXPR_LITERAL);
        return expression_literal;
    }
    allowed_type = LEFT_PAREN;
    if (match_parser(parser, &allowed_type, 1))
    {
        advance_parser(parser); // Consume '('
        Expression *expr = expression(parser);
        // fprintf(stderr, "%s\n", expr->as.literal->data.string);
        consume(parser, RIGHT_PAREN, "Expect ')' after expression.");

        // fprintf(stderr, "Wrapping in group\n");
        return init_expression_binary(expr, NULL, NULL, EXPR_GROUPING);
    }
    allowed_type = IDENTIFIER;
    if (match_parser(parser, &allowed_type, 1))
    {
        Token *name = advance_parser(parser);
        Expression *var_expr = calloc(1, sizeof(Expression));
        var_expr->type = EXPR_VARIABLE;
        var_expr->as.variable.name = name;
        return var_expr;

        // advance_parser(parser); // consume IDENTIFIER
        // return expression(parser);
    }
    consume(parser, EOF_LOX, "Expect expression.");
    return NULL;
}

// unary          → ( "!" | "-" ) unary
// | primary ;
Expression *unary(Parser *parser)
{
    TokenType allowed_Types[] = {BANG, MINUS};
    if (match_parser(parser, allowed_Types, 2))
    {
        Token *operator = advance_parser(parser);
        Expression *right = unary(parser);
        return init_expression_binary(NULL, operator, right, EXPR_UNARY);
    }
    return primary(parser);
}

Expression *factor(Parser *parser)
{
    Expression *expr = unary(parser);
    TokenType allowed_types[] = {SLASH, STAR};
    while (match_parser(parser, allowed_types, 2))
    {
        Token *operator = advance_parser(parser);
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
        Token *operator = advance_parser(parser);
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
        Token *operator = advance_parser(parser);
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
        Token *operator = advance_parser(parser);
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

Statement *init_statement(ExpressionType type, Expression *stmt1, Expression *stmt2)
{
    Statement *new = calloc(1, sizeof(Statement));
    new->type = type;
    switch (type)
    {
    case STMT_EXPR:
    case STMT_PRINT:
        new->expression1 = stmt1;
        new->expression2 = NULL;
        return new;
        break;
    case STMT_VAR:
        new->expression1 = stmt1;
        new->expression2 = stmt2;
        return new;
        break;
    }
    printf("HOW THE FUCK DID YOU GET HERE\n");
}

Statement *printStatement(Parser *parser)
{
    Expression *value = expression(parser);
    // advance_parser(parser);
    consume(parser, SEMICOLON, "Expect ';' after value.");
    Statement *new = init_statement(STMT_PRINT, value, NULL);
    return new;
}

Statement *expressionStatement(Parser *parser)
{
    Expression *expr = expression(parser);
    consume(parser, SEMICOLON, "Expect ';' after value.");
    Statement *new = init_statement(STMT_EXPR, expr, NULL);
    return new;
}

Statement *statement(Parser *parser)
{
    TokenType allowed[] = {PRINT};
    if (match_parser(parser, allowed, 1))
    {
        advance_parser(parser); // Consume PRINT token
        return printStatement(parser);
    }
    return expressionStatement(parser);
}

Statement *varDeclaration(Parser *parser)
{
    advance_parser(parser);
    Token *name = consume(parser, IDENTIFIER, "Expect variable name.");

    Expression *initializer = NULL;
    TokenType allowed = EQUAL;
    if (match_parser(parser, &allowed, 1))
    {
        advance_parser(parser);
        initializer = expression(parser);
        if (error_return_global != 0)
        {
            return NULL;
        }
    }
    else
    {
        Literal *nil_literal = init_literal(NIL, NULL, 1);
        initializer = init_expression_literal(nil_literal, EXPR_LITERAL);
    }
    consume(parser, SEMICOLON, "Expect ';' after variable declaration.");
    Literal *name_literal = name->literal;
    // Literal *name_literal = init_literal(STRING, name->lexeme, 0);
    return init_statement(STMT_VAR, init_expression_literal(name_literal, EXPR_LITERAL), initializer);
}

Statement *declaration(Parser *parser)
{
    TokenType allowed[] = {VAR};
    if (match_parser(parser, allowed, 1))
    {
        return varDeclaration(parser);
    }
    Statement *stmt = statement(parser);
    if (error_return_global != 0)
    {
        synchronize(parser);
        return NULL;
    }
    return stmt;
}

// Array of pointers to parse
Statement **parse(Parser *parser, size_t *len_statements, int *error_return)
{
    size_t size_statements = 128, current_statements = 0;
    Statement **statements = calloc(size_statements, sizeof(Statement *));
    while (!isAtEnd_parser(parser))
    {
        if (current_statements >= size_statements)
        {
            size_statements *= 2;
            statements = realloc(statements, size_statements * (sizeof(Statement *)));
        }
        if (error_return_global != 0)
            break;
        Statement *stmt = declaration(parser);
        statements[current_statements++] = stmt;
    }
    *error_return = error_return_global;
    *len_statements = current_statements;
    return statements;
}

void parenthesize(char *name, Expression *expression)
{
    if (expression->type == EXPR_LITERAL)
    {
        TokenType type = expression->as.literal->token_type;
        switch (type)
        {
        case NUMBER:
            double number = *expression->as.literal->data.number;
            if (floor(number) == number)
            { // integer
                printf("(%s %.1lf)", name, number);
            }
            else
            { // float
                printf("(%s %.15g)", name, number);
            }
            break;
        case STRING:
            printf("(%s %s)", name, expression->as.literal->data.string);
            break;
        case TRUE:
            printf("(%s true)", name);
            break;
        case FALSE:
            printf("(%s false)", name);
            break;
        case NIL:
            printf("(%s nil)", name);
            break;
        default:
            printf("(<unknown literal> ");
            break;
        }
    }
    else if (expression->type == EXPR_GROUPING)
    {
        printf("(%s ", name);
        print_expression(expression);
        printf(")");
    }
    else if (expression->type == EXPR_UNARY)
    {
        printf("(%s ", name);
        print_expression(expression);
        printf(")");
    }
    else if (expression->type == EXPR_BINARY)
    {
        printf("(%s ", name);
        print_expression(expression->as.binary.left);
        printf(" ");
        print_expression(expression->as.binary.right);
        printf(")");
    }
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
        parenthesize(expr->as.binary.operator->lexeme, expr);
        break;

    case EXPR_LITERAL:
        TokenType type = expr->as.literal->token_type;
        switch (type)
        {
        case NUMBER:
            double number = *expr->as.literal->data.number;
            if (floor(number) == number)
            { // integer
                printf("%.1lf", number);
            }
            else
            { // floatvoid print_expression(Expression *expr)
                printf("%.15g", number);
            }
            break;
        case STRING:
            printf("%s", expr->as.literal->data.string);
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
    case EXPR_GROUPING:
        printf("(group ");
        print_expression(expr->as.binary.left); // Directly print inner expression
        printf(")");
        break;
    case EXPR_UNARY:
        parenthesize(expr->as.binary.operator->lexeme, expr->as.binary.right);
        break;

    default:
        printf("<unknown expr> ");
        break;
    }
}

void print_statement(Statement *stmt)
{
    print_expression(stmt->expression1);
    print_expression(stmt->expression2);
    printf("\n");
}

void print_literal(Literal *literal)
{
    switch (literal->token_type)
    {
    case TRUE:
        printf("true\n");
        break;
    case FALSE:
        printf("false\n");
        break;
    case NIL:
        printf("nil\n");
        break;
    case NUMBER:
        if (floor(*literal->data.number) == *literal->data.number)
        {
            printf("%.0lf\n", *literal->data.number);
        }
        else
        {
            printf("%.15g\n", *literal->data.number);
        }
        break;
    case STRING:
        printf("%s\n", literal->data.string);
        break;
    default:
        fprintf(stderr, "print_literal for type %s not implemented yet\n", token_type_to_str(literal->token_type));
    }
}