#ifndef __SCANNER__
#define __SCANNER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Scanner_ {
    char *source;
    Token *tokens;
    size_t index_tokens;
    size_t size_tokens;
    int start;
    int current;
    int line;
} Scanner;

typedef struct Token_ {
    TokenType type;
    char *lexeme;
    void* literal; //temp line
    int line;
} Token;

typedef enum TokenType_
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
} TokenType;

Scanner *tokenize(char *file_contents);

#endif // __SCANNER__