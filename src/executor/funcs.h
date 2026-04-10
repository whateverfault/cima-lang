#ifndef FUNCS_H
#define FUNCS_H

#include "parser/parser.h"
#include "executor.h"

typedef Value (*func_t)(Context *context, Context *fn_context);

typedef enum {
    FUNC_BUILT_IN,
    FUNC_CUSTOM,
} FuncKind;

typedef struct Func {
    FUNC_FIELDS
} Func;

typedef struct {
    FUNC_FIELDS
    func_t func;
} FuncBuiltIn;

typedef struct {
    FUNC_FIELDS
    AST_Node *body;
} FuncCustom;

Value exec_func(Context *context, Func *func, Args args);

Variadic *get_va_args(Context *context);

Value print_func(Context *context, Context *fn_context);
Value println_func(Context *context, Context *fn_context);
Value format_func(Context *context, Context *fn_context);

Value read_func(Context *context, Context *fn_context);
Value readln_func(Context *context, Context *fn_context);

Value trim_func(Context *context, Context *fn_context);
Value trim_left_func(Context *context, Context *fn_context);
Value trim_right_func(Context *context, Context *fn_context);

Value arr_len_func(Context *context, Context *fn_context);

#endif //FUNCS_H
