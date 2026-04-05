#include <stdio.h>
#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "executor/executor.h"

void print_error(ErrorKind err) {
    if (err == ERROR_NONE) {
        return;
    }
    
    printf("\nERROR: ");

    switch (err) {
        case ERROR_WRONG_NUMBER_FORMAT: {
            printf("Wrong number format.\n");
        } break;

        case ERROR_VALUE_OUT_OF_RANGE: {
            printf("Value out of range.\n");
        } break;

        case ERROR_DIV_BY_ZERO: {
            printf("Division by zero.\n");
        } break;

        case ERROR_UNMATCHED_PAREN: {
            printf("Unmatched parenthesis.\n");
        } break;

        case ERROR_UNEXPECTED_TOKEN: {
            printf("Unexpected token.\n");
        } break;

        case ERROR_NOT_DEFINED: {
            printf("Identifier is not defined.\n");
        } break;

        case ERROR_CANNOT_ASSIGN_TO_CONST: {
            printf("Cannot assign to constant.\n");
        } break;

        case ERROR_TOO_FEW_ARGS: {
            printf("Too few arguments.\n");
        } break;

        case ERROR_TOO_MANY_ARGS: {
            printf("Too many arguments.\n");
        } break;

        case ERROR_RECURSION_LIMIT_EXCEEDED: {
            printf("Recursion limit exceeded.\n");
        } break;

        case ERROR_INCOMPATIBLE_TYPES: {
            printf("Incompatible types.\n");
        } break;

        case ERROR_UNEXPECTED_NAMED_ARG: {
            printf("Unexpected named argument.\n");
        } break;

        case ERROR_MULTI_CHARACTER_CHAR_LIT: {
            printf("Multi character char literal.\n");
        } break;

        case ERROR_EMPTY_CHAR_LIT: {
            printf("Empty character literal.\n");
        } break;

        case ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT: {
            printf("Format mismatches variadic arguments count.\n");
        } break;

        default: break;
    }
}

void usage(char **argv) {
    fprintf(stderr, "USAGE: %s <path>", argv[0]);
}

int main(int argc, char **argv) {
    String_Builder source_sb = {0};

    if (argc != 2) {
        usage(argv);
        return 1;
    }
    
    if (!read_entire_file(&source_sb, argv[1])) {
        usage(argv);
        return 1;
    }
    
    Context context = {0};
    context_init(&context);
    
    Lexer l = {
        .source = sb_to_sv(source_sb),
        .pos = 0,
    };
    
    lexer_init(&l);
    
    AST_NodeProgram *parsed = parse_program(&l);
    execute_program(&context, parsed);

    for (size_t i = 0; i < context.errors->count; ++i) {
        print_error(context.errors->items[i]);
    }

    context_free(&context);
    da_free(source_sb);
    return 0;
}
