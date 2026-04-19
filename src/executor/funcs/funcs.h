#ifndef FUNCS_H
#define FUNCS_H

#include "parser/parser.h"
#include "../executor.h"



Value exec_func(Context *ctx, Func *func, AST_Args args);

Value print_func(Context *ctx, Context *fn_ctx);
Value println_func(Context *ctx, Context *fn_ctx);
Value format_func(Context *ctx, Context *fn_ctx);

Value read_func(Context *ctx, Context *fn_ctx);
Value readln_func(Context *ctx, Context *fn_ctx);
Value read_key_func(Context *ctx, Context *fn_ctx);

Value clear_func(Context *ctx, Context *fn_ctx);
Value move_cursor_func(Context *ctx, Context *fn_ctx);

Value trim_func(Context *ctx, Context *fn_ctx);
Value trim_left_func(Context *ctx, Context *fn_ctx);
Value trim_right_func(Context *ctx, Context *fn_ctx);

Value arr_len_func(Context *ctx, Context *fn_ctx);
Value append_func(Context *ctx, Context *fn_ctx);
Value remove_at_func(Context *ctx, Context *fn_ctx);

Value randint_func(Context *ctx, Context *fn_ctx);

#endif //FUNCS_H
