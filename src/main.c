#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "scanner.h"
#include "parser.h"
#include "interpreter.h"

char *read_file_contents(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error reading file %s: %s\n", filename, strerror(errno));
        exit(1);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < (size_t)file_size)
    {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);

    return file_contents;
}

void print_tokens(Scanner *scanner)
{
    fprintf(stderr, "Number of tokens = %zu\n", scanner->number_tokens);
    for (size_t i = 0; i < scanner->number_tokens; i++)
    {
        Token *token = scanner->tokens[i];
        if (token->literal->token_type == STRING)
        {
            printf("%s %s %s\n", token_type_to_str(token->literal->token_type), token->lexeme, (char *)token->literal->data.string);
        }
        else if (token->literal->token_type == NUMBER)
        {
            double number = *(double *)token->literal->data.number;
            if (floor(number) == number)
            { // integer
                printf("%s %s %.1lf\n", token_type_to_str(token->literal->token_type), token->lexeme, number);
            }
            else
            { // float
                printf("%s %s %.15g\n", token_type_to_str(token->literal->token_type), token->lexeme, number);
            }
        }
        else
        {
            printf("%s %s null\n", token_type_to_str(token->literal->token_type), token->lexeme);
        }
    }
}

int main(int argc, char *argv[])
{
    int error_code = 0;
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    char *file_contents = read_file_contents(argv[2]);
    int debug = 0;

    if (argc >= 4)
    {
        debug = strcmp(argv[3], "-d") == 0;
    }
    if (debug)
    {
        printf("COMMAND: %s\n", command);
    }
    if (strcmp(command, "tokenize") == 0)
    {

        Scanner *scanner = scanToken(file_contents);
        print_tokens(scanner);
        if (scanner->had_error == 1)
        {
            error_code = 65;
        }
        free(file_contents);
        free_scanner(scanner);
    }
    else if (strcmp(command, "parse") == 0)
    {
        Scanner *scanner = scanToken(file_contents);
        print_tokens(scanner);

        if (scanner->number_tokens > 0)
        {
            Parser *parser = init_parser(scanner->tokens, scanner->number_tokens);
            size_t len_statements = 0;
            Statement **statements = parse(parser, &len_statements, &error_code);
            if (error_code != 0)
            {

                free_statements(statements, len_statements);
                free_parser(parser);
                free_scanner(scanner);
                free(file_contents);
                return error_code;
            }
            // for (size_t i = 0; i < len_statements; i++)
            // {
            //     print_statement(statements[i]);
            // }
            free_parser(parser);
            free_statements(statements, len_statements);
        }
        free(file_contents);
        free_scanner(scanner);
    }
    else if (strcmp(command, "run") == 0)
    {
        Scanner *scanner = scanToken(file_contents);
        if (debug)
        {
            print_tokens(scanner);
        }
        if (scanner->number_tokens > 0)
        {
            Parser *parser = init_parser(scanner->tokens, scanner->number_tokens);
            size_t len_statements = 0;
            Statement **statements = parse(parser, &len_statements, &error_code);
            if (error_code != 0)
            {

                free_statements(statements, len_statements);
                free_parser(parser);
                free_scanner(scanner);
                free(file_contents);
                return error_code;
            }
            interpret(statements, len_statements, &error_code);

            free_parser(parser);
            free_statements(statements, len_statements);
        }
        free(file_contents);
        free_scanner(scanner);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    // fprintf(stderr, "Exit code: %d\n", error_code);
    return error_code;
}
