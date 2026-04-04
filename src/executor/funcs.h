#ifndef FUNCS_H
#define FUNCS_H

#include  "executor.h"
#include "parser/parser.h"

typedef struct {
    Value *args;
    size_t count;
} Variadic;

Value exec_func(Context *context, Func *func, Args args);

Variadic *get_va_args(Context *context);

Value print_func(Context *context, Context *fn_context);
Value println_func(Context *context, Context *fn_context);
Value format_func(Context *context, Context *fn_context);

#endif //FUNCS_H
