#include <string.h>
#include <assert.h>

#include "parser/parser.h"

#include "executor/types/type.h"

#define NOTHING_IMPLEMENTATION
#include "nothing/nothing.h"

#include "executor/funcs/funcs.h"
#include "executor/error.h"
#include "executor.h"

#include "other/built_in.h"

#define GET_REF_VALUE(ref) *(Value*)(ref).as_ptr

void global_ctx_init(Context *ctx) {
    ctx->global = ctx;
    ctx->scope.symbols = hm_alloc();
    ctx->errors = calloc(1, sizeof(Errors));
    
    ctx->type_cache = (TypeCache){
        .array_cache = hm_alloc(),
        .func_cache = hm_alloc(),
        .type_cache = hm_alloc(),
        .ref_cache = hm_alloc(),
    };
    
    for (size_t i = 0; i < builtin_funcs_count; ++i) {
        builtin_funcs[i].type = alloc_func_type(ctx, (void*)&builtin_funcs[i]);
        hm_put_sb(ctx->scope.symbols, builtin_funcs[i].name, &builtin_funcs[i]);
    }

    for (size_t i = 0; i < builtin_types_count; ++i) {
        hm_put_sb(ctx->scope.symbols, builtin_types[i]->name, builtin_types[i]);
    }
}

void type_cache_free(TypeCache type_cache) {
    hm_free(type_cache.array_cache);
    hm_free(type_cache.func_cache);
    hm_free(type_cache.type_cache);
    hm_free(type_cache.ref_cache);
}

void ctx_free(Context *ctx) {
    hm_free(ctx->scope.symbols);
    type_cache_free(ctx->type_cache);
    da_pfree(ctx->errors);
}

void pattern_free(Pattern pattern) {
    if (pattern.name != NULL) {
        free(pattern.name->items);
        free(pattern.name);
    }
}

void patterns_free(Patterns patterns) {
    for (size_t i = 0; i < patterns.count; ++i) {
        pattern_free(patterns.items[i]);
    }

    da_free(patterns);
}

void func_free(Func *func) {
    if (func == NULL) {
        return;
    }
    
    if (func->name != NULL) {
        free(func->name->items);
        free(func->name);
    }

    patterns_free(func->args);
}

void funcs_free(Funcs funcs) {
    for (size_t i = 0; i < funcs.count; ++i) {
        func_free(funcs.items[i]);
    }

    free(funcs.items);
}

void member_free(Member *member) {
    if (member->name != NULL) {
        free(member->name->items);
        free(member->name);

        if (member->kind == MEMBER_METHOD) {
            func_free(member->method.func);
        }
    }
}

void members_free(Members fields) {
    for (size_t i = 0; i < fields.count; ++i) {
        member_free(fields.items[i]);
    }

    da_free(fields);
}

void var_free(Var *var) {
    if (var == NULL) {
        return;
    }
    
    if (var->name != NULL) {
        free(var->name->items);
        free(var->name);
    }
}

void type_free(Type *type) {
    if (type == NULL) {
        return;
    }
    
    if (type->name != NULL) {
        free(type->name->items);
        free(type->name);
    }

    type_free(type->el_type);
    members_free(type->members);
}

void symbol_free(Symbol *symb) {
    switch (symb->symb_kind) {
        case SYMB_TYPE: {
            type_free((void*)symb);
        } break;

        case SYMB_VAR: {
            var_free((void*)symb);
        } break;
        
        case SYMB_FUNC: {
            func_free((void*)symb);
        } break;
    }
}

bool has_errors(Context *ctx) {
    return ctx->errors->count > 0;
}

void append_error(Context *ctx, RuntimeError err) {
    if (err == ERROR_NONE) {
        return;
    }
    
    da_append(ctx->errors, err);
}

RuntimeError get_signal_error(Signal sig) {
    switch (sig) {
        case SIGNAL_CONTINUE: return ERROR_CONTINUE_OUTSIDE_LOOP;
        case SIGNAL_BREAK: return ERROR_BREAK_OUTSIDE_LOOP;
        default: return ERROR_NONE;
    }
}

RuntimeError get_signal_error_unexpected(Signal sig) {
    switch (sig) {
        case SIGNAL_CONTINUE: return ERROR_UNEXPECTED_CONTINUE;
        case SIGNAL_BREAK: return ERROR_UNEXPECTED_BREAK;
        case SIGNAL_RETURN: return ERROR_UNEXPECTED_RETURN;
        default: return ERROR_NONE;
    }
}

Value alloc_func(Context *ctx, Func *func) {
    Type *func_type = alloc_func_type(ctx, func);

    Value val = {
        .type = func_type,
        .as_ptr = func,
    };
    
    return val;
}

Var *alloc_var(String_Builder *name_sb, Value *val, bool constant) {
    Var *var = (Var*)calloc(1, sizeof(Var));
    assert(var != NULL && "Memory allocation failed");
    
    var->symb_kind = SYMB_VAR;
    var->name = name_sb;
    var->val = val;
    var->constant = constant;
    var->is_static = false;
    
    return var;
}

