#include <stdio.h>

#include "funcs.h"
#include "nothing/nothing.h" 
#include "other/built_in.h"
#include "other/sys/sys.h"
#include "executor/types/type.h"

#define HAS_VARIADIC(func) ((func)->args.count > 0 && (func)->args.items[(func)->args.count - 1].type == VARIADIC_TYPE)

void check_args(Context *ctx, Func *func, AST_Args args) {
    size_t args_count = HAS_VARIADIC(func)? func->args.count - 1 : func->args.count;
            
    if (args.count < args_count) {
        append_error(ctx, ERROR_TOO_FEW_ARGS);
        return;
    }
    
    if (!HAS_VARIADIC(func) && args.count > args_count) {
        append_error(ctx, ERROR_TOO_MANY_ARGS);
        return;
    }

    // TODO: Implement static type checking
    
    /*for (size_t i = 0; i < args_count; ++i) {
        Pattern arg = func->args.items[i];
        AST_Arg ast_arg = args.items[i];
            
        if (sv_cmp_sb(&ast_arg.name, arg.name)) {
            if (!compatible_types(get_type_of_ast(ast_arg.node, arg.type))) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return;
            }
        }
    }*/
}

bool check_arg_name(Func *func, String_View *name_sv) {
    for (size_t i = 0; i < func->args.count; ++i) {
        if (sv_cmp_sb(name_sv, func->args.items[i].name)) {
            return true;
        }
    }

    return false;
}

// TODO: Implement default initializers for function arguments

void unwrap_args(Context *ctx, Func *func, AST_Args args, Var **unwrapped) {
    if (func->args.count <= 0) {
        return;
    }
    
    Var *func_args = (Var*)malloc(sizeof(Var) * func->args.count);

    Value va_args = create_value(VARIADIC_TYPE);
    if (HAS_VARIADIC(func)) {
        va_args.as_ptr = alloc_array_value(ANY_TYPE);
    }

    bool has_named_va_arg = false;
    
    for (size_t i = 0; i < args.count; ++i) {
        if (has_named_va_arg) {
            free(func_args);
            free(va_args.as_ptr);
            append_error(ctx, ERROR_ARGS_AFTER_NAMED_VA_ARG);
            return;
        }
        
        AST_Arg ast_arg = args.items[i];
        
        EvalResult result = execute_expr(ctx, ast_arg.node);
        append_error(ctx, get_signal_error(result.sig));
        if (has_errors(ctx)) {
            free(func_args);
            free(va_args.as_ptr);
            return;
        }

        String_Builder *name_sb = func->args.items[i].name;
        if (ast_arg.has_name) {
            name_sb = sb_alloc();
            sv_to_sb(&ast_arg.name, name_sb);
        }

        String_View sv = {0};
        sv_from_sb(&sv, name_sb);
        
        if (!check_arg_name(func, &sv)) {
            append_error(ctx, ERROR_UNEXPECTED_NAMED_ARG);
            free(func_args);
            free(va_args.as_ptr);
            return;
        }
        
        if (i >= func->args.count - HAS_VARIADIC(func)) {
            Array *va_args_arr = va_args.as_ptr;
            if (result.val.type->kind == TYPE_ARRAY) {
                Array *arr = result.val.as_ptr;
                if (arr->el_type == VARIADIC_TYPE || ast_arg.has_name) {
                    has_named_va_arg = ast_arg.has_name;
                    da_append_many(va_args_arr, arr);
                    continue;
                }
            }
            
            da_append(va_args_arr, result.val);
            continue;
        }

        for (size_t j = 0; j < func->args.count; ++j) {
            Pattern arg = func->args.items[j];
            String_View arg_sv;
            sv_from_sb(&arg_sv, arg.name);
            
            if (sv_cmp_sb(&arg_sv, name_sb)) {
                result.val = cast_value(ctx, result.val, arg.type);
                if (has_errors(ctx)) {
                    return;
                }
            }
        }
        
        func_args[i].symb_kind = SYMB_VAR;
        func_args[i].name = name_sb;
        func_args[i].constant = false;
        func_args[i].is_static = false;
        func_args[i].val = alloc_value(result.val);
    }

    if (va_args.as_ptr != NULL) {
        size_t last = func->args.count - 1;
        func_args[last].name = func->args.items[last].name;
        func_args[last].val = alloc_value(va_args);
        func_args[last].constant = false;
    }
    
    *unwrapped = func_args;
}

