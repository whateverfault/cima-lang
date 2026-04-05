#include <stdio.h>
#include <math.h>

#include "funcs.h"
#include "nothing/nothing.h" 
#include "other/built_in.h"

#define HAS_VARIADIC(func) ((func)->args.items[(func)->args.count - 1].type->kind == TYPE_VARIADIC)

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

bool check_arg_name(Func *func, String_View name) {
    for (size_t i = 0; i < func->args.count; ++i) {
        if (sv_cmp_sb(name, func->args.items[i].name)) {
            return true;
        }
    }

    return false;
}

void unwrap_args(Context *context, Func *func, Args args, Var **unwrapped, Variadic *variadic) {
    size_t va_args_count = args.count - func->args.count + HAS_VARIADIC(func);
    size_t va_args_start = HAS_VARIADIC(func)? args.count - va_args_count : SIZE_MAX;
    
    Value *va_args = NULL;
    if (va_args_count > 0) {
        va_args = (Value*)malloc(sizeof(Value) * va_args_count);   
    }
    
    size_t args_count = func->args.count - HAS_VARIADIC(func);
    Var *func_args = (Var*)malloc(sizeof(Var) * args_count);
    
    for (size_t i = 0; i < args.count; ++i) {
        Arg arg = args.items[i];
        
        Value value = execute(context, arg.node);
        if (has_errors(context)) {
            free(func_args);
            free(va_args);
            return;
        }

        if (i >= va_args_start && va_args != NULL) {
            va_args[i - va_args_start] = value;
            continue;
        }
        
        func_args[i].name = arg.has_name? sv_to_sb(arg.name) : func->args.items[i].name;
        func_args[i].val = value;

        if (!check_arg_name(func, sb_to_sv(func_args[i].name))) {
            append_error(context, ERROR_UNEXPECTED_NAMED_ARG);
            free(func_args);
            free(va_args);
            return;
        }
    }

    *unwrapped = func_args;
    variadic->args = va_args;
    variadic->count = va_args_count;
}

static size_t recursion_depth = 0;
static const size_t max_recursion_depth = 1024;

Value exec_func(Context *context, Func *func, Args args) {
    Value value = alloc_value(VOID_TYPE);
    
    if (recursion_depth >= max_recursion_depth) {
        append_error(context, ERROR_RECURSION_LIMIT_EXCEEDED);
        return value;
    }
    
    check_args(context, func, args);
    if (has_errors(context)) {
        return value;
    }

    Var *unwrapped = NULL;
    
    Variadic *va_args = (Variadic*)malloc(sizeof(Variadic));
    *va_args = (Variadic){0};
    unwrap_args(context, func, args, &unwrapped, va_args);
    if (has_errors(context)) {
        return value;
    }

    Var *va_args_var = NULL;
    
    HashMap *local_vars = hm_alloc();
            
    for (size_t i = 0; i < func->args.count - HAS_VARIADIC(func); ++i) {
        assert(hm_nput(local_vars, unwrapped[i].name.items, unwrapped[i].name.count, &unwrapped[i]) == 0);
    }
    
    if (HAS_VARIADIC(func) && va_args->count > 0) {
        va_args_var = (Var*)malloc(sizeof(Var));
        *va_args_var = (Var){
            .name = (String_Builder){
                .items = "...",
                .count = 3,
            },
            .val = (Value){
                .type = VARIADIC_TYPE,
                .as_ptr = va_args,
            },
            .constant = true,
        };
        
        assert(hm_nput(local_vars, va_args_var->name.items, va_args_var->name.count, va_args_var) == 0);
    }

    Context local_context = {
        .scope = (Scope){
            .vars = local_vars,
            .funcs = context->scope.funcs,
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
        } break;
    }

    --recursion_depth;
    
    free(unwrapped);
    free(va_args->args);
    free(va_args);
    free(va_args_var);
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
    Value ret = alloc_value(VOID_TYPE);
    
    Value formated = format_func(context, fn_context);
    if (has_errors(context)) {
        return ret;
    }

    sb_print(formated.as_str);
    free(formated.as_str.items);
    return ret;
}


// TODO: Implement function for reading user input
Value read_func(Context *context, Context *fn_context) {
    assert(0 && "NOT IMPLEMENTED");
}

Value format_func(Context *context, Context *fn_context) {
    Value ret = alloc_value(STR_TYPE);
    
    Var *fmt;
    if (!resolve_name_cstr(fn_context, "msg", &fmt)) {
        append_error(context, ERROR_NOT_DEFINED);
        return ret;
    }
    
    Variadic *va_args = get_va_args(fn_context);

    String_View fmt_sv = sb_to_sv(fmt->val.as_str);
    ret.as_str = (String_Builder){0};
    format_str(&ret.as_str, context, fmt_sv, va_args);
    return ret;
}

Variadic *get_va_args(Context *context) {
    Var *va_args_var = NULL;
    if (!resolve_name_cstr(context, "...", &va_args_var)) {
        return NULL;
    }
    
    return (void*)va_args_var->val.as_ptr;
}