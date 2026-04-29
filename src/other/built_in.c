#include "built_in.h"
#include "executor/executor.h"
#include "executor/funcs/funcs.h"
#include "executor/types/type.h"

static String_Builder msg_sb = CSTR_TO_SB("msg");
static String_Builder args_sb = CSTR_TO_SB("args");
static String_Builder string_sb = CSTR_TO_SB("string");
static String_Builder intercept_sb = CSTR_TO_SB("intercept");
static String_Builder column_sb = CSTR_TO_SB("column");
static String_Builder row_sb = CSTR_TO_SB("row");
static String_Builder arr_sb = CSTR_TO_SB("arr");
static String_Builder min_sb = CSTR_TO_SB("min");
static String_Builder max_sb = CSTR_TO_SB("max");
static String_Builder els_sb = CSTR_TO_SB("elements");
static String_Builder index_sb = CSTR_TO_SB("index");
static String_Builder ms_sb = CSTR_TO_SB("ms");

static Pattern format_pattern[] = {
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

static Pattern move_cursor_pattern[] = {
    (Pattern){
        .name = &column_sb,
        .type = INT_TYPE,
    },
    (Pattern){
        .name = &row_sb,
        .type = INT_TYPE,
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

static Pattern sleep_pattern[] = {
    (Pattern){
        .name = &ms_sb,
        .type = INT_TYPE,
    },
};

static const Pattern empty_pattern[] = {0};

static const Patterns format_patterns = (Patterns){
    .items = format_pattern,
    .count = 2,
    .capacity = 2,
};

static const Patterns read_patterns = (Patterns){
    .items = read_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns move_cursor_patterns = (Patterns){
    .items = move_cursor_pattern,
    .count = 2,
    .capacity = 2,
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

static const Patterns sleep_patterns = (Patterns){
    .items = sleep_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns empty_patterns = (Patterns){
    .items = (Pattern*)empty_pattern,
    .count = 0,
    .capacity = 0,
};

static String_Builder format_sb = CSTR_TO_SB("format");
static String_Builder print_sb = CSTR_TO_SB("print");
static String_Builder println_sb = CSTR_TO_SB("println");
static String_Builder read_sb = CSTR_TO_SB("read");
static String_Builder readln_sb = CSTR_TO_SB("readln");
static String_Builder readkey_sb = CSTR_TO_SB("readkey");
static String_Builder key_pressed_sb = CSTR_TO_SB("key_pressed");
static String_Builder clear_sb = CSTR_TO_SB("clear");
static String_Builder sleep_sb = CSTR_TO_SB("sleep");
static String_Builder move_cursor_sb = CSTR_TO_SB("move_cursor");
static String_Builder trim_sb = CSTR_TO_SB("trim");
static String_Builder trim_left_sb = CSTR_TO_SB("trim_left");
static String_Builder trim_right_sb = CSTR_TO_SB("trim_right");
static String_Builder len_sb = CSTR_TO_SB("len");
static String_Builder randint_sb = CSTR_TO_SB("randint");
static String_Builder append_sb = CSTR_TO_SB("append");
static String_Builder remove_at_sb = CSTR_TO_SB("remove_at");

FuncBuiltIn builtin_funcs[builtin_funcs_count] = {
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &format_sb,
        .func = format_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &print_sb,
        .func = print_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &println_sb,
        .func = println_func,
        .args = format_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &read_sb,
        .func = read_func,
        .args = read_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &readln_sb,
        .func = readln_func,
        .args = read_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &readkey_sb,
        .func = read_key_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &key_pressed_sb,
        .func = key_pressed_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &clear_sb,
        .func = clear_func,
        .args = empty_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &sleep_sb,
        .func = sleep_func,
        .args = sleep_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &move_cursor_sb,
        .func = move_cursor_func,
        .args = move_cursor_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &trim_sb,
        .func = trim_func,
        .args = str_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &trim_left_sb,
        .func = trim_left_func,
        .args = str_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &trim_right_sb,
        .func = trim_right_func,
        .args = str_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &len_sb,
        .func = arr_len_func,
        .args = len_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &append_sb,
        .func = append_func,
        .args = append_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &remove_at_sb,
        .func = remove_at_func,
        .args = remove_at_patterns,
        .constant = true,
    },
    (FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &randint_sb,
        .func = randint_func,
        .args = randint_patterns,
        .constant = true,
    },
};

Type *builtin_types[builtin_types_count] = {
    INT_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    STR_TYPE,
    CHAR_TYPE,
    ANY_TYPE,
    VOID_TYPE,
    VARIADIC_TYPE,
};