FuncCustom *alloc_custom_func(Context *ctx, String_Builder *name, Patterns args, AST_Node *body, Type *ret_type, bool is_static) {
    FuncCustom *func = (FuncCustom*)calloc(1, sizeof(FuncCustom));
    assert(func != NULL && "Memory allocation failed");
    
    func->symb_kind = SYMB_FUNC;
    func->kind = FUNC_CUSTOM;
    func->ret_type = ret_type;
    func->name = name;
    func->body = body;
    func->args = args;
    func->constant = true;
    func->is_static = is_static;
    func->type = alloc_func_type(ctx, (void*)func);

    return func;
}

Value *alloc_value(Value value) {
    Value *val = calloc(1, sizeof(Value));
    assert(val != NULL && "Failed to allocate value");

    *val = value;
    return val;
}

Value create_value(Type *type) {
    return (Value){
        .as_struct = (Struct){0},
        .type = type,
    };
}

EvalResult create_result(Type *type) {
    return (EvalResult){
        .sig = SIGNAL_NONE,
        .val = create_value(type),
    };
}

bool get_symbol(Context *ctx, String_View name_sv, Symbol **symb) {
    assert(symb != NULL);
    
    *symb = hm_nget(ctx->scope.symbols, name_sv.items, name_sv.count);
    return *symb != NULL;
}

bool resolve_symb(Context *ctx, String_View type_sv, Symbol **symb) {
    return symb != NULL && get_symbol(ctx, type_sv, symb) && *symb != NULL;
}

bool resolve_name(Context *ctx, AST_Node *name, Var **var) {
    AST_NodeName *name_node = (AST_NodeName*)name;
    return get_symbol(ctx, name_node->name, (void*)var) && var != NULL;
}

bool resolve_name_sv(Context *ctx, String_View name_sv, Var **var) {
    return get_symbol(ctx, name_sv, (void*)var) && var != NULL;
}

bool resolve_name_cstr(Context *ctx, char *name, Var **var) {
    String_View sv = {
        .items = name,
        .count = strlen(name),
    };
    return resolve_symb(ctx, sv, (void*)var);
}

bool resolve_name_node(Context *ctx, AST_Node *node, Var **var) {
    AST_NodeName *name_node = (void*)node;
    return resolve_symb(ctx, name_node->name, (void*)var);
}

bool resolve_func(Context *ctx, String_View name, Func **func) {
    return resolve_symb(ctx, name, (void*)func);
}

bool is_node_value(AST_Node *node) {
    return node->kind == AST_LIT || node->kind == AST_NAME || node->kind == AST_CALL || node->kind == AST_ARR;
}

bool is_assignable(AST_Node *node) {
    switch (node->kind) {
        case AST_INDEX:
        case AST_NAME:
        case AST_MEMBER_ACCESS: return true;

        default: return false;
    }
}

bool is_assignment(AST_Node *node) {
    switch (node->kind) {
        case AST_BINOP: {
            AST_NodeBinOp *op = (void*)node;
            switch (op->op) {
                case BINOP_ASSIGN: return true;
                
                default: return false;
            }
        }

        case AST_UNOP: {
            AST_NodeUnOp *op = (void*)node;
            switch (op->op) {
                case UNOP_DECREMENT:
                case UNOP_INCREMENT: return true;
                
                default: return false;
            }
        }

        default: return false;
    }
}

Type *resolve_type(Context *ctx, AST_Type *ast_type) {
    if (ast_type == NULL || !ast_type->provided_name) {
        if (ast_type != NULL && ast_type->is_array) {
            return alloc_array_type(ctx, resolve_type(ctx, ast_type->el_type));
        }
        
        return ANY_TYPE;
    }
    
    Type *type = NULL;
    if (!resolve_symb(ctx, ast_type->name, (void*)&type) || type->symb_kind != SYMB_TYPE) {
        append_error(ctx, ERROR_UNKNOWN_TYPE);
        return type;
    }

    if (ast_type->el_type != NULL) {
        type->el_type = resolve_type(ctx, ast_type->el_type);
    }
    
    return type;
}

void resolve_pattern(Context *ctx, AST_Pattern ast_pattern, Pattern *pattern) {
    String_Builder *name_sb = sb_alloc();
    sv_to_sb(&ast_pattern.name, name_sb);

    Type *type = resolve_type(ctx, ast_pattern.type);
    if (has_errors(ctx)) {
        return;
    }
    
    *pattern = (Pattern){
        .name = name_sb,
        .type = type,
        .initializer = ast_pattern.initializer,
        .constant = ast_pattern.is_const,
    };
}

void resolve_patterns(Context *ctx, AST_Patterns ast_patterns, Patterns *patterns) {
    for (size_t i = 0; i < ast_patterns.count; ++i) {
        Pattern pattern = {0};
        resolve_pattern(ctx, ast_patterns.items[i], &pattern);
        if (has_errors(ctx)) {
            return;
        }

        da_append(patterns, pattern);
    }
}

void resolve_field(Context *ctx, AST_Pattern ast_field, size_t offset, Member *member) {
    String_Builder *name_sb = sb_alloc();
    sv_to_sb(&ast_field.name, name_sb);

    Type *type = resolve_type(ctx, ast_field.type);
    if (has_errors(ctx)) {
        return;
    }
    
    *member = (Member){
        .name = name_sb,
        .type = type,
        .kind = MEMBER_FIELD,
        .is_const = ast_field.is_const,
        .is_static = ast_field.is_static,
    };

    if (member->is_static) {
        EvalResult result = execute(ctx, ast_field.initializer);
        append_error(ctx, get_signal_error_unexpected(result.sig));
        if (has_errors(ctx)) {
            return;
        }
        
        member->field.static_initializer = alloc_value(result.val);
    }
    else {
        member->field.initializer = ast_field.initializer;
    }
}

