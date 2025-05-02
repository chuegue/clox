#include "scanner.h"

Scanner *init_scanner(char *file_contents)
{
    Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
    scanner->source = strdup(file_contents);
    scanner->tokens = (Token *)calloc(128, sizeof(Token));
    scanner->size_tokens = 128;
    scanner->start = scanner->current = scanner->number_tokens = 0;
    scanner->line = 1;
    return scanner;
}

void free_scanner(Scanner *scanner)
{
    free(scanner->source);
    for (size_t i = 0; i < scanner->number_tokens; i++)
    {
        free(scanner->tokens[i].lexeme);
    }
    free(scanner->tokens);
    free(scanner);
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

char advance(Scanner *scanner)
{
    return scanner->source[scanner->current++];
}

void addToken(Scanner *scanner, TokenType type, void *literal /*temp i guess*/)
{
    char *text = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(text, scanner->source + scanner->start, scanner->current - scanner->start);
    // fprintf(stderr, "%d %d\n", scanner->start, scanner->current);
    if (scanner->number_tokens >= scanner->size_tokens)
    {
        fprintf(stderr, "Not implemented: increase tokens array size\n");
        exit(1);
    }
    memcpy(&(scanner->tokens[scanner->number_tokens++]), init_token(type, text, literal, scanner->line), sizeof(Token));
}

Scanner *scanToken(char *file_contents)
{
    Scanner *scanner = init_scanner(file_contents);
    size_t file_len = strlen(file_contents);
    if (file_len > 0)
    {
        while (scanner->current < file_len)
        {
            scanner->start = scanner->current;
            char c = advance(scanner);
            switch (c)
            {
            case '(':
                addToken(scanner, LEFT_PAREN, NULL);
                break;
            case ')':
                addToken(scanner, RIGHT_PAREN, NULL);
                break;
            case '{':
                addToken(scanner, LEFT_BRACE, NULL);
                break;
            case '}':
                addToken(scanner, RIGHT_BRACE, NULL);
                break;

            default:
                fprintf(stderr, "Unexpected char: %s\n", c);
            }
        }
        // implement EOF token
    }
    return scanner;
}

char *token_type_to_str(TokenType type)
{
    char *text;
    switch (type)
    {
    case LEFT_PAREN:
        text = strdup("LEFT_PAREN");
        break;
    case RIGHT_PAREN:
        text = strdup("RIGHT_PAREN");
        break;
    case LEFT_BRACE:
        text = strdup("LEFT_BRACE");
        break;
    case RIGHT_BRACE:
        text = strdup("RIGHT_BRACE");
        break;
    default:
        text = strdup("NOT_IMPLEMENTED");
        break;
    }
}