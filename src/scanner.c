#include "scanner.h"

Scanner *tokenize(char *file_contents)
{
    Scanner *scanner = init_scanner(file_contents);
    size_t file_len = strlen(file_contents);
    if (file_len > 0)
    {
        while(scanner->current < file_len);
        scanToken(scanner);
    }

    //implement EOF token
    return scanner;
}

Scanner *init_scanner(char *file_contents)
{
    Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
    scanner->source = strdup(file_contents);
    scanner->tokens = (Token *)calloc(128, sizeof(Token));
    scanner->size_tokens = 128;
    scanner->start = scanner->current = scanner->index_tokens = 0;
    scanner->line = 1;
    return scanner;
}

Token *init_token(TokenType type, char *lexeme, void *literal, int line)
{
    Token *token = (Token *)calloc(1, sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->literal = literal;
    token->line = line;
    return token;
}

void scanToken(Scanner *scanner)
{
    char c = advance(scanner);
    switch (c)
    {
    case '(':
        addToken(scanner, LEFT_PAREN, NULL);
        break;
    case ')':
        addToken(scanner, RIGHT_PAREN, NULL);
        break;

    default:
        fprintf(stderr, "Unexpected char: %s\n", c);
    }
}

char advance(Scanner *scanner)
{
    return scanner->source[scanner->current++];
}

void addToken(Scanner *scanner, TokenType type, void *literal /*temp i guess*/)
{
    char *text = calloc(scanner->current - scanner->start + 2);
    strncpy(text, scanner->source + scanner->start, scanner->current - scanner->start);
    if (scanner->index_tokens >= scanner->size_tokens)
    {
        fprintf(stderr, "Not implemented: increase tokens array size\n");
        exit(1);
    }
    memcpy(&(scanner->tokens[scanner->index_tokens]), init_token(type, text, literal, scanner->line), sizeof(Token));
}