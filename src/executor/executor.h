#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

#include "error.h"
#include "value.h"
#include "symbols/symbol.h"
#include "parser/ast.h"

typedef struct HashMap HashMap;
typedef struct {
    HashMap *symbols;
} Scope;

typedef struct TypeCache {
    HashMap *array_cache;
    HashMap *func_cache;
    HashMap *type_cache;
    HashMap *ref_cache;
} TypeCache;

typedef struct Context Context;
typedef struct Context {
    Context *global;
    Scope scope;
    Errors *errors;

    TypeCache type_cache;
} Context;

typedef enum Signal {
    SIGNAL_NONE,
    SIGNAL_CONTINUE,
    SIGNAL_BREAK,
    SIGNAL_RETURN,
} Signal;

typedef struct EvalResult {
    Signal sig;
    Value val;
} EvalResult;

void global_ctx_init(Context *ctx);
void ctx_init(Context *ctx);
void ctx_free(Context *ctx);

bool has_errors(Context *ctx);
void append_error(Context *ctx, RuntimeError err);
RuntimeError get_signal_error(Signal sig);

Value *alloc_value(Value value);
Value create_value(Type *type);
EvalResult create_result(Type *type);

Array *alloc_arr(Context *ctx, Type *el_type);

bool resolve_name_sv(Context *ctx, String_View name_sv, Var **var);
bool resolve_name_cstr(Context *ctx, char *name, Var **var);
bool get_func(Context *ctx, String_View name_sv, Func **func);

Member *get_member_from_node(Context *ctx, AST_Node *node, Value *base);

EvalResult execute_member_access_expr(Context *ctx, AST_Node *node);
Value execute_arr_expr(Context *ctx, AST_Node *node);
Value execute_call_expr(Context *ctx, AST_Node *node);
EvalResult execute_expr(Context *ctx, AST_Node *expr);
EvalResult execute_nodes(Context *ctx, AST_Nodes *nodes);
EvalResult execute(Context *ctx, AST_Node *node);
void execute_program(Context *ctx, AST_NodeProgram *program);

#endif //EXECUTOR_H
