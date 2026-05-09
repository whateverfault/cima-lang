#include "built_in.h"
#include "executor/executor.h"
#include "executor/funcs/funcs.h"
#include "executor/types/type.h"

static int32_t fmt_utf32[] = {'f', 'm', 't'};
static UnicodeStringBuilder fmt_sb = CSTR_TO_SB_UNICODE(fmt_utf32);

static int32_t args_utf32[] = { 'a','r','g','s' };
static UnicodeStringBuilder args_sb = CSTR_TO_SB_UNICODE(args_utf32);

static int32_t string_utf32[] = { 's','t','r','i','n','g' };
static UnicodeStringBuilder string_sb = CSTR_TO_SB_UNICODE(string_utf32);

static int32_t intercept_utf32[] = { 'i','n','t','e','r','c','e','p','t' };
static UnicodeStringBuilder intercept_sb = CSTR_TO_SB_UNICODE(intercept_utf32);

static int32_t column_utf32[] = { 'c','o','l','u','m','n' };
static UnicodeStringBuilder column_sb = CSTR_TO_SB_UNICODE(column_utf32);

static int32_t row_utf32[] = { 'r','o','w' };
static UnicodeStringBuilder row_sb = CSTR_TO_SB_UNICODE(row_utf32);

static int32_t arr_utf32[] = { 'a','r','r' };
static UnicodeStringBuilder arr_sb = CSTR_TO_SB_UNICODE(arr_utf32);

static int32_t str_utf32[] = { 's','t','r' };
static UnicodeStringBuilder str_sb = CSTR_TO_SB_UNICODE(str_utf32);

static int32_t min_utf32[] = { 'm','i','n' };
static UnicodeStringBuilder min_sb = CSTR_TO_SB_UNICODE(min_utf32);

static int32_t max_utf32[] = { 'm','a','x' };
static UnicodeStringBuilder max_sb = CSTR_TO_SB_UNICODE(max_utf32);

static int32_t elements_utf32[] = { 'e','l','e','m','e','n','t','s' };
static UnicodeStringBuilder els_sb = CSTR_TO_SB_UNICODE(elements_utf32);

static int32_t index_utf32[] = { 'i','n','d','e','x' };
static UnicodeStringBuilder index_sb = CSTR_TO_SB_UNICODE(index_utf32);

static int32_t ms_utf32[] = { 'm','s' };
static UnicodeStringBuilder ms_sb = CSTR_TO_SB_UNICODE(ms_utf32);

static Pattern format_pattern[] = {
    (Pattern){
        .name = &fmt_sb,
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

static Pattern arr_len_pattern[] = {
    (Pattern){
        .name = &arr_sb,
        .type = ARRAY_ANY_TYPE,
    },
};

static Pattern str_len_pattern[] = {
    (Pattern){
        .name = &str_sb,
        .type = STR_TYPE,
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

static const Patterns arr_len_patterns = (Patterns){
    .items = arr_len_pattern,
    .count = 1,
    .capacity = 1,
};

static const Patterns str_len_patterns = (Patterns){
    .items = str_len_pattern,
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

static int32_t format_utf32[] = {'f','o','r','m','a','t'};
static UnicodeStringBuilder format_sb = CSTR_TO_SB_UNICODE(format_utf32);

static int32_t print_utf32[] = {'p','r','i','n','t'};
static UnicodeStringBuilder print_sb = CSTR_TO_SB_UNICODE(print_utf32);

static int32_t println_utf32[] = {'p','r','i','n','t','l','n'};
static UnicodeStringBuilder println_sb = CSTR_TO_SB_UNICODE(println_utf32);

static int32_t read_utf32[] = {'r','e','a','d'};
static UnicodeStringBuilder read_sb = CSTR_TO_SB_UNICODE(read_utf32);

static int32_t readln_utf32[] = {'r','e','a','d','l','n'};
static UnicodeStringBuilder readln_sb = CSTR_TO_SB_UNICODE(readln_utf32);

static int32_t readkey_utf32[] = {'r','e','a','d','k','e','y'};
static UnicodeStringBuilder readkey_sb = CSTR_TO_SB_UNICODE(readkey_utf32);

static int32_t key_pressed_utf32[] = {'k','e','y','_','p','r','e','s','s','e','d'};
static UnicodeStringBuilder key_pressed_sb = CSTR_TO_SB_UNICODE(key_pressed_utf32);

static int32_t clear_utf32[] = {'c','l','e','a','r'};
static UnicodeStringBuilder clear_sb = CSTR_TO_SB_UNICODE(clear_utf32);

static int32_t sleep_utf32[] = {'s','l','e','e','p'};
static UnicodeStringBuilder sleep_sb = CSTR_TO_SB_UNICODE(sleep_utf32);

static int32_t move_cursor_utf32[] = {'m','o','v','e','_','c','u','r','s','o','r'};
static UnicodeStringBuilder move_cursor_sb = CSTR_TO_SB_UNICODE(move_cursor_utf32);

static int32_t trim_utf32[] = {'t','r','i','m'};
static UnicodeStringBuilder trim_sb = CSTR_TO_SB_UNICODE(trim_utf32);

static int32_t trim_left_utf32[] = {'t','r','i','m','_','l','e','f','t'};
static UnicodeStringBuilder trim_left_sb = CSTR_TO_SB_UNICODE(trim_left_utf32);

static int32_t trim_right_utf32[] = {'t','r','i','m','_','r','i','g','h','t'};
static UnicodeStringBuilder trim_right_sb = CSTR_TO_SB_UNICODE(trim_right_utf32);

static int32_t len_utf32[] = {'l','e','n'};
static UnicodeStringBuilder len_sb = CSTR_TO_SB_UNICODE(len_utf32);

static int32_t randint_utf32[] = {'r','a','n','d','i','n','t'};
static UnicodeStringBuilder randint_sb = CSTR_TO_SB_UNICODE(randint_utf32);

static int32_t append_utf32[] = {'a','p','p','e','n','d'};
static UnicodeStringBuilder append_sb = CSTR_TO_SB_UNICODE(append_utf32);

static int32_t remove_at_utf32[] = {'r','e','m','o','v','e','_','a','t'};
static UnicodeStringBuilder remove_at_sb = CSTR_TO_SB_UNICODE(remove_at_utf32);

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
        .args = arr_len_patterns,
        .constant = true,
    },
    /*(FuncBuiltIn){
        .symb_kind = SYMB_FUNC,
        .name = &len_sb,
        .func = str_len_func,
        .args = str_len_patterns,
        .constant = true,
    },*/
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