Member *alloc_member() {
    Member *member = calloc(1, sizeof(Member));
    assert(member != NULL && "Failed to allocate member.");

    return member;
}

void resolve_fields(Context *ctx, AST_Patterns ast_patterns, Members *fields) {
    for (size_t i = 0; i < ast_patterns.count; ++i) {
        Member *member = alloc_member();
        resolve_field(ctx, ast_patterns.items[i], sizeof(Value) * i, member);
        if (has_errors(ctx)) {
            return;
        }
        
        da_append(fields, member);
    }
}

void resolve_methods(Context *ctx, AST_Nodes ast_methods, Members *members) {
    for (size_t i = 0; i < ast_methods.count; ++i) {
        AST_NodeFuncDecl *func_node = (void*)ast_methods.items[i];

        Patterns args = {0};
        resolve_patterns(ctx, func_node->args, &args);
        if (has_errors(ctx)) {
            return;
        }

        Type *ret_type = NULL;
        if (func_node->ret_type != NULL) {
            ret_type = resolve_type(ctx, func_node->ret_type);
        }
        else {
            ret_type = VOID_TYPE;
        }
        
        String_Builder *name_sb = sb_alloc();
        sv_to_sb(&func_node->name, name_sb);
        
        FuncCustom *func = alloc_custom_func(ctx, name_sb, args, func_node->body, ret_type, func_node->is_static);
        Type *func_type = alloc_func_type(ctx, (void*)func);
        
        Member *method = alloc_member();
        *method = (Member){
            .name = func->name,
            .type = func_type,
            .is_static = func_node->is_static,
            .is_const = true,
            .kind = MEMBER_METHOD,
            .method = {
                .func = (Func*)func,
            },
        };
        
        da_append(members, method);
    }
}

// TODO: Implement function overloading

void register_func(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_FUNC_DECL);

    AST_NodeFuncDecl *func_node = (AST_NodeFuncDecl*)node;
    
    Func *check = NULL;
    if (resolve_func(ctx, func_node->name, &check)
    &&  check != NULL && check->constant) {
        append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }

    Patterns args = {0};
    resolve_patterns(ctx, func_node->args, &args);
    if (has_errors(ctx)) {
        return;
    }

    Type *ret_type = NULL;
    if (func_node->ret_type != NULL) {
        ret_type = resolve_type(ctx, func_node->ret_type);
    }
    else {
        ret_type = VOID_TYPE;
    }
    
    if (has_errors(ctx)) {
        return;
    }

    String_Builder *name_sb = NULL;
    if (check == NULL) {
        name_sb = sb_alloc();
        sv_to_sb(&func_node->name, name_sb);
    }
    else {
        name_sb = check->name;
        patterns_free(check->args);
    }
    
    FuncCustom *func = alloc_custom_func(ctx, name_sb, args, func_node->body, ret_type, func_node->is_static);
    assert(hm_nput(ctx->scope.symbols, func_node->name.items, func_node->name.count, func) == 0 && "Failed to register function.");
}

void register_struct(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_STRUCT_DECL);

    AST_NodeStructDecl *struct_node = (AST_NodeStructDecl*)node;
    
    Type *check = NULL;
    if (resolve_symb(ctx, struct_node->name, (void*)&check) && check != NULL) {
        if (check->kind != TYPE_STRUCT) {
            append_error(ctx, ERROR_ILLEGAL_TYPE_REDEFINITION);
            return;
        }
        
        if (check->constant) {
            append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
            return;
        }
    }
    
    String_Builder *name_sb = NULL;
    if (check == NULL) {
        name_sb = sb_alloc();
        sv_to_sb(&struct_node->name, name_sb);
    }
    else {
        name_sb = check->name;
        members_free(check->members);
    }

    Members members = {0};
    resolve_fields(ctx, struct_node->fields, &members);
    if (has_errors(ctx)) {
        return;
    }
    
    Type *type = alloc_struct_type(name_sb, members);
    assert(hm_put_sb(ctx->scope.symbols, type->name, type) == 0 && "Failed to register type.");

    resolve_methods(ctx, struct_node->methods, &type->members);
}

void register_var(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_LET);

    Var *check = NULL;
    if (resolve_name(ctx, node, &check)
    &&  check != NULL && check->constant) {
        append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    AST_NodeLetStmt *let_node = (void*)node;

    Type *provided_type = NULL;
    if (let_node->type != NULL) {
        provided_type = resolve_type(ctx, let_node->type);
    }
    else {
        provided_type = ANY_TYPE;
    }
    if (has_errors(ctx)) {
        return;
    }
    
    EvalResult result = create_result(provided_type);
    if (let_node->initializer != NULL) {
        result = execute_expr(ctx, let_node->initializer);
        append_error(ctx, get_signal_error(result.sig));
        if (has_errors(ctx)) {
            return;
        }

        // TODO: Handle 'any' type properly
        result.val = cast_value(ctx, result.val, provided_type);
        if (has_errors(ctx)) {
            return;
        }
    }
    else {
        alloc_type_value(&result.val, provided_type);
    }

    Var *var = NULL;
    String_Builder *sb = sb_alloc();
    sv_to_sb(&let_node->name, sb);
    
    if (!resolve_name_sv(ctx, let_node->name, &var)) {
        var = alloc_var(sb, alloc_value(copy_value(&result.val)), let_node->constant);
        assert(hm_nput(ctx->scope.symbols, let_node->name.items, let_node->name.count, var) == 0 && "Failed to register function.");
        return;
    }

    var->val = alloc_value(copy_value(&result.val));
    var->constant = let_node->constant;
}

