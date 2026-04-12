#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

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

typedef struct {
    ErrorKind *items;
    size_t count;
    size_t capacity;
} Errors;

typedef struct Var{
    String_Builder *name;
    Value val;
    bool constant;
} Var;

typedef struct Func Func;
#define FUNC_FIELDS          \
    FuncKind kind;           \
    String_Builder *name;    \
    Patterns args;           \
    bool constant;

typedef struct HashMap HashMap;
typedef struct {
    HashMap *vars;
    HashMap *funcs;
} Scope;

typedef struct Context{
    Context *global;
    Scope scope;
    Errors *errors;
} Context;

void context_init(Context *context);
void context_free(Context *context);

bool has_errors(Context *context);
void append_error(Context *context, ErrorKind err);
ErrorKind get_signal_error(Signal sig);

EvalResult create_result(ValueType *type);

Array *alloc_arr(ValueType *el_type);

bool resolve_name_sv(Context *context, String_View *name_sv, Var **var);
bool resolve_name_cstr(Context *context, char *name, Var **var);
bool get_func(Context *context, String_View *name_sv, Func **func);

Value execute_arr_expr(Context *context, AST_Node *node);
Value execute_call_expr(Context *context, AST_Node *node);
EvalResult execute_expr(Context *context, AST_Node *expr);
EvalResult execute_nodes(Context *context, Nodes *nodes);
EvalResult execute(Context *context, AST_Node *node);
void execute_program(Context *context, AST_NodeProgram *program);

#endif //EXECUTOR_H
