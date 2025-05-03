#include "scanner.h"

Scanner *init_scanner(char *file_contents)
{
    Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
    scanner->source = strdup(file_contents);
    scanner->tokens = (Token *)calloc(128, sizeof(Token));
    scanner->size_tokens = 128;
    scanner->start = scanner->current = scanner->number_tokens = scanner->had_error = 0;
    scanner->line = 1;
    return scanner;
}

void free_scanner(Scanner *scanner)
{
    free(scanner->source);
    for (size_t i = 0; i < scanner->number_tokens; i++)
    {
        free(scanner->tokens[i].lexeme);
        if (scanner->tokens[i].literal != NULL)
        {
            free(scanner->tokens[i].literal);
        }
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

void addToken(Scanner *scanner, TokenType type, void *literal)
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

int match(Scanner *scanner, char expected)
{
    if (scanner->current >= strlen(scanner->source))
        return 0;
    if (scanner->source[scanner->current] != expected)
        return 0;
    scanner->current++;
    return 1;
}

int isAtEnd(Scanner *scanner)
{
    return scanner->current >= strlen(scanner->source);
}

char peek(Scanner *scanner)
{
    if (isAtEnd(scanner))
    {
        return '\0';
    }
    return scanner->source[scanner->current];
}

void string(Scanner *scanner)
{
    while (peek(scanner) != '"' && !isAtEnd(scanner))
    {
        if (peek(scanner) == '\n')
        {
            scanner->line++;
        }
        advance(scanner);
    }
    if (isAtEnd(scanner))
    {
        fprintf(stderr, "[line %d] Error: Unterminated string.\n", scanner->line);
        scanner->had_error = 1;
        return;
    }
    advance(scanner);
    char *value = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(value, scanner->source + scanner->start + 1, scanner->current - scanner->start - 2);
    addToken(scanner, STRING, value);
}

int isDigit(char c)
{
    return c >= '0' && c <= '9';
}

char peekNext(Scanner *scanner)
{
    if (scanner->current + 1 >= strlen(scanner->source))
        return '\0';
    return scanner->source[scanner->current + 1];
}

void number(Scanner *scanner)
{
    while (isDigit(peek(scanner)))
    {
        advance(scanner);
    }
    if (peek(scanner) == '.' && isDigit(peekNext(scanner)))
    {
        advance(scanner);
        while (isDigit(peek(scanner)))
        {
            advance(scanner);
        }
    }
    char *lexeme = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(lexeme, scanner->source + scanner->start, scanner->current - scanner->start);
    double *value = malloc(sizeof(double));
    *value = strtod(lexeme, (char **)NULL);
    addToken(scanner, NUMBER, (void *)value);
}

int isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

int isAlphanumeric(char c)
{
    return isDigit(c) || isAlpha(c);
}

Keyword keywords[] = {
    {"and", AND},
    {"class", CLASS},
    {"else", ELSE},
    {"false", FALSE},
    {"for", FOR},
    {"fun", FUN},
    {"if", IF},
    {"nil", NIL},
    {"or", OR},
    {"print", PRINT},
    {"return", RETURN},
    {"super", SUPER},
    {"this", THIS},
    {"true", TRUE},
    {"var", VAR},
    {"while", WHILE},
};

const int keywordCount = sizeof(keywords) / sizeof(Keyword);

TokenType get_keyword_type(const char *text)
{
    for (int i = 0; i < keywordCount; i++)
    {
        if (strcmp(text, keywords[i].keyword) == 0)
        {
            return keywords[i].type;
        }
    }
    return IDENTIFIER;
}

void identifier(Scanner *scanner)
{
    while (isAlphanumeric(peek(scanner)))
    {
        advance(scanner);
    }
    char *value = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(value, scanner->source + scanner->start, scanner->current - scanner->start);
    TokenType type = get_keyword_type(value);
    addToken(scanner, type, NULL);
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
            case ',':
                addToken(scanner, COMMA, NULL);
                break;
            case '.':
                addToken(scanner, DOT, NULL);
                break;
            case '-':
                addToken(scanner, MINUS, NULL);
                break;
            case '+':
                addToken(scanner, PLUS, NULL);
                break;
            case ';':
                addToken(scanner, SEMICOLON, NULL);
                break;
            case '*':
                addToken(scanner, STAR, NULL);
                break;
            case '/':
                if (match(scanner, '/'))
                {
                    while (scanner->source[scanner->current] != '\n' && scanner->current < strlen(scanner->source))
                    {
                        advance(scanner);
                    }
                }
                else
                {
                    addToken(scanner, SLASH, NULL);
                }
                break;
            case '"':
                string(scanner);
                break;
            case '!':
                addToken(scanner, match(scanner, '=') ? BANG_EQUAL : BANG, NULL);
                break;
            case '=':
                addToken(scanner, match(scanner, '=') ? EQUAL_EQUAL : EQUAL, NULL);
                break;
            case '<':
                addToken(scanner, match(scanner, '=') ? LESS_EQUAL : LESS, NULL);
                break;
            case '>':
                addToken(scanner, match(scanner, '=') ? GREATER_EQUAL : GREATER, NULL);
                break;
            case '#':
            case '$':
            case '@':
            case '%':
                fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", scanner->line, c);
                scanner->had_error = 1;
                break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace.
                break;
            case '\n':
                scanner->line++;
                break;
            case EOF:
                addToken(scanner, EOF_LOX, NULL);
                return scanner;
                break;
            default:
                if (isDigit(c))
                {
                    number(scanner);
                }
                else if (isAlpha(c))
                {
                    identifier(scanner);
                }
                else
                {
                    fprintf(stderr, "Unexpected char: %c\n", c);
                }
            }
        }
    }
    return scanner;
}

