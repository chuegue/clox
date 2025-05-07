
#include <string.h>
#include <stdlib.h>

#include "scanner.h"

Scanner *init_scanner(char *file_contents)
{
    Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
    scanner->source = strdup(file_contents);
    scanner->tokens = (Token **)calloc(128, sizeof(Token *));
    scanner->size_tokens = 128;
    scanner->start = scanner->current = scanner->number_tokens = scanner->had_error = 0;
    scanner->line = 1;
    return scanner;
}

Literal *init_literal(TokenType type, void *string_number, int bool_nil)
{
    Literal *lit = calloc(1, sizeof(Literal));
    lit->token_type = type;
    switch (type)
    {
    case STRING:
    case IDENTIFIER:
        lit->data.string = string_number;
        break;
    case NUMBER:
        lit->data.number = string_number;
        break;
    case NIL:
        lit->data.null = bool_nil;
        break;
    case TRUE:
    case FALSE:
        lit->data.bool_val = bool_nil;
        break;

    default:
        break;
    }
    return lit;
}

void free_literal(Literal *lit)
{
    switch (lit->token_type)
    {
    case STRING:
        if (lit->data.string)
        {
            //fprintf(stderr, "Freeing string literal %s\n", lit->data.string);
            free(lit->data.string);
            lit->data.string = NULL;
        }
        break;
    case NUMBER:
        if (lit->data.number)
        {
            //fprintf(stderr, "Freeing number literal %lf\n", *lit->data.number);
            free(lit->data.number);
            lit->data.number = NULL;
        }
        break;

    default:
        if (lit->token_type == TRUE || lit->token_type == FALSE)
        {
            //fprintf(stderr, "Freeing bool literal %d\n", lit->data.bool_val);
        }
        else if(lit->token_type == NIL)
        {
            //fprintf(stderr, "Freeing NIL literal %d\n", lit->data.null);
        }
        lit->data.bool_val = -1;
        break;
    }
    free(lit);
}

void free_token(Token *tok)
{
    if (tok == NULL)
    {
        return;
    }
    if (tok->lexeme)
    {
        free(tok->lexeme);
        tok->lexeme = NULL;
    }
    if (tok->literal)
    {
        free_literal(tok->literal);
        tok->literal = NULL;
    }
    free(tok);
}

void free_scanner(Scanner *scanner)
{
    free(scanner->source);
    scanner->source = NULL;
    for (size_t i = 0; i < scanner->size_tokens; i++)
    {
        free_token(scanner->tokens[i]);
        scanner->tokens[i] = NULL;
    }
    free(scanner->tokens);
    scanner->tokens = NULL;
    free(scanner);
}

Token *init_token(char *lexeme, Literal *literal, int line)
{
    Token *token = (Token *)calloc(1, sizeof(Token));
    token->lexeme = lexeme;
    token->literal = literal;
    token->line = line;
    return token;
}

char advance(Scanner *scanner)
{
    return scanner->source[scanner->current++];
}

void addToken(Scanner *scanner, Literal *literal)
{
    char *text;
    if (literal->token_type != EOF_LOX)
    {
        text = calloc(scanner->current - scanner->start + 2, sizeof(char));
        strncpy(text, scanner->source + scanner->start, scanner->current - scanner->start);
    }
    else
    {
        text = calloc(1, sizeof(char));
    }
    if (scanner->number_tokens >= scanner->size_tokens)
    {
        fprintf(stderr, "Not implemented: increase tokens array size\n");
        exit(1);
    }
    scanner->tokens[scanner->number_tokens++] = init_token(text, literal, scanner->line);
}

int match_scanner(Scanner *scanner, char expected)
{
    if (scanner->current >= strlen(scanner->source))
        return 0;
    if (scanner->source[scanner->current] != expected)
        return 0;
    scanner->current++;
    return 1;
}

int isAtEnd_scanner(Scanner *scanner)
{
    return scanner->current >= strlen(scanner->source);
}

char peek_scanner(Scanner *scanner)
{
    if (isAtEnd_scanner(scanner))
    {
        return '\0';
    }
    return scanner->source[scanner->current];
}

