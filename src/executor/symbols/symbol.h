#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

#include "nothing/nothing.h"
#include "executor/value.h"

typedef enum SymbolKind {
    SYMB_VAR,
    SYMB_FUNC,
    SYMB_TYPE,
} SymbolKind;

#define SYMBOL_FIELDS       \
    String_Builder *name;   \
    SymbolKind symb_kind;   \
    bool constant;          \
    bool is_static;

typedef struct Symbol {
    SYMBOL_FIELDS
} Symbol;

typedef struct Var {
    SYMBOL_FIELDS
    Value *val;
} Var;

#define FUNC_FIELDS \
    SYMBOL_FIELDS   \
    Type *type;     \
    Type *ret_type;  \
    FuncKind kind;  \
    Patterns args;      

typedef enum FuncKind {
    FUNC_BUILT_IN,
    FUNC_CUSTOM,
} FuncKind;

typedef struct Context Context;
typedef Value (*func_t)(Context *ctx, Context *fn_context);

typedef struct Func {
    FUNC_FIELDS
} Func;

typedef struct {
    FUNC_FIELDS
    func_t func;
} FuncBuiltIn;

typedef struct AST_Node AST_Node;
typedef struct {
    FUNC_FIELDS
    AST_Node *body;
    Value receiver;
    bool has_receiver;
} FuncCustom;

typedef enum TypeKind {
    TYPE_PRIMITIVE,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_FUNC,
    TYPE_TYPE,
    TYPE_REF,
} TypeKind;

typedef struct Type Type;
typedef struct Type {
    SYMBOL_FIELDS
    TypeKind kind;
    Type *el_type;
    Members *members;
    Func *func;
    size_t member_index;
    bool initialized;
} Type;

#endif //SYMBOL_H