EvalResult execute_loop_stmt(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_FOR);

    AST_NodeForStmt *for_node = (void*)node;

    EvalResult result = create_result(VOID_TYPE);

    if (for_node->initializer != NULL) {
        result = execute(ctx, for_node->initializer);
        append_error(ctx, get_signal_error(result.sig));
        if (has_errors(ctx)) {
            return result;
        }
    }

    while (true) {
        EvalResult condition = create_result(BOOL_TYPE);
        condition.val.as_int = true;
        
        if (for_node->condition != NULL) {
            condition = execute_expr(ctx, for_node->condition);
            append_error(ctx, get_signal_error(condition.sig));
            if (has_errors(ctx)) {
                return result;
            }
        }

        bool condition_value = to_bool(ctx, condition.val);
        if (has_errors(ctx)) {
            return result;
        }
        
        if (!condition_value) {
            break;
        }

        result = execute(ctx, for_node->body);
        if (has_errors(ctx)) {
            return result;
        }
        
        if (result.sig == SIGNAL_CONTINUE) {
            if (for_node->next != NULL) {
                EvalResult next_result = execute_expr(ctx, for_node->next);
                append_error(ctx, get_signal_error(next_result.sig));
                if (has_errors(ctx)) {
                    return result;
                }
            }
            
            result.sig = SIGNAL_NONE;
            continue;
        }
        
        if (result.sig == SIGNAL_BREAK) {
            result.sig = SIGNAL_NONE;
            return result;
        }

        if (result.sig == SIGNAL_RETURN) {
            return result;
        }

        if (for_node->next != NULL) {
            EvalResult next_result = execute_expr(ctx, for_node->next);
            append_error(ctx, get_signal_error(next_result.sig));
            if (has_errors(ctx)) {
                return result;
            }
        }
    }

    return result;
}

void unwrap_index_expr(Context *ctx, AST_Node *node, Value *arr, Value* index) {
    assert(node->kind == AST_INDEX);

    AST_NodeIndex *index_node = (void*)node;

    EvalResult arr_res = execute_expr(ctx, index_node->node);
    append_error(ctx, get_signal_error(arr_res.sig));
    if (has_errors(ctx)) {
        return;
    }

    if (arr_res.val.type != STR_TYPE && arr_res.val.type->kind != TYPE_ARRAY) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        return;
    }
    
    EvalResult index_res = execute_expr(ctx, index_node->index);
    append_error(ctx, get_signal_error(index_res.sig));
    if (has_errors(ctx)) {
        return;
    }

    if (index_res.val.type != INT_TYPE) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        return;
    }

    *arr = arr_res.val;
    *index = index_res.val;
}

void assign_var(Context *ctx, AST_NodeName *name_node, Value val) {
    String_View name_sv = name_node->name;

    Var *var = NULL;
    if (resolve_name_sv(ctx, name_sv, &var)) {
        if (var->constant) {
            append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
            return;
        }
    }
    else {
        append_error(ctx, ERROR_NOT_DEFINED);
        return;
    }

    if (!compatible_types(var->val->type, val.type)) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        return;
    }
    
    String_Builder *sb = sb_alloc();
    sv_to_sb(&name_sv, sb);
    
    var->val = alloc_value(copy_value(&val));
}

void assign_arr_el(Context *ctx, AST_NodeIndex *index_node, Value val) {
    Value arr_val;
    Value index;
    unwrap_index_expr(ctx, (void*)index_node, &arr_val, &index);

    if (arr_val.type->constant) {
        append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    switch (arr_val.type->kind) {
        case TYPE_PRIMITIVE: {
            if (arr_val.type == STR_TYPE) {
                if (val.type != CHAR_TYPE) {
                    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                    return;
                }
            
                String_Builder *str = arr_val.as_ptr;
                str->items[index.as_int] = val.as_int;
                return;
            }

            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        }
        
        case TYPE_ARRAY: {
            Array *arr = arr_val.as_ptr;

            if (!compatible_types(arr->el_type, val.type)) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return;
            }
            
            arr->items[index.as_int] = val;
        } break;
            
        default: append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    }
}