void string(Scanner *scanner)
{
    while (peek_scanner(scanner) != '"' && !isAtEnd_scanner(scanner))
    {
        if (peek_scanner(scanner) == '\n')
        {
            scanner->line++;
        }
        advance(scanner);
    }
    if (isAtEnd_scanner(scanner))
    {
        fprintf(stderr, "[line %d] Error: Unterminated string.\n", scanner->line);
        scanner->had_error = 1;
        return;
    }
    advance(scanner);
    char *value = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(value, scanner->source + scanner->start + 1, scanner->current - scanner->start - 2);
    addToken(scanner, init_literal(STRING, value, 0));
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
    while (isDigit(peek_scanner(scanner)))
    {
        advance(scanner);
    }
    if (peek_scanner(scanner) == '.' && isDigit(peekNext(scanner)))
    {
        advance(scanner);
        while (isDigit(peek_scanner(scanner)))
        {
            advance(scanner);
        }
    }
    char *lexeme = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(lexeme, scanner->source + scanner->start, scanner->current - scanner->start);
    double *value = calloc(1, sizeof(double));
    *value = strtod(lexeme, (char **)NULL);
    free(lexeme);
    addToken(scanner, init_literal(NUMBER, value, 0));
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
    while (isAlphanumeric(peek_scanner(scanner)))
    {
        advance(scanner);
    }
    char *value = calloc(scanner->current - scanner->start + 2, sizeof(char));
    strncpy(value, scanner->source + scanner->start, scanner->current - scanner->start);
    TokenType type = get_keyword_type(value);
    addToken(scanner, init_literal(type, value, 0));
    free(value);
    value = NULL;
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
                addToken(scanner, init_literal(LEFT_PAREN, NULL, 0));
                break;
            case ')':
                addToken(scanner, init_literal(RIGHT_PAREN, NULL, 0));
                break;
            case '{':
                addToken(scanner, init_literal(LEFT_BRACE, NULL, 0));
                break;
            case '}':
                addToken(scanner, init_literal(RIGHT_BRACE, NULL, 0));
                break;
            case ',':
                addToken(scanner, init_literal(COMMA, NULL, 0));
                break;
            case '.':
                addToken(scanner, init_literal(DOT, NULL, 0));
                break;
            case '-':
                addToken(scanner, init_literal(MINUS, NULL, 0));
                break;
            case '+':
                addToken(scanner, init_literal(PLUS, NULL, 0));
                break;
            case ';':
                addToken(scanner, init_literal(SEMICOLON, NULL, 0));
                break;
            case '*':
                addToken(scanner, init_literal(STAR, NULL, 0));
                break;
            case '/':
                if (match_scanner(scanner, '/'))
                {
                    while (scanner->source[scanner->current] != '\n' && scanner->current < strlen(scanner->source))
                    {
                        advance(scanner);
                    }
                }
                else
                {
                    addToken(scanner, init_literal(SLASH, NULL, 0));
                }
                break;
            case '"':
                string(scanner);
                break;
            case '!':
                addToken(scanner, init_literal(match_scanner(scanner, '=') ? BANG_EQUAL : BANG, NULL, 0));
                break;
            case '=':
                addToken(scanner, init_literal(match_scanner(scanner, '=') ? EQUAL_EQUAL : EQUAL, NULL, 0));
                break;
            case '<':
                addToken(scanner, init_literal(match_scanner(scanner, '=') ? LESS_EQUAL : LESS, NULL, 0));
                break;
            case '>':
                addToken(scanner, init_literal(match_scanner(scanner, '=') ? GREATER_EQUAL : GREATER, NULL, 0));
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
                addToken(scanner, init_literal(EOF_LOX, NULL, 0));
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
    addToken(scanner, init_literal(EOF_LOX, NULL, 0));
    return scanner;
}

char *token_type_to_str(TokenType type)
{
    char *text;
    switch (type)
    {
    case LEFT_PAREN:
        text = ("LEFT_PAREN");
        break;
    case RIGHT_PAREN:
        text = ("RIGHT_PAREN");
        break;
    case LEFT_BRACE:
        text = ("LEFT_BRACE");
        break;
    case RIGHT_BRACE:
        text = ("RIGHT_BRACE");
        break;
    case COMMA:
        text = ("COMMA");
        break;
    case DOT:
        text = ("DOT");
        break;
    case MINUS:
        text = ("MINUS");
        break;
    case PLUS:
        text = ("PLUS");
        break;
    case SEMICOLON:
        text = ("SEMICOLON");
        break;
    case SLASH:
        text = ("SLASH");
        break;
    case STAR:
        text = ("STAR");
        break;
    case BANG:
        text = ("BANG");
        break;
    case BANG_EQUAL:
        text = ("BANG_EQUAL");
        break;
    case EQUAL:
        text = ("EQUAL");
        break;
    case EQUAL_EQUAL:
        text = ("EQUAL_EQUAL");
        break;
    case LESS:
        text = ("LESS");
        break;
    case LESS_EQUAL:
        text = ("LESS_EQUAL");
        break;
    case GREATER:
        text = ("GREATER");
        break;
    case GREATER_EQUAL:
        text = ("GREATER_EQUAL");
        break;
    case STRING:
        text = ("STRING");
        break;
    case NUMBER:
        text = ("NUMBER");
        break;
    case IDENTIFIER:
        text = ("IDENTIFIER");
        break;
    case AND:
        text = ("AND");
        break;
    case CLASS:
        text = ("CLASS");
        break;
    case ELSE:
        text = ("ELSE");
        break;
    case FALSE:
        text = ("FALSE");
        break;
    case FOR:
        text = ("FOR");
        break;
    case FUN:
        text = ("FUN");
        break;
    case IF:
        text = ("IF");
        break;
    case NIL:
        text = ("NIL");
        break;
    case OR:
        text = ("OR");
        break;
    case PRINT:
        text = ("PRINT");
        break;
    case RETURN:
        text = ("RETURN");
        break;
    case SUPER:
        text = ("SUPER");
        break;
    case THIS:
        text = ("THIS");
        break;
    case TRUE:
        text = ("TRUE");
        break;
    case VAR:
        text = ("VAR");
        break;
    case WHILE:
        text = ("WHILE");
        break;
    case EOF_LOX:
        text = ("EOF");
        break;

    default:
        text = ("NOT_IMPLEMENTED");
        break;
    }
    return text;
}
