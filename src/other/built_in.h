#ifndef BUILT_IN_H
#define BUILT_IN_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#include "executor/executor.h"
#include "executor/funcs.h"

static String_Builder pi_sb = {
    .items = "pi",
    .count = 2,
    .capacity = 2,
};

static String_Builder e_sb = {
    .items = "e",
    .count = 1,
    .capacity = 1,
};

static String_Builder nan_sb = {
    .items = "nan",
    .count = 3,
    .capacity = 3,
};

static Var builtin_vars[] = {
    (Var){
        .name = &pi_sb,
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = M_PI,
        },
        .constant = true,
    },
    (Var){
        .name = &e_sb,
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = M_E,
        },
        .constant = true,
    },
    (Var){
        .name = &nan_sb,
        .val = (Value){
            .type = FLOAT_TYPE,
            .as_float = NAN,
        },
        .constant = true,
    },
};

#define builtin_vars_count sizeof(builtin_vars)/sizeof(builtin_vars[0])

static String_Builder msg_sb = {
    .items = "msg",
    .count = 3,
    .capacity = 3,
};

static String_Builder ellipsis_sb = {
    .items = "...",
    .count = 3,
    .capacity = 3,
};

static Pattern print_pattern[] = {
    (Pattern){
        .name = &msg_sb,
        .type = STR_TYPE,
    },
    (Pattern){
        .name = &ellipsis_sb,
        .type = VARIADIC_TYPE,
    },
};

static const Patterns format_patterns = (Patterns){
    .items = print_pattern,
    .count = 2,
    .capacity = 2,
};

static String_Builder format_sb = {
    .items = "format",
    .count = 6,
    .capacity = 6,
};

static String_Builder print_sb = {
    .items = "print",
    .count = 5,
    .capacity = 5,
};

static String_Builder println_sb = {
    .items = "println",
    .count = 7,
    .capacity = 7,
};

static FuncBuiltIn builtin_funcs[] = {
    (FuncBuiltIn){
        .name = &format_sb,
        .func = format_func,
        .args = format_patterns,
    },
    (FuncBuiltIn){
        .name = &print_sb,
        .func = print_func,
        .args = format_patterns,
    },
    (FuncBuiltIn){
        .name = &println_sb,
        .func = println_func,
        .args = format_patterns,
    },
};

#define builtin_funcs_count sizeof(builtin_funcs)/sizeof(builtin_funcs[0])

#endif //BUILT_IN_H
