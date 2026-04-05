#include <string.h>
#include <assert.h>
#include <math.h>

#include "parser/parser.h"

#define NOTHING_IMPLEMENTATION
#include "nothing/nothing.h"
#include "../types/type.h"
#include "../other/operations.h"

#include "executor.h"
#include "funcs.h"
#include "other/built_in.h"

void context_init(Context *context) {
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

Var *alloc_var(String_Builder name, Value val, bool constant) {
    Var *var = (Var*)malloc(sizeof(Var));
    assert(var != NULL && "Memory allocation failed");

    var->name = name;
    var->val = val;
    var->constant = constant;
    
    return var;
}

FuncCustom *alloc_custom_func(String_Builder name, Patterns args, AST_Node *body, bool constant) {
    FuncCustom *func = (FuncCustom*)malloc(sizeof(FuncCustom));
    assert(func != NULL && "Memory allocation failed");

    func->kind = FUNC_CUSTOM;
    func->name = name;
    func->body = body;
    func->args = args;
    func->constant = constant;

    return func;
}

bool get_var(Context *context, String_View name_sv, Var **variable) {
    for (size_t i = 0; i < builtin_vars_count; ++i) {
        if (sv_cmp_sb(name_sv, builtin_vars[i].name)) {
            *variable = &builtin_vars[i];
            return true;
        }
    }

    *variable = hm_nget(context->scope.vars, name_sv.items, name_sv.count);
    return *variable != NULL;
}

bool get_func(Context *context, String_View name_sv, Func **func) {
    for (size_t i = 0; i < builtin_funcs_count; ++i) {
        if (sv_cmp_sb(name_sv, builtin_funcs[i].name)) {
            *func = (void*)&builtin_funcs[i];
            return true;
        }
    }

    *func = hm_nget(context->scope.funcs, name_sv.items, name_sv.count);
    return *func != NULL;
}

bool resolve_name(Context *context, String_View name_sv, Var **var) {
    return get_var(context, name_sv, var) && var != NULL;
}

bool resolve_name_cstr(Context *context, char *name, Var **var) {
    String_View sv = {
        .items = name,
        .count = strlen(name),
    };
    return resolve_name(context, sv, var);
}

bool resolve_name_node(Context *context, AST_Node *node, Var **var) {
    AST_NodeName *name_node = (void*)node;
    return resolve_name(context, name_node->name, var);
}

bool resolve_func(Context *context, AST_Node *node, Func **func) {
    AST_NodeCall *call_node = (void*)node;
    return get_func(context, call_node->name, func) && func != NULL;
}

bool is_node_value(AST_Node *node) {
    return node->kind == AST_LIT || node->kind == AST_NAME || node->kind == AST_CALL;
}

bool is_assignment(AST_Node *node) {
    bool result = node->kind == AST_BINOP;
    AST_NodeBinOp *op = (void*)node;

    result &= op->op == BINOP_EQ;
    return result;
}

Value get_node_value(Context *context, AST_Node *node) {
    Value value = alloc_value(VOID_TYPE);
    
    if (node->kind == AST_ERROR) {
        da_append(context->errors, ((AST_NodeError*)node)->err);
        return value;
    }

    if (!is_node_value(node)) {
        da_append(context->errors, ERROR_NOT_DEFINED);
        return value;
    }

    switch (node->kind) {
        case AST_LIT: {
            value = ((AST_NodeLit*)node)->val;
        } break;

        case AST_NAME: {
            Var *var = NULL;
            if (!resolve_name_node(context, node, &var) || var == NULL) {
                da_append(context->errors, ERROR_NOT_DEFINED);
                return value;
            }
            
            value = var->val;
        } break;

        case AST_CALL: {
            Func *func = NULL;
            if (!resolve_func(context, node, &func) || func == NULL) {
                da_append(context->errors, ERROR_NOT_DEFINED);
                return value;
            }

            AST_NodeCall *call = ((AST_NodeCall*)node);
            value = exec_func(context, func, call->args);
            if (has_errors(context)) {
                return value;
            }
        } break;
            
        default: break;
    }

    return value;
}

void register_func(Context *context, AST_Node *node) {
    assert(node->kind == AST_FUNC);

    AST_NodeFunc *func_node = (AST_NodeFunc*)node;
    FuncCustom *func = alloc_custom_func(sv_to_sb(func_node->name), func_node->args, func_node->body, false);

    assert(hm_nput(context->scope.funcs, func_node->name.items, func_node->name.count, func) == 0 && "Failed to register function.");
}

void register_var(Context *context, AST_Node *node) {
    assert(node->kind == AST_LET);

    AST_NodeLet *let_node = (void*)node;
    Value val = execute_expr(context, let_node->initializer);
    if (has_errors(context)) {
        return;
    }

    Var *var = NULL;
    if (!resolve_name(context, let_node->name, &var)) {
        var = alloc_var(sv_to_sb(let_node->name), val, let_node->constant);
        assert(hm_nput(context->scope.vars, let_node->name.items, let_node->name.count, var) == 0 && "Failed to register function.");
        return;
    }

    var->val = val;
    var->constant = let_node->constant;
}

Value execute_binop(Context *context, AST_Node *root) {
    assert(root->kind == AST_BINOP);
    Output res = alloc_output(VOID_TYPE);

    AST_NodeBinOp *binop = (void*)root;

    Value lhs = alloc_value(VOID_TYPE);
    if (!is_assignment(root)) {
        lhs = execute_expr(context, binop->lhs);
        if (has_errors(context)) {
            return res.val;
        }
    }
    
    Value rhs = execute_expr(context, binop->rhs);
    if (has_errors(context)) {
        return res.val;
    }
    
    switch (binop->op) {
        case BINOP_PLUS: {
            res = binary_plus(lhs, rhs);
        } break;

        case BINOP_MINUS: {
            res = binary_minus(lhs, rhs);
        } break;

        case BINOP_MUL: {
            res = binary_mul(lhs, rhs);
        } break;

        case BINOP_DIV: {
            res = binary_div(lhs, rhs);
        } break;

        case BINOP_MOD: {
            res = binary_mod(lhs, rhs);
        } break;

        case BINOP_POW: {
            res = binary_pow(lhs, rhs);
        } break;

        case BINOP_EQ: {
            if (binop->lhs->kind != AST_NAME) {
                append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                return res.val;
            }
            
            String_View name_sv = ((AST_NodeName*)binop->lhs)->name;

            Var *var = NULL;
            if (resolve_name(context, name_sv, &var)) {
                if (var->constant) {
                    append_error(context, ERROR_CANNOT_ASSIGN_TO_CONST);
                    return res.val;
                }
            }
            else {
                append_error(context, ERROR_NOT_DEFINED);
                return res.val;
            }
            
            var = alloc_var(sv_to_sb(name_sv), rhs, var->constant);
            hm_nput(context->scope.vars, name_sv.items, name_sv.count, var);

            res.val = rhs;
        } break;
            
        default: break;
    }
    
    return res.val;
}

Value execute_unop(Context *context, AST_Node *root) {
    assert(root->kind == AST_UNOP);
    Output output = alloc_output(VOID_TYPE);

    AST_NodeUnOp *unop = (void*)root;
    Value value = execute_expr(context, unop->expr);
    if (has_errors(context)) {
        return output.val;
    }
    
    switch (unop->op) {
        case UNOP_PLUS: {
            output = unary_plus(value);
            if (output.err != ERROR_NONE) {
                append_error(context, output.err);
            }
            
            return output.val;
        }
        case UNOP_MINUS: {
            output = unary_minus(value);
            if (output.err != ERROR_NONE) {
                append_error(context, output.err);
            }
            
            return output.val;
        }
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

Value execute_call(Context *context, AST_Node *node) {
    assert(node->kind == AST_CALL);

    Value value = alloc_value(VOID_TYPE);
    
    AST_NodeCall *call = (AST_NodeCall*)node;
    Func *func = NULL;
    if (!resolve_func(context, node, &func)) {
        da_append(context->errors, ERROR_NOT_DEFINED);
        return value;
    }

    return exec_func(context, func, call->args);
}

Value execute_block(Context *context, AST_Node *node) {
    assert(node->kind == AST_BLOCK);
    Value ret = alloc_value(VOID_TYPE);

    AST_NodeBlock *block = (void*)node;

    execute_nodes(context, block->nodes);
    if (has_errors(context)) {
        return ret;
    }
    
    if (block->ret_expr != NULL) {
        ret = execute(context, block->ret_expr);
    }

    return ret;
}

Value execute_expr(Context *context, AST_Node *expr) {
    Value val = alloc_value(VOID_TYPE);

    switch (expr->kind) {
        case AST_BINOP: {
            val = execute_binop(context, expr);
        } break;

        case AST_UNOP: {
            val = execute_unop(context, expr);
        } break;

        case AST_CALL: {
            val = execute_call(context, expr);
        } break;
            
        case AST_NAME: {
            val = get_node_value(context, expr);
        } break;
            
        case AST_LIT: {
            val = ((AST_NodeLit*)expr)->val;
        } break;

        case AST_BLOCK: {
            val = execute_block(context, expr);
        } break;
            
        case AST_ERROR: {
            da_append(context->errors, ((AST_NodeError*)expr)->err);
        } break;

        default: assert(0 && "UNREACHABLE");
    }

    return val;
}

Value execute(Context *context, AST_Node *node) {
    Value value = alloc_value(VOID_TYPE);
    
    switch (node->kind) {
        case AST_FUNC: {
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

void execute_nodes(Context *context, Nodes nodes) {
    for (size_t i = 0; i < nodes.count; ++i) {
        AST_Node *node = nodes.items[i];

        execute(context, node);
        if (has_errors(context)) {
            return;
        }
    }
}

void execute_program(Context *context, AST_NodeProgram *program) {
    execute_nodes(context, program->nodes);
    ast_free((void*)program);
}