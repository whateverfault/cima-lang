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
static String_Builder intercept_sb = CSTR_TO_SB("intercept");
static String_Builder arr_sb = CSTR_TO_SB("arr");
static String_Builder min_sb = CSTR_TO_SB("min");
static String_Builder max_sb = CSTR_TO_SB("max");
static String_Builder els_sb = CSTR_TO_SB("elements");
static String_Builder index_sb = CSTR_TO_SB("index");

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

static Pattern read_pattern[] = {
    (Pattern){
        .name = &intercept_sb,
        .type = BOOL_TYPE,
    },
};

static Pattern str_pattern[] = {
    (Pattern){
        .name = &string_sb,
        .type = STR_TYPE,
    },
};

static Pattern len_pattern[] = {
    (Pattern){
        .name = &arr_sb,
        .type = ARRAY_ANY_TYPE,
    },
};

static Pattern randint_pattern[] = {
    (Pattern){
        .name = &min_sb,
        .type = INT_TYPE,
    },
    (Pattern){
        .name = &max_sb,
        .type = INT_TYPE,
    },
};

static Pattern append_pattern[] = {
    (Pattern){
        .name = &arr_sb,
        .type = ARRAY_ANY_TYPE,
    },
    (Pattern){
        .name = &els_sb,
        .type = VARIADIC_TYPE,
    },
};

static Pattern remove_at_pattern[] = {
    (Pattern){
        .name = &arr_sb,
        .type = ARRAY_ANY_TYPE,
    },
    (Pattern){
        .name = &index_sb,
        .type = INT_TYPE,
    },
};

static Pattern empty_pattern[] = {0};

static const Patterns format_patterns = (Patterns){
    .items = print_pattern,
    .count = 2,
    .capacity = 2,
};

static const Patterns read_patterns = (Patterns){
    .items = read_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns str_patterns = (Patterns){
    .items = str_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns len_patterns = (Patterns){
    .items = len_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns randint_patterns = (Patterns){
    .items = randint_pattern,
    .count = 2,
    .capacity = 2,
};

static const Patterns append_patterns = (Patterns){
    .items = append_pattern,
    .count = 2,
    .capacity = 2,
};

static const Patterns remove_at_patterns = (Patterns){
    .items = remove_at_pattern,
    .count = 2,
    .capacity = 2,
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
static String_Builder read_key_sb = CSTR_TO_SB("read_key");
static String_Builder trim_sb = CSTR_TO_SB("trim");
static String_Builder trim_left_sb = CSTR_TO_SB("trim_left");
static String_Builder trim_right_sb = CSTR_TO_SB("trim_right");
static String_Builder len_sb = CSTR_TO_SB("len");
static String_Builder clear_sb = CSTR_TO_SB("clear");
static String_Builder randint_sb = CSTR_TO_SB("randint");
static String_Builder append_sb = CSTR_TO_SB("append");
static String_Builder remove_at_sb = CSTR_TO_SB("remove_at");

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
        .args = read_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &readln_sb,
        .func = readln_func,
        .args = read_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &read_key_sb,
        .func = read_key_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &clear_sb,
        .func = clear_func,
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
    (FuncBuiltIn){
        .name = &len_sb,
        .func = arr_len_func,
        .args = len_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &append_sb,
        .func = append_func,
        .args = append_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &remove_at_sb,
        .func = remove_at_func,
        .args = remove_at_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .name = &randint_sb,
        .func = randint_func,
        .args = randint_patterns,
        .constant = true,
    },
};

#define builtin_funcs_count sizeof(builtin_funcs)/sizeof(builtin_funcs[0])

#endif //BUILT_IN_H
