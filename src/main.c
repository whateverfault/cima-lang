#include <stdio.h>
#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "executor/error.h"
#include "executor/executor.h"

void print_parser_error(ParserError err) {
    if (err == PERROR_NONE) {
        return;
    }

    printf("\nPARSER ERROR: ");

    switch (err) {
        case PERROR_WRONG_NUMBER_FORMAT: {
            printf("Wrong number format.\n");
        } break;

        case PERROR_VALUE_OUT_OF_RANGE: {
            printf("Value out of range.\n");
        } break;

        case PERROR_UNMATCHED_PAREN: {
            printf("Unmatched parenthesis.\n");
        } break;

        case PERROR_UNEXPECTED_TOKEN: {
            printf("Unexpected token.\n");
        } break;

        case PERROR_UNEXPECTED_EOF: {
            printf("Unexpected end of file.\n");
        } break;

        case PERROR_MULTI_CHARACTER_CHAR_LIT: {
            printf("Multi character char literal.\n");
        } break;

        case PERROR_EMPTY_CHAR_LIT: {
            printf("Empty character literal.\n");
        } break;

        case PERROR_WRONG_FIELD_NAME_FORMAT: {
            printf("Wrong initializer name format.\n");
        } break;

        case PERROR_WRONG_INITIALIZER_OBJECT: {
            printf("Wrong initializer object.\n");
        } break;

        case PERROR_MULTIPLE_INITIALIZERS: {
            printf("Multiple initializers for one field.\n");
        } break;

        case PERROR_MULTIPLE_ARGS: {
            printf("Multiple arguments for one function parameter.\n");
        } break;

        case PERROR_ARGS_AFTER_VA_ARG: {
            printf("Arguments after variadic.\n");
        } break;
            
        default: assert(0 && "UNREACHABLE");
    }
}

void print_runtime_error(RuntimeError err) {
    if (err == ERROR_NONE) {
        return;
    }
    
    printf("\nRUNTIME ERROR: ");

    switch (err) {
        case ERROR_DIV_BY_ZERO: {
            printf("Division by zero.\n");
        } break;

        case ERROR_UNEXPECTED_CONTINUE: {
            printf("Unexpected continue signal.\n");
        } break;

        case ERROR_UNEXPECTED_BREAK: {
            printf("Unexpected break signal.\n");
        } break;

        case ERROR_UNEXPECTED_RETURN: {
            printf("Unexpected return signal.\n");
        } break;

        case ERROR_NOT_DEFINED: {
            printf("Identifier is not defined.\n");
        } break;

        case ERROR_CANNOT_ASSIGN_TO_CONST: {
            printf("Cannot assign to constant.\n");
        } break;

        case ERROR_CANNOT_REASSIGN_CONST: {
            printf("Cannot reassign constant.\n");
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

        case ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT: {
            printf("Format mismatches variadic arguments count.\n");
        } break;
        
        case ERROR_CLOSED_STDIN: {
            printf("Stdin closed.\n");
        } break;

        case ERROR_INDEX_OUT_OF_BOUNDS: {
            printf("Index out of bounds.\n");
        } break;

        case ERROR_CONTINUE_OUTSIDE_LOOP: {
            printf("Continue signal outside a loop.\n");
        } break;
        
        case ERROR_BREAK_OUTSIDE_LOOP: {
            printf("Break signal outside a loop.\n");
        } break;

        case ERROR_UNKNOWN_TYPE: {
            printf("Unknown type.\n");
        } break;

        case ERROR_UNKNOWN_FIELD: {
            printf("Unknown field.\n");
        } break;

        case ERROR_WRONG_FORMAT_STR: {
            printf("Wrong format string.\n");
        } break;

        case ERROR_MULTIPLE_INITIALIZERS: {
            printf("Multiple initializers for one field.\n");
        } break;

        case ERROR_ARGS_AFTER_NAMED_VA_ARG: {
            printf("Arguments after named variadic.\n");
        } break;
            
        default: assert(0 && "UNREACHABLE");
    }
}

void usage(char **argv) {
    fprintf(stderr, "USAGE: %s <path>", argv[0]);
}

// TODO: Implement garbage collector so no memory is leaked on structure reassignment

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
    
    Context ctx = {0};
    global_ctx_init(&ctx);

    String_View source_sv = {0};
    sv_from_sb(&source_sv, &source_sb);
    
    Lexer l = {
        .source = source_sv,
        .pos = 0,
    };
    
    lexer_init(&l);

    AST_NodeProgram *program;
    ParserError err = parse_program(&l, &program);
    if (err != PERROR_NONE) {
        print_parser_error(err);
        ctx_free(&ctx);
        da_free(source_sb);
        return 0;
    }
    
    execute_program(&ctx, program);

    for (size_t i = 0; i < ctx.errors->count; ++i) {
        print_runtime_error(ctx.errors->items[i]);
    }

    ctx_free(&ctx);
    da_free(source_sb);
    return 0;
}
