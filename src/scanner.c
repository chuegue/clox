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

int match(Scanner *scanner, char expected)
{
    if (scanner->current >= strlen(scanner->source))
        return 0;
    if (scanner->source[scanner->current] != expected)
        return 0;
    scanner->current++;
    return 1;
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
    case SLASH:
        text = strdup("SLASH");
        break;

    default:
        text = strdup("NOT_IMPLEMENTED");
        break;
    }
}