void assign_member(Context *ctx, AST_NodeMemberAccess *member_access_node, Value val) {
    Value base;
    Member *member = get_member_from_node(ctx, (void*)member_access_node, &base);
    if (has_errors(ctx)){
        return;
    }

    if (member->is_const) {
        append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    if (member->kind != MEMBER_FIELD) {
        append_error(ctx, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }

    if (!compatible_types(member->type, val.type)) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        return;
    }
    
    assign_field(ctx, base.as_struct, member, val);
}

void assign(Context *ctx, AST_Node *node, Value val) {
    switch (node->kind) {
        case AST_NAME: {
            assign_var(ctx, (void*)node, val);
        } break;

        case AST_INDEX: {
            assign_arr_el(ctx, (void*)node, val);
        } break;

        case AST_MEMBER_ACCESS: {
            assign_member(ctx, (void*)node, val);
        } break;
            
        default: {
            append_error(ctx, ERROR_CANNOT_ASSIGN_TO_CONST);
        }
    }
}

Value execute_binary_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_BINOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeBinOp *binop = (void*)node;

    EvalResult lhs = create_result(VOID_TYPE);
    if (!is_assignment(node)) {
        lhs = execute_expr(ctx, binop->lhs);
        if (lhs.sig != SIGNAL_NONE) {
            append_error(ctx, get_signal_error_unexpected(lhs.sig));
        }
        
        if (has_errors(ctx)) {
            return val;
        }
    }

    if (binop->op == BINOP_LOGIC_AND) {
        bool lhs_bool = to_bool(ctx, lhs.val);
        if (has_errors(ctx)) {
            return val;
        }

        if (!lhs_bool) {
            val.type = BOOL_TYPE;
            val.as_int = false;
            return val;
        }
    }
    
    EvalResult rhs = execute_expr(ctx, binop->rhs);
    if (rhs.sig != SIGNAL_NONE) {
        append_error(ctx, get_signal_error_unexpected(rhs.sig));
    }
    
    if (has_errors(ctx)) {
        return val;
    }
    
    switch (binop->op) {
        case BINOP_PLUS: {
            val = binary_plus(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_MINUS: {
            val = binary_minus(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_MUL: {
            val = binary_mul(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_DIV: {
            val = binary_div(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_MOD: {
            val = binary_mod(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_POW: {
            val = binary_pow(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_LOGIC_AND: {
            val = binary_logic_and(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_BIT_AND: {
            val = binary_bitwise_and(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_LOGIC_OR: {
            val = binary_logic_or(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_BIT_OR: {
            val = binary_bitwise_or(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_GT: {
            val = binary_gt(ctx, lhs.val, rhs.val);
        } break;
            
        case BINOP_LT: {
            val = binary_lt(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_GTEQ: {
            val = binary_gteq(ctx, lhs.val, rhs.val);
        } break;
            
        case BINOP_LTEQ: {
            val = binary_lteq(ctx, lhs.val, rhs.val);
        } break;
            
        case BINOP_EQ: {
            val = binary_eq(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_NEQ: {
            val = binary_neq(ctx, lhs.val, rhs.val);
        } break;

        case BINOP_ASSIGN: {
            if (!is_assignable(binop->lhs)) {
                append_error(ctx, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            assign(ctx, (void*)binop->lhs, rhs.val);
        } break;
            
        default: break;
    }
    
    return val;
}

Value get_member_ref_by_node(Context *ctx, AST_Node *expr) {
    assert(expr->kind == AST_MEMBER_ACCESS);

    Value base = create_value(VOID_TYPE);
    Member *member = get_member_from_node(ctx, expr, &base);
    if (has_errors(ctx)) {
        return base;
    }
    
    if (base.type->kind == TYPE_TYPE) {
        if (!member->is_static) {
            append_error(ctx, ERROR_UNKNOWN_FIELD);
            return base;
        }
        
        base = get_static_member_ref(ctx, member);
    }
    else {
        base = get_member_ref(ctx, base.as_struct, member);
    }

    return base;
}

Value get_reference(Context *ctx, AST_Node *expr) {
    Value val = create_value(VOID_TYPE);
    
    switch (expr->kind) {
       case AST_NAME: {
           AST_NodeName *name_node = (void*)expr;
           
           Symbol *symb = NULL;
           if (!resolve_symb(ctx, name_node->name, &symb)) {
               append_error(ctx, ERROR_CANNOT_TAKE_REF_TO_CONST);
               return val;
           }

           switch (symb->symb_kind) {
               case SYMB_VAR: {
                   Var *var = (void*)symb;

                   val.type = alloc_ref_type(ctx, var->val->type);
                   val.as_ptr = var->val;
               } break;

               default: {
                   append_error(ctx, ERROR_CANNOT_TAKE_REF_TO_CONST);
               } break;
           }
       } break;

       case AST_MEMBER_ACCESS: {
           val = get_member_ref_by_node(ctx, expr);
       } break;

        default: {
            append_error(ctx, ERROR_CANNOT_TAKE_REF_TO_CONST);
        } break;
    }

    return val;
}

Value execute_unary_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_UNOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeUnOp *unop = (void*)node;

    if (unop->op == UNOP_REF) {
        val = get_reference(ctx, unop->expr);
        return val;
    }
    
    EvalResult result = execute_expr(ctx, unop->expr);
    if (result.sig != SIGNAL_NONE) {
        append_error(ctx, get_signal_error_unexpected(result.sig));
    }

    if (result.val.type->kind == TYPE_REF) {
        result.val = GET_REF_VALUE(result.val);
    }
    
    if (has_errors(ctx)) {
        return val;
    }
    
    switch (unop->op) {
        case UNOP_INCREMENT: {
            if (!is_assignable(unop->expr)) {
                append_error(ctx, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            val = unary_increment(ctx, result.val);
            
            assign(ctx, (void*)unop->expr, val);
            return val;
        }
        case UNOP_DECREMENT: {
            if (!is_assignable(unop->expr)) {
                append_error(ctx, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            val = unary_decrement(ctx, result.val);
            
            assign(ctx, (void*)unop->expr, val);
            return val;
        }
        case UNOP_NOT: {
            val = unary_not(ctx, result.val);
            return val;
        }
        case UNOP_PLUS: {
            val = unary_plus(ctx, result.val);
            return val;
        }
        case UNOP_MINUS: {
            val = unary_minus(ctx, result.val);
            return val;
        }
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

Value execute_call_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_CALL);

    Value value = create_value(VOID_TYPE);
    
    AST_NodeCall *call = (AST_NodeCall*)node;

    EvalResult func = execute(ctx, call->to_call);
    if (has_errors(ctx)) {
        return value;
    }

    if (func.val.type->kind != TYPE_FUNC) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    }
    
    return exec_func(ctx, func.val.as_ptr, call->args);
}

// TODO: Force types to implement index operation (Implement traits and operator overloading)

Value execute_index_expr(Context *ctx, AST_Node *node) {
    Value value = create_value(VOID_TYPE);
    
    Value arr;
    Value index;
    unwrap_index_expr(ctx, node, &arr, &index);
    if (has_errors(ctx)) {
        return value;
    }
    
    switch (arr.type->kind) {
        case TYPE_PRIMITIVE: {
            if (arr.type == STR_TYPE) {
                String_Builder *str = arr.as_ptr;
                value = create_value(CHAR_TYPE);

                if (index.as_int < 0 || index.as_int >= str->count) {
                    append_error(ctx, ERROR_INDEX_OUT_OF_BOUNDS);
                    return value;
                }
            
                value.as_int = str->items[index.as_int];
                return value;
            }
            
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        }
        
        case TYPE_ARRAY: {
            Array *array = arr.as_ptr;
            value = create_value(array->el_type);

            if (index.as_int < 0 || index.as_int >= array->count) {
                append_error(ctx, ERROR_INDEX_OUT_OF_BOUNDS);
                return value;
            }
            
            value = array->items[index.as_int];
            return value;
        }

        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return value;
        }
    }
}

Value execute_arr_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_ARR);
    
    AST_NodeArray *arr_node = (void*)node;
    
    // TODO: Type inference
    Value val = create_value(ARRAY_ANY_TYPE);
    Array *arr = alloc_array_value(ANY_TYPE);
    da_reserve(arr, arr_node->nodes.count);

    for (size_t i = 0; i < arr_node->nodes.count; ++i) {
        EvalResult el = execute_expr(ctx, arr_node->nodes.items[i]);
        append_error(ctx, get_signal_error(el.sig));
        if (has_errors(ctx)) {
            da_pfree(arr);
            free(arr);
            return val;
        }
        
        da_append(arr, el.val);
    }

    val.as_ptr = arr;
    return val;
}

EvalResult execute_block_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_BLOCK);
    AST_NodeBlock *block = (void*)node;

    Context local_context = {
        .global = ctx->global,
        .scope = (Scope){
            .symbols = hm_copy(ctx->scope.symbols),
        },
        .errors = ctx->errors,
        .type_cache = ctx->type_cache,
    };
    
    EvalResult result = execute_nodes(&local_context, &block->nodes);
    if (result.sig != SIGNAL_NONE) {
        hm_free(local_context.scope.symbols);
        return result;
    }
    
    if (has_errors(ctx)) {
        return result;
    }
    
    if (block->ret_expr != NULL) {
        result = execute(&local_context, block->ret_expr);
    }
    
    hm_free(local_context.scope.symbols);
    return result;
}

EvalResult execute_if_expr(Context *ctx, AST_Node *node, bool *executed_ret) {
    assert(node->kind == AST_IF);

    bool executed = false;
    
    AST_NodeBranch *if_node = (void*)node;
    EvalResult result = create_result(VOID_TYPE);

    EvalResult condition = execute_expr(ctx, if_node->condition);
    append_error(ctx, get_signal_error(result.sig));
    if (has_errors(ctx)) {
        return result;
    }
    
    bool condition_value = to_bool(ctx, condition.val);
    if (has_errors(ctx)) {
        return result;
    }
    
    if (condition_value) {
        result = execute(ctx, if_node->body);
        executed = true;
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
        
        return result;
    }

    for (size_t i = 0; i < if_node->elif_branches.count; ++i) {
        result = execute_if_expr(ctx, if_node->elif_branches.items[i], &executed);
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
        
        if (executed) {
            return result;
        }
    }

    if (if_node->else_branch != NULL) {
        result = execute(ctx, if_node->else_branch);
        executed = true;
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
    }
    
    return result;
}

EvalResult execute_struct_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_STRUCT && "Unexpected node");
    
    EvalResult result = create_result(VOID_TYPE);

    AST_NodeInit *struct_node = (void*)node;
    
    Symbol *type_symb = NULL;
    
    if (struct_node->node->kind == AST_NAME) {
        AST_NodeName *name_node = (void*)struct_node->node;
        
        if (!resolve_symb(ctx, name_node->name, &type_symb) || type_symb == NULL) {
            append_error(ctx, ERROR_UNKNOWN_TYPE);
        }

        if (has_errors(ctx)) {
            return result;
        }
    }
    else if (struct_node->node->kind == AST_TYPE) {
        AST_Type *name_node = (void*)struct_node->node;
        
        if (!resolve_symb(ctx, name_node->name, &type_symb)) {
            append_error(ctx, ERROR_UNKNOWN_TYPE);
        }
        
        if (has_errors(ctx)) {
            return result;
        }
    }
    else {
        append_error(ctx, ERROR_UNKNOWN_TYPE);
        return result;
    }

    Type *type = NULL;
    
    switch (type_symb->symb_kind) {
        case SYMB_VAR: {
            type = ((Var*)type_symb)->val->type;
        } break;
            
        case SYMB_TYPE: {
            type = (Type*)type_symb;
        } break;

        default: {
            append_error(ctx, ERROR_UNKNOWN_TYPE);
            return result;
        }
    }
    
    result.val = create_value(type);
    result.val.as_struct = alloc_struct_value(type);

    for (size_t i = 0; i < type->members.count; ++i) {
        Member *member = type->members.items[i];
        if (member->kind != MEMBER_FIELD) {
            continue;
        }
        
        AST_Node *initializer = NULL;

        for (size_t j = 0; j < struct_node->initializers.count; ++j) {
            AST_Initializer ast_initializer = struct_node->initializers.items[j];

            if (!ast_initializer.has_name) {
                if (i == j) {
                    if (initializer != NULL) {
                        append_error(ctx, ERROR_MULTIPLE_INITIALIZERS);
                        free(result.val.as_ptr);
                        return result;
                    }
                    
                    initializer = ast_initializer.initializer;
                }
                
                continue;
            }
            
            if (sv_cmp_sb(&ast_initializer.name, member->name)) {
                if (initializer != NULL) {
                    append_error(ctx, ERROR_MULTIPLE_INITIALIZERS);
                    free(result.val.as_ptr);
                    return result;
                }
                
                initializer = ast_initializer.initializer;
            }
        }

        if (initializer == NULL) {
            initializer = member->field.initializer;
        }

        if (initializer == NULL) {
            continue;
        }

        EvalResult initializer_res = execute_expr(ctx, initializer);
        append_error(ctx, get_signal_error_unexpected(initializer_res.sig));
        if (has_errors(ctx)) {
            return result;
        }
            
        assign_field(ctx, result.val.as_struct, member, initializer_res.val);
        if (has_errors(ctx)) {
            return result;
        }
    }
    
    return result;
}

Member *get_member_from_node(Context *ctx, AST_Node *node, Value *base) {
    Member *member = NULL;
    
    assert(node->kind == AST_MEMBER_ACCESS);
    AST_NodeMemberAccess *member_access_node = (void*)node;

    assert(member_access_node->member->kind == AST_NAME);
    AST_NodeName *member_name_node = (void*)member_access_node->member;
    
    EvalResult base_result = execute_expr(ctx, member_access_node->base);
    append_error(ctx, get_signal_error_unexpected(base_result.sig));

    if (has_errors(ctx)) {
        return member;
    }

    if (base_result.val.type->kind == TYPE_REF) {
        base_result.val = GET_REF_VALUE(base_result.val);
    }
    
    Type *type =
        base_result.val.type->kind == TYPE_TYPE
            ? base_result.val.type->el_type
            : base_result.val.type;
    if (!get_member(type, &member_name_node->name, &member)) {
        append_error(ctx, ERROR_UNKNOWN_FIELD);
        return member;
    }

    *base = base_result.val;
    return member;
}

EvalResult execute_member_access_expr(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_MEMBER_ACCESS);

    EvalResult base_result = create_result(VOID_TYPE);
    
    Value base;
    Member *member = get_member_from_node(ctx, node, &base);
    if (has_errors(ctx)) {
        return base_result;
    }

    base_result.val = base;
    
    if (base_result.val.type->kind == TYPE_TYPE) {
        if (!member->is_static) {
            append_error(ctx, ERROR_UNKNOWN_FIELD);
            return base_result;
        }
        
        base_result.val = get_static_member_val(member);
    }
    else if (base_result.val.type->kind == TYPE_REF) {
        base_result.val = GET_REF_VALUE(base_result.val);
        base_result.val = get_member_val(base_result.val.as_struct, member);
    }
    else {
        base_result.val = get_member_val(base_result.val.as_struct, member);
    }

    return base_result;
}

Value execute_name_expr(Context *ctx, AST_Node *expr) {
    AST_NodeName *name_node = (void*)expr;
    
    Value val = create_value(VOID_TYPE);
    
    Symbol *symb = NULL;
    if (!resolve_symb(ctx, name_node->name, &symb) || symb == NULL) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return val;
    }

    switch (symb->symb_kind) {
        case SYMB_VAR: {
            Var *var = (void*)symb;
            val = *var->val;
        } break;

        case SYMB_FUNC: {
            Func *func = (void*)symb;
            val.type = func->type;
            val.as_ptr = func;
        } break;

        case SYMB_TYPE: {
            Type *type = (void*)symb;
            val.type = alloc_type_type(ctx, type);
            val.as_ptr = type;
        } break;
            
        default: assert(0 && "UNREACHABLE");
    }
    
    
    return val;
}

EvalResult create_continue_signal() {
    EvalResult result = create_result(VOID_TYPE);
    result.sig = SIGNAL_CONTINUE;
    return result;
}

EvalResult create_break_signal() {
    EvalResult result = create_result(VOID_TYPE);
    result.sig = SIGNAL_BREAK;
    return result;
}

EvalResult create_return_signal(Context *ctx, AST_Node *node) {
    assert(node->kind == AST_RETURN);

    AST_NodeReturn *ret_node = (void*)node;

    EvalResult result = create_result(VOID_TYPE);
    if (ret_node->node != NULL) {
        result = execute_expr(ctx, ret_node->node);
    }
    
    result.sig = SIGNAL_RETURN;
    
    return result;
}

EvalResult execute_lit_expr(Context *ctx, AST_Node *expr) {
    assert(expr->kind == AST_LIT);

    AST_NodeLit *lit_node = (void*)expr;
    EvalResult result = create_result(VOID_TYPE);
    
    switch (lit_node->val.type) {
        case AST_TYPE_INT: {
            result.val.type = INT_TYPE;
            parse_int_sv(lit_node->val.view, &result.val.as_int);
        } break;

        case AST_TYPE_FLOAT: {
            result.val.type = FLOAT_TYPE;
            parse_float_sv(lit_node->val.view, &result.val.as_float);
        } break;

        case AST_TYPE_BOOL: {
            result.val.type = BOOL_TYPE;
            result.val.as_int = lit_node->val.as_bool;
        } break;

        case AST_TYPE_CHAR: {
            result.val.type = CHAR_TYPE;
            result.val.as_int = lit_node->val.as_char;
        } break;

        case AST_TYPE_STR: {
            result.val.type = STR_TYPE;
            String_Builder *sb = sb_alloc();
            sv_to_escaped_sb(sb, &lit_node->val.view);
            result.val.as_ptr = sb;
        } break;
    }

    return result;
}

EvalResult execute_expr(Context *ctx, AST_Node *expr) {
    EvalResult result = create_result(VOID_TYPE);

    switch (expr->kind) {
        case AST_BINOP: {
            result.val = execute_binary_expr(ctx, expr);
        } break;

        case AST_UNOP: {
            result.val = execute_unary_expr(ctx, expr);
        } break;
        
        case AST_LIT: {
            result = execute_lit_expr(ctx, expr);
        } break;

        case AST_ARR: {
            result.val = execute_arr_expr(ctx, expr);
        } break;

        case AST_NAME: {
            result.val = execute_name_expr(ctx, expr);
        } break;

        case AST_CALL: {
            result.val = execute_call_expr(ctx, expr);
        } break;

        case AST_INDEX: {
            result.val = execute_index_expr(ctx, expr);
        } break;

        case AST_BLOCK: {
            result = execute_block_expr(ctx, expr);
        } break;

        case AST_IF: {
            result = execute_if_expr(ctx, expr, NULL);
        } break;

        case AST_STRUCT: {
            result = execute_struct_expr(ctx, expr);
        } break;

        case AST_MEMBER_ACCESS: {
            result = execute_member_access_expr(ctx, expr);
        } break;

        case AST_CONTINUE: {
            result = create_continue_signal();
        } break;
            
        case AST_BREAK: {
            result = create_break_signal();
        } break;

        case AST_RETURN: {
            result = create_return_signal(ctx, expr);
        } break;

        default: assert(0 && "UNREACHABLE");
    }

    return result;
}

EvalResult execute(Context *ctx, AST_Node *node) {
    EvalResult result = create_result(VOID_TYPE);
    
    switch (node->kind) {
        case AST_FUNC_DECL: {
            register_func(ctx, node);
        } break;

        case AST_STRUCT_DECL: {
            register_struct(ctx, node);
        } break;

        case AST_LET: {
            register_var(ctx, node);
        } break;
        
        case AST_FOR: {
            result = execute_loop_stmt(ctx, node);
        } break;
            
        default: {
            result = execute_expr(ctx, node);
        }
    }
    
    return result;
}

// TODO: Implement time-dependent declarations

EvalResult execute_nodes(Context *ctx, AST_Nodes *nodes) {
    EvalResult result = create_result(VOID_TYPE);

    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        if (node->kind != AST_FUNC_DECL && node->kind != AST_STRUCT_DECL) {
            continue;
        }

        result = execute(ctx, node);
        if (result.sig != SIGNAL_NONE) {
            return result;
        }
        
        if (has_errors(ctx)) {
            return result;
        }
    }
    
    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        
        if (node->kind == AST_FUNC_DECL || node->kind == AST_STRUCT_DECL) {
            continue;
        }
        
        result = execute(ctx, node);
        if (result.sig != SIGNAL_NONE) {
            return result;
        }
        
        if (has_errors(ctx)) {
            return result;
        }
    }

    return result;
}

// TODO: Implement global context
void execute_program(Context *ctx, AST_NodeProgram *program) {
    EvalResult result = execute_nodes(ctx, &program->nodes);
    ast_free((void*)program);
    append_error(ctx, get_signal_error(result.sig));
}