static size_t recursion_depth = 0;
static const size_t max_recursion_depth = 512;

Value exec_func(Context *ctx, Func *func, AST_Args args) {
    EvalResult result = create_result(VOID_TYPE);
    
    if (recursion_depth >= max_recursion_depth) {
        append_error(ctx, ERROR_RECURSION_LIMIT_EXCEEDED);
        return result.val;
    }
    
    check_args(ctx, func, args);
    if (has_errors(ctx)) {
        return result.val;
    }

    Var *unwrapped = NULL;
    
    unwrap_args(ctx, func, args, &unwrapped);
    if (has_errors(ctx)) {
        return result.val;
    }
    
    HashMap *local_names = hm_copy(ctx->global->scope.names);
            
    for (size_t i = 0; i < func->args.count; ++i) {
        assert(hm_nput(local_names, unwrapped[i].name->items, unwrapped[i].name->count, &unwrapped[i]) == 0);
    }

    Context local_context = {
        .global = ctx->global,
        .scope = (Scope){
            .names = local_names,
            .types = ctx->scope.types
        },
        .errors = ctx->errors,
        .type_cache = ctx->type_cache,
    };

    ++recursion_depth;
    
    switch (func->kind) {
        case FUNC_BUILT_IN: {
            FuncBuiltIn *built_in = (void*)func;
            result.val = built_in->func(ctx, &local_context);
        } break;

        case FUNC_CUSTOM: {
            FuncCustom *custom_func = (void*)func;
            
            result = execute(&local_context, custom_func->body);
            append_error(ctx, get_signal_error(result.sig));

            result.val = cast_value(ctx, result.val, custom_func->ret_type);
            
            hm_free(local_names);
        } break;
    }

    --recursion_depth;
    
    free(unwrapped);
    return result.val;
}

Value println_func(Context *ctx, Context *fn_ctx) {
    Value val = print_func(ctx, fn_ctx);
    if (has_errors(ctx)) {
        return val;
    }

    printf("\n");
    return val;
}

Value print_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(VOID_TYPE);
    
    Value formated = format_func(ctx, fn_ctx);
    if (has_errors(ctx)) {
        return ret;
    }

    sb_pprint((String_Builder*)formated.as_ptr);
    da_pfree((String_Builder*)formated.as_ptr);
    free(formated.as_ptr);
    return ret;
}

Value format_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(STR_TYPE);
    
    Var *fmt;
    if (!resolve_name_cstr(fn_ctx, "msg", (void*)&fmt)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }
    
    Var *va_args;
    if (!resolve_name_cstr(fn_ctx, "args", (void*)&va_args)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    String_View fmt_sv = {0};
    sv_from_sb(&fmt_sv, fmt->val->as_ptr);

    Array *va_args_arr = (void*)va_args->val->as_ptr;
    
    ret.as_ptr = sb_alloc();
    format_str(ret.as_ptr, ctx, fmt_sv, va_args_arr);
    return ret;
}

Value read_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(CHAR_TYPE);

    Var *intercept;
    if (!resolve_name_cstr(fn_ctx, "intercept", (void*)&intercept)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    if (intercept->val->as_int) {
        set_echo_enabled(false);
    }

    int c = getchar();

    if (intercept->val->as_int) {
        set_echo_enabled(true);
    }

    if (c == EOF) {
        append_error(ctx, ERROR_CLOSED_STDIN);
        ret.as_int = 0;
        return ret;
    }

    ret.as_int = (char)c;
    return ret;
}

Value readln_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(STR_TYPE);
    ret.as_ptr = sb_alloc();

    Var *intercept;
    if (!resolve_name_cstr(fn_ctx, "intercept", (void*)&intercept)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    if (intercept->val->as_int) {
        set_echo_enabled(false);
    }

    if (!sb_getline(ret.as_ptr, stdin)) {
        append_error(ctx, ERROR_CLOSED_STDIN);

        if (intercept->val->as_int) {
            set_echo_enabled(true);
        }

        return ret;
    }

    if (intercept->val->as_int) {
        set_echo_enabled(true);
    }

    return ret;
}

