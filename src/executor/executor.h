#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

#include "parser/parser.h"

typedef struct {
    ErrorKind *items;
    size_t count;
    size_t capacity;
} Errors;

typedef struct {
    char *name;
    Value val;
    bool constant;
} Var;

typedef struct Func Func;
#define FUNC_FIELDS \
    FuncKind kind;  \
    char *name;     \
    Patterns args;  \

typedef struct HashMap HashMap;
typedef struct {
    HashMap *vars;
    HashMap *funcs;
} Scope;

typedef struct Context{
    Scope scope;
    Errors *errors;
} Context;

typedef Value (*func_t)(Context *context, Context *fn_context);

typedef enum {
    FUNC_BUILT_IN,
    FUNC_CUSTOM,
} FuncKind;

typedef struct Func {
    FUNC_FIELDS
} Func;

typedef struct {
    FUNC_FIELDS
    func_t func;
} FuncBuiltIn;

typedef struct {
    FUNC_FIELDS
    AST_Node *body;
    bool constant;
} FuncCustom;

void context_init(Context *context);
void context_free(Context *context);

bool has_errors(Context *context);
void append_error(Context *context, ErrorKind err);

bool resolve_name(Context *context, char *name, Var **var);
Value get_node_value(Context *context, AST_Node *node);

Value execute_expr(Context *context, AST_Node *expr);
void execute_nodes(Context *context, Nodes nodes);
Value execute(Context *context, AST_Node *node);
void execute_program(Context *context, AST_NodeProgram *program);

#endif //EXECUTOR_H
