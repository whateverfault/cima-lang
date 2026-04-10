#include <stdio.h>

#include "funcs.h"
#include "nothing/nothing.h" 
#include "other/built_in.h"

#define HAS_VARIADIC(func) ((func)->args.count > 0 && (func)->args.items[(func)->args.count - 1].type->tag == TYPE_VARIADIC)

void check_args(Context *context, Func *func, Args args) {
    // TODO: Proper type checking
    size_t args_count = HAS_VARIADIC(func)? func->args.count - 1 : func->args.count;
            
    if (args.count < args_count) {
        append_error(context, ERROR_TOO_FEW_ARGS);
        return;
    }
    
    if (!HAS_VARIADIC(func) && args.count > args_count) {
        append_error(context, ERROR_TOO_MANY_ARGS);
        return;
    }
}

bool check_arg_name(Func *func, String_View *name_sv) {
    for (size_t i = 0; i < func->args.count; ++i) {
        if (sv_cmp_sb(name_sv, func->args.items[i].name)) {
            return true;
        }
    }

    return false;
}

void unwrap_args(Context *context, Func *func, Args args, Var **unwrapped) {
    Var *func_args = (Var*)malloc(sizeof(Var) * func->args.count);

    Value va_args = create_value(ARRAY_VA_TYPE);
    if (HAS_VARIADIC(func)) {
        va_args.as_ptr = alloc_arr(VARIADIC_TYPE);
    }
    
    for (size_t i = 0; i < args.count; ++i) {
        Arg arg = args.items[i];
        
        Value value = execute(context, arg.node);
        if (has_errors(context)) {
            free(func_args);
            return;
        }

        if (i >= func->args.count - HAS_VARIADIC(func)) {
            Array *va_args_arr = va_args.as_ptr;
            if (value.type->tag == TYPE_ARRAY) {
                Array *arr = value.as_ptr;
                if (arr->el_type->tag == TYPE_VARIADIC) {
                    da_append_many(&va_args_arr->els, &arr->els);
                    continue;
                }
            }
            
            da_append(&va_args_arr->els, value);
            continue;
        }
        
        String_Builder *name_sb = func->args.items[i].name;
        if (arg.has_name) {
            name_sb = sb_alloc();
            sv_to_sb(&arg.name, name_sb);
        }

        String_View sv = {0};
        sv_from_sb(&sv, name_sb);
        
        if (!check_arg_name(func, &sv)) {
            append_error(context, ERROR_UNEXPECTED_NAMED_ARG);
            free(func_args);
            return;
        }
        
        func_args[i].name = name_sb;
        func_args[i].val = value;
        func_args[i].constant = false;
    }

    if (va_args.as_ptr != NULL) {
        size_t last = func->args.count - 1;
        func_args[last].name = func->args.items[last].name;
        func_args[last].val = va_args;
        func_args[last].constant = false;
    }
    
    *unwrapped = func_args;
}

static size_t recursion_depth = 0;
static const size_t max_recursion_depth = 1024;

Value exec_func(Context *context, Func *func, Args args) {
    Value value = create_value(VOID_TYPE);
    
    if (recursion_depth >= max_recursion_depth) {
        append_error(context, ERROR_RECURSION_LIMIT_EXCEEDED);
        return value;
    }
    
    check_args(context, func, args);
    if (has_errors(context)) {
        return value;
    }

    Var *unwrapped = NULL;
    
    unwrap_args(context, func, args, &unwrapped);
    if (has_errors(context)) {
        return value;
    }
    
    HashMap *local_vars = hm_copy(context->global->scope.vars);
    HashMap *local_funcs = hm_copy(context->scope.funcs);
            
    for (size_t i = 0; i < func->args.count; ++i) {
        assert(hm_nput(local_vars, unwrapped[i].name->items, unwrapped[i].name->count, &unwrapped[i]) == 0);
    }

    Context local_context = {
        .global = context->global,
        .scope = (Scope){
            .vars = local_vars,
            .funcs = local_funcs,
        },
        .errors = context->errors,
    };

    ++recursion_depth;
    
    switch (func->kind) {
        case FUNC_BUILT_IN: {
            FuncBuiltIn *built_in = (void*)func;
            value = built_in->func(context, &local_context);
        } break;

        case FUNC_CUSTOM: {
            FuncCustom *custom = (void*)func;
            
            value = execute(&local_context, custom->body);
            hm_free(local_vars);
            hm_free(local_funcs);
        } break;
    }

    --recursion_depth;
    
    free(unwrapped);
    return value;
}

Value println_func(Context *context, Context *fn_context) {
    Value val = print_func(context, fn_context);
    if (has_errors(fn_context)) {
        return val;
    }

    printf("\n");
    return val;
}

Value print_func(Context *context, Context *fn_context) {
    Value ret = create_value(VOID_TYPE);
    
    Value formated = format_func(context, fn_context);
    if (has_errors(context)) {
        return ret;
    }

    sb_pprint((String_Builder*)formated.as_ptr);
    da_pfree((String_Builder*)formated.as_ptr);
    return ret;
}

Value format_func(Context *context, Context *fn_context) {
    Value ret = create_value(STR_TYPE);
    
    Var *fmt;
    if (!resolve_name_cstr(fn_context, "msg", &fmt)) {
        append_error(context, ERROR_NOT_DEFINED);
        return ret;
    }
    
    Var *va_args;
    if (!resolve_name_cstr(fn_context, "args", &va_args)) {
        append_error(context, ERROR_NOT_DEFINED);
        return ret;
    }

    String_View fmt_sv = {0};
    sv_from_sb(&fmt_sv, fmt->val.as_ptr);

    Array *va_args_arr = (void*)va_args->val.as_ptr;
    
    ret.as_ptr = sb_alloc();
    format_str(ret.as_ptr, context, fmt_sv, va_args_arr);
    return ret;
}

Value read_func(Context *context, Context *fn_context) {
    Value ret = create_value(CHAR_TYPE);
    scanf("%c", &ret.as_int);
    return ret;
}

Value readln_func(Context *context, Context *fn_context) {
    Value ret = create_value(STR_TYPE);
    
    ret.as_ptr = sb_alloc();
    
    if (!sb_getline(ret.as_ptr, stdin)) {
        append_error(context, ERROR_CLOSED_STDIN);
    }
    
    return ret;
}

typedef String_Builder (*trim_fn)(String_Builder *sb);
Value trim(Context *context, Context *fn_context, trim_fn trim_fn) {
    Value ret = create_value(STR_TYPE);
    
    Var *str;
    if (!resolve_name_cstr(fn_context, "string", &str)) {
        append_error(context, ERROR_NOT_DEFINED);
        return ret;
    }

    String_Builder trimmed = trim_fn(str->val.as_ptr);
    String_Builder *sb = sb_alloc();
    sb->items = trimmed.items;
    sb->count = trimmed.count;
    sb->capacity = trimmed.capacity;
    
    ret.as_ptr = sb;
    return ret;
}

Value trim_func(Context *context, Context *fn_context) {
    return trim(context, fn_context, sb_trim);
}

Value trim_left_func(Context *context, Context *fn_context) {
    return trim(context, fn_context, sb_trim_left);
}

Value trim_right_func(Context *context, Context *fn_context) {
    return trim(context, fn_context, sb_trim_right);
}