#include <string.h>
#include <stdlib.h>

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
    case COMMA:
        text = strdup("COMMA");
        break;
    case DOT:
        text = strdup("DOT");
        break;
    case MINUS:
        text = strdup("MINUS");
        break;
    case PLUS:
        text = strdup("PLUS");
        break;
    case SEMICOLON:
        text = strdup("SEMICOLON");
        break;
    case SLASH:
        text = strdup("SLASH");
        break;
    case STAR:
        text = strdup("STAR");
        break;
    case BANG:
        text = strdup("BANG");
        break;
    case BANG_EQUAL:
        text = strdup("BANG_EQUAL");
        break;
    case EQUAL:
        text = strdup("EQUAL");
        break;
    case EQUAL_EQUAL:
        text = strdup("EQUAL_EQUAL");
        break;
    case LESS:
        text = strdup("LESS");
        break;
    case LESS_EQUAL:
        text = strdup("LESS_EQUAL");
        break;
    case GREATER:
        text = strdup("GREATER");
        break;
    case GREATER_EQUAL:
        text = strdup("GREATER_EQUAL");
        break;
    case STRING:
        text = strdup("STRING");
        break;
    case NUMBER:
        text = strdup("NUMBER");
        break;
    case IDENTIFIER:
        text = strdup("IDENTIFIER");
        break;
    case AND:
        text = strdup("AND");
        break;
    case CLASS:
        text = strdup("CLASS");
        break;
    case ELSE:
        text = strdup("ELSE");
        break;
    case FALSE:
        text = strdup("FALSE");
        break;
    case FOR:
        text = strdup("FOR");
        break;
    case FUN:
        text = strdup("FUN");
        break;
    case IF:
        text = strdup("IF");
        break;
    case NIL:
        text = strdup("NIL");
        break;
    case OR:
        text = strdup("OR");
        break;
    case PRINT:
        text = strdup("PRINT");
        break;
    case RETURN:
        text = strdup("RETURN");
        break;
    case SUPER:
        text = strdup("SUPER");
        break;
    case THIS:
        text = strdup("THIS");
        break;
    case TRUE:
        text = strdup("TRUE");
        break;
    case VAR:
        text = strdup("VAR");
        break;
    case WHILE:
        text = strdup("WHILE");
        break;

    default:
        text = strdup("NOT_IMPLEMENTED");
        break;
    }
    return text;
}
