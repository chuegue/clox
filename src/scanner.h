#ifndef __SCANNER__
#define __SCANNER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum 
{
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    STRING,
    NUMBER,
    IDENTIFIER,
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,
    EOF_LOX,
} TokenType;

typedef struct
{
    const char *keyword;
    TokenType type;
} Keyword;

typedef struct
{
    TokenType token_type;
    union
    {
        int bool_val;
        int null;
        char *string;
        double *number;
    } data;
} Literal;

typedef struct Token_
{
    char *lexeme;
    Literal *literal;
    int line;
} Token;

typedef struct Scanner_
{
    char *source;
    Token **tokens; // array of Token*
    size_t number_tokens;
    size_t size_tokens;
    int start;
    int current;
    int line;
    int had_error;
} Scanner;

Scanner *scanToken(char *file_contents);
char *token_type_to_str(TokenType type);
void free_scanner(Scanner *scanner);
void free_token(Token *tok);
Literal *init_literal(TokenType type, void *string_number, int bool_nil);
void free_literal(Literal *lit);

#endif // __SCANNER__