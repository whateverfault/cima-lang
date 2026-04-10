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

#define CSTR_TO_SB(cstr) (String_Builder){       \
    .items=   (cstr),                            \
    .count=   (sizeof(cstr)/sizeof((cstr)[0])-1),\
    .capacity=(sizeof(cstr)/sizeof((cstr)[0])-1) \
}

static String_Builder pi_sb = CSTR_TO_SB("pi");

static String_Builder e_sb = CSTR_TO_SB("e");

static String_Builder nan_sb = CSTR_TO_SB("nan");

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

static String_Builder msg_sb = CSTR_TO_SB("msg");
static String_Builder args_sb = CSTR_TO_SB("args");
static String_Builder string_sb = CSTR_TO_SB("string");

static Pattern print_pattern[] = {
    (Pattern){
        .name = &msg_sb,
        .type = STR_TYPE,
    },
    (Pattern){
        .name = &args_sb,
        .type = VARIADIC_TYPE,
    },
};

static Pattern str_pattern[] = {
    (Pattern){
        .name = &string_sb,
        .type = STR_TYPE,
    },
};

static Pattern empty_pattern[] = {0};

static const Patterns format_patterns = (Patterns){
    .items = print_pattern,
    .count = 2,
    .capacity = 2,
};

static const Patterns str_patterns = (Patterns){
    .items = str_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns empty_patterns = (Patterns){
    .items = empty_pattern,
    .count = 0,
    .capacity = 0,
};

static String_Builder format_sb = CSTR_TO_SB("format");
static String_Builder print_sb = CSTR_TO_SB("print");
static String_Builder println_sb = CSTR_TO_SB("println");
static String_Builder read_sb = CSTR_TO_SB("read");
static String_Builder readln_sb = CSTR_TO_SB("readln");
static String_Builder trim_sb = CSTR_TO_SB("trim");
static String_Builder trim_left_sb = CSTR_TO_SB("trim_left");
static String_Builder trim_right_sb = CSTR_TO_SB("trim_right");

static FuncBuiltIn builtin_funcs[] = {
    (FuncBuiltIn){
        .name = &format_sb,
        .func = format_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &print_sb,
        .func = print_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &println_sb,
        .func = println_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &read_sb,
        .func = read_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &readln_sb,
        .func = readln_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &trim_sb,
        .func = trim_func,
        .args = str_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &trim_left_sb,
        .func = trim_left_func,
        .args = str_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &trim_right_sb,
        .func = trim_right_func,
        .args = str_patterns,
        .constant = true,
    },
};

#define builtin_funcs_count sizeof(builtin_funcs)/sizeof(builtin_funcs[0])

#endif //BUILT_IN_H
