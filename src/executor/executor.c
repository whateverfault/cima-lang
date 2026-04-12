#include <string.h>
#include <assert.h>

#include "parser/parser.h"

#define NOTHING_IMPLEMENTATION
#include "nothing/nothing.h"
#include "types/type.h"
#include "other/operations.h"

#include "executor.h"
#include "funcs.h"
#include "other/built_in.h"

void context_init(Context *context) {
    context->global = context;
    context->scope.vars = hm_alloc();
    context->scope.funcs = hm_alloc();
    context->errors = (Errors*)calloc(1, sizeof(Errors));
}

void context_free(Context *context) {
    hm_free(context->scope.vars);
    hm_free(context->scope.funcs);
    da_pfree(context->errors);
}

bool has_errors(Context *context) {
    return context->errors->count > 0;
}

void append_error(Context *context, ErrorKind err) {
    if (err == ERROR_NONE) {
        return;
    }
    
    da_append(context->errors, err);
}

ErrorKind get_signal_error(Signal sig) {
    switch (sig) {
        case SIGNAL_CONTINUE: return ERROR_CONTINUE_OUTSIDE_LOOP;
        case SIGNAL_BREAK: return ERROR_BREAK_OUTSIDE_LOOP;
        default: return ERROR_NONE;
    }
}

ErrorKind get_signal_error_unexpected(Signal sig) {
    switch (sig) {
        case SIGNAL_CONTINUE: return ERROR_UNEXPECTED_CONTINUE;
        case SIGNAL_BREAK: return ERROR_UNEXPECTED_BREAK;
        case SIGNAL_RETURN: return ERROR_UNEXPECTED_RETURN;
        default: return ERROR_NONE;
    }
}

Array *alloc_arr(ValueType *el_type) {
    Array *arr = (Array*)malloc(sizeof(Array));
    assert(arr != NULL && "Memory allocation failed");

    arr->els = (ArrayElements){0};
    arr->el_type = el_type;
    
    return arr;
}

Var *alloc_var(String_Builder *name_sb, Value val, bool constant) {
    Var *var = (Var*)malloc(sizeof(Var));
    assert(var != NULL && "Memory allocation failed");

    var->name = name_sb;
    var->val = val;
    var->constant = constant;
    
    return var;
}

FuncCustom *alloc_custom_func(String_Builder *name, Patterns args, AST_Node *body, bool constant) {
    FuncCustom *func = (FuncCustom*)malloc(sizeof(FuncCustom));
    assert(func != NULL && "Memory allocation failed");

    func->kind = FUNC_CUSTOM;
    func->name = name;
    func->body = body;
    func->args = args;
    func->constant = constant;

    return func;
}

EvalResult create_result(ValueType *type) {
    return (EvalResult){
        .sig = SIGNAL_NONE,
        .val = create_value(type),
    };
}

bool get_var(Context *context, String_View *name_sv, Var **variable) {
    for (size_t i = 0; i < builtin_vars_count; ++i) {
        if (sv_cmp_sb(name_sv, builtin_vars[i].name)) {
            *variable = &builtin_vars[i];
            return true;
        }
    }

    *variable = hm_nget(context->scope.vars, name_sv->items, name_sv->count);
    return *variable != NULL;
}

bool get_func(Context *context, String_View *name_sv, Func **func) {
    for (size_t i = 0; i < builtin_funcs_count; ++i) {
        if (sv_cmp_sb(name_sv, builtin_funcs[i].name)) {
            *func = (void*)&builtin_funcs[i];
            return true;
        }
    }

    *func = hm_nget(context->scope.funcs, name_sv->items, name_sv->count);
    return *func != NULL;
}

bool resolve_name(Context *context, AST_Node *name, Var **var) {
    AST_NodeName *name_node = (AST_NodeName*)name;
    return get_var(context, &name_node->name, var) && var != NULL;
}

bool resolve_name_sv(Context *context, String_View *name_sv, Var **var) {
    return get_var(context, name_sv, var) && var != NULL;
}

bool resolve_name_cstr(Context *context, char *name, Var **var) {
    String_View sv = {
        .items = name,
        .count = strlen(name),
    };
    return resolve_name_sv(context, &sv, var);
}

bool resolve_name_node(Context *context, AST_Node *node, Var **var) {
    AST_NodeName *name_node = (void*)node;
    return resolve_name_sv(context, &name_node->name, var);
}

bool resolve_func(Context *context, AST_Node *node, Func **func) {
    AST_NodeCall *call_node = (void*)node;
    return get_func(context, &call_node->name, func) && func != NULL;
}