Value read_key_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(CHAR_TYPE);
    ret.as_int = read_key();
    return ret;
}

Value key_pressed_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(BOOL_TYPE);
    ret.as_int = key_pressed();
    return ret;
}

Value clear_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(VOID_TYPE);
    printf("\x1b[2J\x1b[3J\x1b[H");
    fflush(stdout);
    return ret;
}

Value sleep_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(VOID_TYPE);

    Var *ms;
    if (!resolve_name_cstr(fn_ctx, "ms", (void*)&ms)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }
    
    sleep_ms(ms->val->as_int);
    return ret;
}

Value move_cursor_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(VOID_TYPE);

    Var *col;
    if (!resolve_name_cstr(fn_ctx, "column", (void*)&col)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Var *row;
    if (!resolve_name_cstr(fn_ctx, "row", (void*)&row)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }
    
    printf("\x1b[%d;%dH", col->val->as_int, row->val->as_int);
    return ret;
}

typedef String_Builder (*trim_fn)(String_Builder *sb);
Value trim(Context *ctx, Context *fn_ctx, trim_fn trim_fn) {
    Value ret = create_value(STR_TYPE);
    
    Var *str;
    if (!resolve_name_cstr(fn_ctx, "string", (void*)&str)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    String_Builder trimmed = trim_fn(str->val->as_ptr);
    String_Builder *sb = sb_alloc();
    sb->items = trimmed.items;
    sb->count = trimmed.count;
    sb->capacity = trimmed.capacity;
    
    ret.as_ptr = sb;
    return ret;
}

Value trim_func(Context *ctx, Context *fn_ctx) {
    return trim(ctx, fn_ctx, sb_trim);
}

Value trim_left_func(Context *ctx, Context *fn_ctx) {
    return trim(ctx, fn_ctx, sb_trim_left);
}

Value trim_right_func(Context *ctx, Context *fn_ctx) {
    return trim(ctx, fn_ctx, sb_trim_right);
}

Value arr_len_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(INT_TYPE);
    
    Var *arr;
    if (!resolve_name_cstr(fn_ctx, "arr", (void*)&arr)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }
    
    Array *arr_val = arr->val->as_ptr;
    ret.as_int = arr_val->count;

    return ret;
}

Value randint_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(INT_TYPE);
    
    Symbol *min;
    if (!resolve_name_cstr(fn_ctx, "min", &min)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Symbol *max;
    if (!resolve_name_cstr(fn_ctx, "max", &max)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    init_rng();

    Var *min_var = (void*)min;
    Var *max_var = (void*)max;

    int rand_max = max_var->val->as_int - min_var->val->as_int + 1;
    
    if (rand_max == 0) {
        append_error(ctx, ERROR_DIV_BY_ZERO);
        return ret;
    }
    
    ret.as_int = min_var->val->as_int + rand() % (max_var->val->as_int - min_var->val->as_int + 1);
    return ret;
}

Value append_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(VOID_TYPE);

    Symbol *arr_arg;
    if (!resolve_name_cstr(fn_ctx, "arr", &arr_arg)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Symbol *va_args_arg;
    if (!resolve_name_cstr(fn_ctx, "elements", &va_args_arg)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Var *arr_arg_var = (void*)arr_arg;
    Var *va_args_arg_var = (void*)va_args_arg;
    
    Array *arr = arr_arg_var->val->as_ptr;
    Array *va_args = va_args_arg_var->val->as_ptr;

    da_append_many(arr, va_args);
    return ret;
}

Value remove_at_func(Context *ctx, Context *fn_ctx) {
    Value ret = create_value(ANY_TYPE);

    Symbol *arr_arg;
    if (!resolve_name_cstr(fn_ctx, "arr", &arr_arg)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Symbol *index_arg;
    if (!resolve_name_cstr(fn_ctx, "index", &index_arg)) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return ret;
    }

    Var *arr_arg_var = (void*)arr_arg;
    Var *index_arg_var = (void*)index_arg;
    
    Array *arr = arr_arg_var->val->as_ptr;
    INT_CTYPE index = index_arg_var->val->as_int;

    if (index < 0 || index >= arr->count) {
        append_error(ctx, ERROR_OUT_OF_BOUNDS);
        return ret;
    }

    ret = arr->items[index];
    da_remove(arr, index);
    return ret;
}