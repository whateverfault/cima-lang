#ifndef BUILT_IN_H
#define BUILT_IN_H

#include "executor/executor.h"
#include "executor/funcs.h"

static Var builtin_vars[] = {
    (Var){
        .name = (String_Builder){
            .items = "pi",
            .count = 2,
            .capacity = 2,
        },
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = M_PI,
        },
        .constant = true,
    },
    (Var){
        .name = (String_Builder){
            .items = "e",
            .count = 1,
            .capacity = 1,
        },
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = M_E,
        },
        .constant = true,
    },
    (Var){
        .name = (String_Builder){
            .items = "nan",
            .count = 3,
            .capacity = 3,
        },
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = NAN,
        },
        .constant = true,
    },
};

#define builtin_vars_count sizeof(builtin_vars)/sizeof(builtin_vars[0])

static Pattern print_pattern[] = {
    (Pattern){
        .name = (String_Builder){
            .items = "msg",
            .count = 3,
        },
        .type = STR_TYPE,
    },
    (Pattern){
        .name = (String_Builder){
            .items = "...",
            .count = 3,
        },
        .type = VARIADIC_TYPE,
    },
};

static const Patterns format_patterns = (Patterns){
    .items = print_pattern,
    .count = 2,
    .capacity = 2,
};

static FuncBuiltIn builtin_funcs[] = {
    (FuncBuiltIn){
        .name = (String_Builder){
            .items = "format",
            .count = 6,
        },
        .func = format_func,
        .args = format_patterns,
    },
    (FuncBuiltIn){
        .name = (String_Builder){
            .items = "print",
            .count = 5,
        },
        .func = print_func,
        .args = format_patterns,
    },
    (FuncBuiltIn){
        .name = (String_Builder){
            .items = "println",
            .count = 7,
        },
        .func = println_func,
        .args = format_patterns,
    },
};

#define builtin_funcs_count sizeof(builtin_funcs)/sizeof(builtin_funcs[0])

#endif //BUILT_IN_H
