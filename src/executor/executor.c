#include <string.h>
#include <assert.h>

#include "parser/parser.h"

#define NOTHING_IMPLEMENTATION
#include "nothing/nothing.h"
#include "../types/type.h"
#include "../other/operations.h"

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
    da_free(*context->errors);
}

bool has_errors(Context *context) {
    return context->errors->count > 0;
}

void append_error(Context *context, ErrorKind err) {
    da_append(context->errors, err);
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

bool is_assignment(AST_Node *node) {
    bool result = node->kind == AST_BINOP;
    AST_NodeBinOp *op = (void*)node;

    result &= op->op == BINOP_EQ;
    return result;
}

void register_func(Context *context, AST_Node *node) {
    assert(node->kind == AST_FN);

    Func *check = NULL;
    if (resolve_func(context, node, &check)
    &&  check != NULL && check->constant) {
        append_error(context, ERROR_CANNOT_REASSIGN_CONST);
        return;
    }
    
    AST_NodeFunc *func_node = (AST_NodeFunc*)node;
    
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
    
    AST_NodeLet *let_node = (void*)node;

    Value val = create_value(let_node->type);
    if (let_node->has_initializer) {
        val = execute_expr(context, let_node->initializer);
        if (has_errors(context)) {
            return;
        }
    }

    Var *var = NULL;
    String_Builder *sb = sb_alloc();
    sv_to_sb(&let_node->name, sb);
    
    if (!resolve_name_sv(context, &let_node->name, &var)) {
        var = alloc_var(sb, val, let_node->constant);
        assert(hm_nput(context->scope.vars, let_node->name.items, let_node->name.count, var) == 0 && "Failed to register function.");
        return;
    }

    var->val = val;
    var->constant = let_node->constant;
}

Value execute_binop(Context *context, AST_Node *root) {
    assert(root->kind == AST_BINOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeBinOp *binop = (void*)root;

    Value lhs = create_value(VOID_TYPE);
    if (!is_assignment(root)) {
        lhs = execute_expr(context, binop->lhs);
        if (has_errors(context)) {
            return val;
        }
    }
    
    Value rhs = execute_expr(context, binop->rhs);
    if (has_errors(context)) {
        return val;
    }
    
    switch (binop->op) {
        case BINOP_PLUS: {
            val = binary_plus(context, lhs, rhs);
        } break;

        case BINOP_MINUS: {
            val = binary_minus(context, lhs, rhs);
        } break;

        case BINOP_MUL: {
            val = binary_mul(context, lhs, rhs);
        } break;

        case BINOP_DIV: {
            val = binary_div(context, lhs, rhs);
        } break;

        case BINOP_MOD: {
            val = binary_mod(context, lhs, rhs);
        } break;

        case BINOP_POW: {
            val = binary_pow(context, lhs, rhs);
        } break;

        case BINOP_EQ: {
            if (binop->lhs->kind != AST_NAME) {
                append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                return val;
            }
            
            String_View name_sv = ((AST_NodeName*)binop->lhs)->name;

            Var *var = NULL;
            if (resolve_name_sv(context, &name_sv, &var)) {
                if (var->constant) {
                    append_error(context, ERROR_CANNOT_REASSIGN_CONST);
                    return val;
                }
            }
            else {
                append_error(context, ERROR_NOT_DEFINED);
                return val;
            }

            String_Builder *sb = sb_alloc();
            sv_to_sb(&name_sv, sb);
            
            var = alloc_var(sb, rhs, var->constant);
            hm_nput(context->scope.vars, name_sv.items, name_sv.count, var);

            val = rhs;
        } break;
            
        default: break;
    }
    
    return val;
}

Value execute_unop(Context *context, AST_Node *root) {
    assert(root->kind == AST_UNOP);
    Value val = create_value(VOID_TYPE);

    AST_NodeUnOp *unop = (void*)root;
    Value value = execute_expr(context, unop->expr);
    if (has_errors(context)) {
        return val;
    }
    
    switch (unop->op) {
        case UNOP_PLUS: {
            val = unary_plus(context, value);
            if (has_errors(context)) {
                return val;
            }
            
            return val;
        }
        case UNOP_MINUS: {
            val = unary_minus(context, value);
            if (has_errors(context)) {
                return val;
            }
            
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

// TODO: Implement arrays/strings indexing
Value execute_index_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_INDEX);

    AST_NodeIndex *index_node = (void*)node;
    Value value = create_value(VOID_TYPE);

    Value arr = execute(context, index_node->node);
    if (has_errors(context)) {
        return value;
    }
    
    Value index = execute(context, index_node->index);
    if (has_errors(context)) {
        return value;
    }
    
    AST_NodeCall *call = (AST_NodeCall*)node;
    Func *func = NULL;
    if (!resolve_func(context, node, &func)) {
        append_error(context, ERROR_NOT_DEFINED);
        return value;
    }

    return exec_func(context, func, call->args);
}

Value execute_arr_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_ARR);
    
    AST_NodeArray *arr_node = (void*)node;
    
    // TODO: Type inference
    Value val = create_value(ARRAY_ANY_TYPE);
    Array *arr = alloc_arr(ANY_TYPE);
    da_reserve(&arr->els, arr_node->nodes.count);

    for (size_t i = 0; i < arr_node->nodes.count; ++i) {
        Value el = execute(context, arr_node->nodes.items[i]);
        if (has_errors(context)) {
            da_free(arr->els);
            free(arr);
            return val;
        }
        
        da_append(&arr->els, el);
    }

    val.as_ptr = arr;
    return val;
}

Value execute_block_expr(Context *context, AST_Node *node) {
    assert(node->kind == AST_BLOCK);
    Value ret = create_value(VOID_TYPE);

    AST_NodeBlock *block = (void*)node;

    Context local_context = {
        .global = context->global,
        .scope = (Scope){
            .vars = hm_copy(context->scope.vars),
            .funcs = hm_copy(context->scope.funcs),
        },
        .errors = context->errors, 
    };
    
    execute_nodes(&local_context, &block->nodes);
    if (has_errors(context)) {
        return ret;
    }
    
    if (block->ret_expr != NULL) {
        ret = execute(&local_context, block->ret_expr);
    }
    
    hm_free(local_context.scope.vars);
    hm_free(local_context.scope.funcs);
    return ret;
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

Value execute_expr(Context *context, AST_Node *expr) {
    Value val = create_value(VOID_TYPE);

    switch (expr->kind) {
        case AST_BINOP: {
            val = execute_binop(context, expr);
        } break;

        case AST_UNOP: {
            val = execute_unop(context, expr);
        } break;
        
        case AST_LIT: {
            val = ((AST_NodeLit*)expr)->val;
        } break;

        case AST_ARR: {
            val = execute_arr_expr(context, expr);
        } break;

        case AST_NAME: {
            val = execute_name_expr(context, expr);
        } break;

        case AST_CALL: {
            val = execute_call_expr(context, expr);
        } break;

        case AST_INDEX: {
            val = execute_index_expr(context, expr);
        } break;

        case AST_BLOCK: {
            val = execute_block_expr(context, expr);
        } break;
            
        case AST_ERROR: {
            append_error(context, ((AST_NodeError*)expr)->err);
        } break;

        default: assert(0 && "UNREACHABLE");
    }

    return val;
}

Value execute(Context *context, AST_Node *node) {
    Value value = create_value(VOID_TYPE);
    
    switch (node->kind) {
        case AST_FN: {
            register_func(context, node);
        } break;

        case AST_LET: {
            register_var(context, node);
        } break;

        default: {
            value = execute_expr(context, node);
        }
    }
    
    return value;
}

void execute_nodes(Context *context, Nodes *nodes) {
    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        if (node->kind != AST_FN) {
            if (node->kind == AST_ERROR) {
                append_error(context, ((AST_NodeError*)node)->err);
                return;
            }
            
            continue;
        }

        execute(context, node);
        if (has_errors(context)) {
            return;
        }
    }
    
    for (size_t i = 0; i < nodes->count; ++i) {
        AST_Node *node = nodes->items[i];
        
        if (node->kind == AST_FN) {
            continue;
        }
        
        if (node->kind == AST_ERROR) {
            append_error(context, ((AST_NodeError*)node)->err);
            return;
        }
        
        execute(context, node);
        if (has_errors(context)) {
            return;
        }
    }
}

// TODO: Implement global context
void execute_program(Context *context, AST_NodeProgram *program) {
    execute_nodes(context, &program->nodes);
    ast_free((void*)program);
}