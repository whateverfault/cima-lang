#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void) {
    char *source = "let msg = format(\"Hello, {}{'!'}\", \"World\")"
                   "println(msg)";
    
    Context context = {0};
    context_init(&context);
    
    Lexer l = {
        .source = source,
        .source_len = strlen(source),
        .pos = 0,
    };
    
    lexer_init(&l);
    
    AST_NodeProgram *parsed = parse_program(&l);
    execute_program(&context, parsed);

    for (size_t i = 0; i < context.errors->count; ++i) {
        print_error(context.errors->items[i]);
    }

    context_free(&context);
}