bool is_node_value(AST_Node *node) {
    return node->kind == AST_LIT || node->kind == AST_NAME || node->kind == AST_CALL || node->kind == AST_ARR;
}

bool is_assignable(AST_Node *node) {
    switch (node->kind) {
        case AST_INDEX:
        case AST_NAME: return true;

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

void register_func(Context *context, AST_Node *node) {
    assert(node->kind == AST_FN);

    Func *check = NULL;
    if (resolve_func(context, node, &check)
    &&  check != NULL && check->constant) {
        append_error(context, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    AST_NodeFuncDecl *func_node = (AST_NodeFuncDecl*)node;
    
    String_Builder *sb = sb_alloc();
    sv_to_sb(&func_node->name, sb);
    
    FuncCustom *func = alloc_custom_func(sb, func_node->args, func_node->body, func_node->constant);
    assert(hm_nput(context->scope.funcs, func_node->name.items, func_node->name.count, func) == 0 && "Failed to register function.");
}

void register_var(Context *context, AST_Node *node) {
    assert(node->kind == AST_LET);

    Var *check = NULL;
    if (resolve_name(context, node, &check)
    &&  check != NULL && check->constant) {
        append_error(context, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    AST_NodeLetStmt *let_node = (void*)node;

    EvalResult result = create_result(let_node->type);
    if (let_node->initializer != NULL) {
        result = execute_expr(context, let_node->initializer);
        append_error(context, get_signal_error(result.sig));
        if (has_errors(context)) {
            return;
        }
    }

    Var *var = NULL;
    String_Builder *sb = sb_alloc();
    sv_to_sb(&let_node->name, sb);
    
    if (!resolve_name_sv(context, &let_node->name, &var)) {
        var = alloc_var(sb, result.val, let_node->constant);
        assert(hm_nput(context->scope.vars, let_node->name.items, let_node->name.count, var) == 0 && "Failed to register function.");
        return;
    }

    var->val = result.val;
    var->constant = let_node->constant;
}

EvalResult execute_loop_stmt(Context *context, AST_Node *node) {
    assert(node->kind == AST_FOR);

    AST_NodeForStmt *for_node = (void*)node;

    EvalResult result = create_result(VOID_TYPE);

    if (for_node->initializer != NULL) {
        result = execute(context, for_node->initializer);
        append_error(context, get_signal_error(result.sig));
        if (has_errors(context)) {
            return result;
        }
    }

    while (true) {
        EvalResult condition = create_result(BOOL_TYPE);
        condition.val.as_int = true;
        
        if (for_node->condition != NULL) {
            condition = execute_expr(context, for_node->condition);
            append_error(context, get_signal_error(condition.sig));
            if (has_errors(context)) {
                return result;
            }
        }

        bool condition_value = to_bool(context, condition.val);
        if (has_errors(context)) {
            return result;
        }
        
        if (!condition_value) {
            break;
        }

        result = execute(context, for_node->body);
        if (result.sig == SIGNAL_CONTINUE) {
            if (for_node->next != NULL) {
                EvalResult next_result = execute_expr(context, for_node->next);
                append_error(context, get_signal_error(next_result.sig));
                if (has_errors(context)) {
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
        
        if (has_errors(context)) {
            return result;
        }

        if (for_node->next != NULL) {
            EvalResult next_result = execute_expr(context, for_node->next);
            append_error(context, get_signal_error(next_result.sig));
            if (has_errors(context)) {
                return result;
            }
        }
    }

    return result;
}

void unwrap_index_expr(Context *context, AST_Node *node, Value *arr, Value* index) {
    assert(node->kind == AST_INDEX);

    AST_NodeIndex *index_node = (void*)node;

    EvalResult arr_res = execute_expr(context, index_node->node);
    append_error(context, get_signal_error(arr_res.sig));
    if (has_errors(context)) {
        return;
    }

    if (arr_res.val.type->tag != TYPE_STR && arr_res.val.type->tag != TYPE_ARRAY) {
        append_error(context, ERROR_INCOMPATIBLE_TYPES);
        return;
    }
    
    EvalResult index_res = execute_expr(context, index_node->index);
    append_error(context, get_signal_error(index_res.sig));
    if (has_errors(context)) {
        return;
    }

    if (index_res.val.type->tag != TYPE_INT) {
        append_error(context, ERROR_INCOMPATIBLE_TYPES);
        return;
    }

    *arr = arr_res.val;
    *index = index_res.val;
}

void assign_var(Context *context, AST_NodeName *name_node, Value val) {
    String_View name_sv = name_node->name;

    Var *var = NULL;
    if (resolve_name_sv(context, &name_sv, &var)) {
        if (var->constant) {
            append_error(context, ERROR_CANNOT_REASSIGN_CONST);
            return;
        }
    }
    else {
        append_error(context, ERROR_NOT_DEFINED);
        return;
    }

    String_Builder *sb = sb_alloc();
    sv_to_sb(&name_sv, sb);
            
    var->val = val;
}

void assign_arr(Context *context, AST_NodeIndex *index_node, Value val) {
    Value arr_val;
    Value index;
    unwrap_index_expr(context, (void*)index_node, &arr_val, &index);
    
    switch (arr_val.type->tag) {
        case TYPE_ARRAY: {
            Array *arr = arr_val.as_ptr;
            arr->els.items[index.as_int] = val;
        } break;

        case TYPE_STR: {
            if (val.type->tag != TYPE_CHAR) {
                append_error(context, ERROR_INCOMPATIBLE_TYPES);
                return;
            }
            
            String_Builder *str = arr_val.as_ptr;
            str->items[index.as_int] = val.as_int;
        } break;
            
        default: append_error(context, ERROR_INCOMPATIBLE_TYPES);
    }
}

void assign(Context *context, AST_Node *node, Value val) {
    switch (node->kind) {
        case AST_NAME: {
            assign_var(context, (void*)node, val);
        } break;

        case AST_INDEX: {
            assign_arr(context, (void*)node, val);
        } break;
            
        default: {
            append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
        }
    }
}

Value execute_binop(Context *context, AST_Node *node) {
    assert(node->kind == AST_BINOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeBinOp *binop = (void*)node;

    EvalResult lhs = create_result(VOID_TYPE);
    if (!is_assignment(node)) {
        lhs = execute_expr(context, binop->lhs);
        if (lhs.sig != SIGNAL_NONE) {
            append_error(context, get_signal_error_unexpected(lhs.sig));
        }
        
        if (has_errors(context)) {
            return val;
        }
    }

    if (binop->op == BINOP_LOGIC_AND) {
        bool lhs_bool = to_bool(context, lhs.val);
        if (has_errors(context)) {
            return val;
        }

        if (!lhs_bool) {
            val.type = BOOL_TYPE;
            val.as_int = false;
            return val;
        }
    }
    
    EvalResult rhs = execute_expr(context, binop->rhs);
    if (rhs.sig != SIGNAL_NONE) {
        append_error(context, get_signal_error_unexpected(rhs.sig));
    }
    
    if (has_errors(context)) {
        return val;
    }
    
    switch (binop->op) {
        case BINOP_PLUS: {
            val = binary_plus(context, lhs.val, rhs.val);
        } break;

        case BINOP_MINUS: {
            val = binary_minus(context, lhs.val, rhs.val);
        } break;

        case BINOP_MUL: {
            val = binary_mul(context, lhs.val, rhs.val);
        } break;

        case BINOP_DIV: {
            val = binary_div(context, lhs.val, rhs.val);
        } break;

        case BINOP_MOD: {
            val = binary_mod(context, lhs.val, rhs.val);
        } break;

        case BINOP_POW: {
            val = binary_pow(context, lhs.val, rhs.val);
        } break;

        case BINOP_LOGIC_AND: {
            val = binary_logic_and(context, lhs.val, rhs.val);
        } break;

        case BINOP_BIT_AND: {
            val = binary_bitwise_and(context, lhs.val, rhs.val);
        } break;

        case BINOP_LOGIC_OR: {
            val = binary_logic_or(context, lhs.val, rhs.val);
        } break;

        case BINOP_BIT_OR: {
            val = binary_bitwise_or(context, lhs.val, rhs.val);
        } break;

        case BINOP_GT: {
            val = binary_gt(context, lhs.val, rhs.val);
        } break;
            
        case BINOP_LT: {
            val = binary_lt(context, lhs.val, rhs.val);
        } break;

        case BINOP_GTEQ: {
            val = binary_gteq(context, lhs.val, rhs.val);
        } break;
            
        case BINOP_LTEQ: {
            val = binary_lteq(context, lhs.val, rhs.val);
        } break;
            
        case BINOP_EQ: {
            val = binary_eq(context, lhs.val, rhs.val);
        } break;

        case BINOP_NEQ: {
            val = binary_neq(context, lhs.val, rhs.val);
        } break;

        case BINOP_ASSIGN: {
            if (!is_assignable(binop->lhs)) {
                append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            assign(context, (void*)binop->lhs, rhs.val);
        } break;
            
        default: break;
    }
    
    return val;
}

Value execute_unop(Context *context, AST_Node *node) {
    assert(node->kind == AST_UNOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeUnOp *unop = (void*)node;
    
    EvalResult result = execute_expr(context, unop->expr);
    if (result.sig != SIGNAL_NONE) {
        append_error(context, get_signal_error_unexpected(result.sig));
    }

    if (has_errors(context)) {
        return val;
    }
    
    switch (unop->op) {
        case UNOP_INCREMENT: {
            if (!is_assignable(unop->expr)) {
                append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            val = unary_increment(context, result.val);
            
            assign(context, (void*)unop->expr, val);
            return val;
        }
        case UNOP_DECREMENT: {
            if (!is_assignable(unop->expr)) {
                append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            val = unary_decrement(context, result.val);
            
            assign(context, (void*)unop->expr, val);
            return val;
        }
        case UNOP_NOT: {
            val = unary_not(context, result.val);
            return val;
        }
        case UNOP_PLUS: {
            val = unary_plus(context, result.val);
            return val;
        }
        case UNOP_MINUS: {
            val = unary_minus(context, result.val);
            return val;
        }
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

Value execute_call_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_CALL);

    Value value = create_value(VOID_TYPE);
    
    AST_NodeCall *call = (AST_NodeCall*)node;
    Func *func = NULL;
    if (!resolve_func(context, node, &func)) {
        append_error(context, ERROR_NOT_DEFINED);
        return value;
    }

    return exec_func(context, func, call->args);
}

Value execute_index_expr(Context *context, AST_Node *node) {
    Value value = create_value(VOID_TYPE);
    
    Value arr;
    Value index;
    unwrap_index_expr(context, node, &arr, &index);
    if (has_errors(context)) {
        return value;
    }
    
    switch (arr.type->tag) {
        case TYPE_ARRAY: {
            Array *array = arr.as_ptr;
            value = create_value(array->el_type);

            if (index.as_int < 0 || index.as_int >= array->els.count) {
                append_error(context, ERROR_INDEX_OUT_OF_BOUNDS);
                return value;
            }
            
            value = array->els.items[index.as_int];
            return value;
        }

        case TYPE_STR: {
            String_Builder *str = arr.as_ptr;
            value = create_value(CHAR_TYPE);

            if (index.as_int < 0 || index.as_int >= str->count) {
                append_error(context, ERROR_INDEX_OUT_OF_BOUNDS);
                return value;
            }
            
            value.as_int = str->items[index.as_int];
            return value;
        }

        default: assert(0 && "UNREACHABLE");
    }
}

Value execute_arr_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_ARR);
    
    AST_NodeArray *arr_node = (void*)node;
    
    // TODO: Type inference
    Value val = create_value(ARRAY_ANY_TYPE);
    Array *arr = alloc_arr(ANY_TYPE);
    da_reserve(&arr->els, arr_node->nodes.count);

    for (size_t i = 0; i < arr_node->nodes.count; ++i) {
        EvalResult el = execute_expr(context, arr_node->nodes.items[i]);
        append_error(context, get_signal_error(el.sig));
        if (has_errors(context)) {
            da_free(arr->els);
            free(arr);
            return val;
        }
        
        da_append(&arr->els, el.val);
    }

    val.as_ptr = arr;
    return val;
}

EvalResult execute_block_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_BLOCK);
    AST_NodeBlock *block = (void*)node;

    Context local_context = {
        .global = context->global,
        .scope = (Scope){
            .vars = hm_copy(context->scope.vars),
            .funcs = hm_copy(context->scope.funcs),
        },
        .errors = context->errors, 
    };
    
    EvalResult result = execute_nodes(&local_context, &block->nodes);
    if (result.sig != SIGNAL_NONE) {
        hm_free(local_context.scope.vars);
        hm_free(local_context.scope.funcs);
        return result;
    }
    
    if (has_errors(context)) {
        return result;
    }
    
    if (block->ret_expr != NULL) {
        result = execute(&local_context, block->ret_expr);
    }
    
    hm_free(local_context.scope.vars);
    hm_free(local_context.scope.funcs);
    return result;
}

EvalResult execute_if_expr(Context *context, AST_Node *node, bool *executed_ret) {
    assert(node->kind == AST_IF);

    bool executed = false;
    
    AST_NodeBranch *if_node = (void*)node;
    EvalResult result = create_result(VOID_TYPE);

    EvalResult condition = execute_expr(context, if_node->condition);
    append_error(context, get_signal_error(result.sig));
    if (has_errors(context)) {
        return result;
    }
    
    bool condition_value = to_bool(context, condition.val);
    if (has_errors(context)) {
        return result;
    }
    
    if (condition_value) {
        result = execute(context, if_node->body);
        executed = true;
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
        
        return result;
    }

    for (size_t i = 0; i < if_node->elif_branches.count; ++i) {
        result = execute_if_expr(context, if_node->elif_branches.items[i], &executed);
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
        
        if (executed) {
            return result;
        }
    }

    if (if_node->else_branch != NULL) {
        result = execute(context, if_node->else_branch);
        executed = true;
        if (executed_ret != NULL) {
            *executed_ret = executed;
        }
    }
    
    return result;
}

Value execute_name_expr(Context *context, AST_Node *expr) {
    Value val = create_value(VOID_TYPE);
    
    Var *var = NULL;
    if (!resolve_name_node(context, expr, &var) || var == NULL) {
        append_error(context, ERROR_NOT_DEFINED);
        return val;
    }
            
    val = var->val;
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

EvalResult create_return_signal(Context *context, AST_Node *node) {
    assert(node->kind == AST_RETURN);

    AST_NodeReturn *ret_node = (void*)node;

    EvalResult result = create_result(VOID_TYPE);
    if (ret_node->node != NULL) {
        result = execute_expr(context, ret_node->node);
    }
    
    result.sig = SIGNAL_RETURN;
    
    return result;
}

EvalResult execute_expr(Context *context, AST_Node *expr) {
    EvalResult result = create_result(VOID_TYPE);

    switch (expr->kind) {
        case AST_BINOP: {
            result.val = execute_binop(context, expr);
        } break;

        case AST_UNOP: {
            result.val = execute_unop(context, expr);
        } break;
        
        case AST_LIT: {
            result.val = ((AST_NodeLit*)expr)->val;
        } break;

        case AST_ARR: {
            result.val = execute_arr_expr(context, expr);
        } break;

        case AST_NAME: {
            result.val = execute_name_expr(context, expr);
        } break;

        case AST_CALL: {
            result.val = execute_call_expr(context, expr);
        } break;

        case AST_INDEX: {
            result.val = execute_index_expr(context, expr);
        } break;

        case AST_BLOCK: {
            result = execute_block_expr(context, expr);
        } break;

        case AST_IF: {
            result = execute_if_expr(context, expr, NULL);
        } break;

        case AST_CONTINUE: {
            result = create_continue_signal();
        } break;
            
        case AST_BREAK: {
            result = create_break_signal();
        } break;

        case AST_RETURN: {
            result = create_return_signal(context, expr);
        } break;
            
        case AST_ERROR: {
            append_error(context, ((AST_NodeError*)expr)->err);
        } break;

        default: assert(0 && "UNREACHABLE");
    }

    return result;
}

EvalResult execute(Context *context, AST_Node *node) {
    EvalResult result = create_result(VOID_TYPE);
    
    switch (node->kind) {
        case AST_FN: {
            register_func(context, node);
        } break;

        case AST_LET: {
            register_var(context, node);
        } break;
        
        case AST_FOR: {
            result = execute_loop_stmt(context, node);
        } break;
            
        default: {
            result = execute_expr(context, node);
        }
    }
    
    return result;
}

EvalResult execute_nodes(Context *context, Nodes *nodes) {
    EvalResult result = create_result(VOID_TYPE);
    
    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        if (node->kind != AST_FN) {
            if (node->kind == AST_ERROR) {
                append_error(context, ((AST_NodeError*)node)->err);
                return result;
            }
            
            continue;
        }

        result = execute(context, node);
        if (result.sig != SIGNAL_NONE) {
            return result;
        }
        
        if (has_errors(context)) {
            return result;
        }
    }
    
    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        
        if (node->kind == AST_FN) {
            continue;
        }
        
        if (node->kind == AST_ERROR) {
            append_error(context, ((AST_NodeError*)node)->err);
            return result;
        }
        
        result = execute(context, node);
        if (result.sig != SIGNAL_NONE) {
            return result;
        }
        
        if (has_errors(context)) {
            return result;
        }
    }

    return result;
}

// TODO: Implement global context
void execute_program(Context *context, AST_NodeProgram *program) {
    EvalResult result = execute_nodes(context, &program->nodes);
    ast_free((void*)program);

    append_error(context, get_signal_error(result.sig));
}