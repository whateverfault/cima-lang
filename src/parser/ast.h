#ifndef AST_H
#define AST_H

#include "nothing/nothing.h"

typedef enum {
    AST_BINOP,
    AST_UNOP,
    AST_LIT,
    AST_NAME,
    AST_TYPE,
    AST_CALL,
    AST_INDEX,
    AST_STRUCT,
    AST_MEMBER_ACCESS,
    AST_FUNC_DECL,
    AST_STRUCT_DECL,
    AST_LET,
    AST_FOR,
    AST_ARR,
    AST_BLOCK,
    AST_IF,
    AST_CONTINUE,
    AST_BREAK,
    AST_RETURN,
    AST_PROGRAM,
} AST_Kind;

typedef enum {
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_POW,
    BINOP_ASSIGN,
    BINOP_LOGIC_AND,
    BINOP_BIT_AND,
    BINOP_LOGIC_OR,
    BINOP_BIT_OR,
    BINOP_GT,
    BINOP_LT,
    BINOP_GTEQ,
    BINOP_LTEQ,
    BINOP_EQ,
    BINOP_NEQ,

} BinaryOp;

typedef enum {
    UNOP_INCREMENT,
    UNOP_DECREMENT,
    UNOP_NOT,
    UNOP_PLUS,
    UNOP_MINUS,
} UnaryOp;

#define AST_FIELDS AST_Kind kind;

typedef struct AST_Node AST_Node;
typedef struct AST_NodeName AST_NodeName;
typedef struct AST_NodeFnDecl AST_NodeFuncDecl;

typedef struct {
    AST_Node *node;
    bool has_name;
    String_View name;
} AST_Arg;

typedef struct {
    AST_Arg *items;
    size_t count;
    size_t capacity;
} AST_Args;

typedef struct AST_Nodes {
    AST_Node **items;
    size_t count;
    size_t capacity;
} AST_Nodes;

typedef struct AST_Type AST_Type;
typedef struct AST_Pattern {
    String_View name;
    AST_Type *type;
    AST_Node *initializer;
    bool is_const;
    bool is_static;
} AST_Pattern;

typedef struct AST_Patterns {
    AST_Pattern *items;
    size_t count;
    size_t capacity;
} AST_Patterns;

typedef struct AST_Initializer {
    String_View name;
    bool has_name;
    AST_Node *initializer;
} AST_Initializer;

typedef struct AST_Initializers {
    AST_Initializer *items;
    size_t count;
    size_t capacity;
} AST_InitializerList;

typedef struct AST_Node {
    AST_FIELDS
} AST_Node;

typedef struct {
    AST_FIELDS
    BinaryOp op;
    AST_Node *lhs;
    AST_Node *rhs;
} AST_NodeBinOp;

typedef struct {
    AST_FIELDS
    UnaryOp op;
    AST_Node *expr;
} AST_NodeUnOp;

typedef enum AST_LitType {
    AST_TYPE_INT,
    AST_TYPE_FLOAT,
    AST_TYPE_BOOL,
    AST_TYPE_CHAR,
    AST_TYPE_STR,
} AST_LitType;

typedef struct AST_Value {
    AST_LitType type;
    String_View view;
    union {
        int as_char;
        bool as_bool;
    };
} AST_Value;

typedef struct {
    AST_FIELDS
    AST_Value val;
} AST_NodeLit;

typedef struct AST_NodeName {
    AST_FIELDS
    String_View name;
} AST_NodeName;

typedef struct AST_Type {
    AST_FIELDS
    String_View name;
    AST_Type *el_type;
    bool provided_name;
    bool is_array;
} AST_Type;

static const AST_Type ast_any_type = (AST_Type){
    .kind = AST_TYPE,
    .provided_name = false,
};

typedef struct {
    AST_FIELDS
    AST_Node *to_call;
    AST_Args args;
} AST_NodeCall;

typedef struct {
    AST_FIELDS
    AST_Node *node;
    AST_Node *index;
} AST_NodeIndex;

typedef struct {
    AST_FIELDS
    AST_Node *node;
    AST_InitializerList initializers;
} AST_NodeInit;

typedef struct {
    AST_FIELDS
    AST_Node *base;
    AST_Node *member;
} AST_NodeMemberAccess;

typedef struct AST_NodeFnDecl {
    AST_FIELDS
    String_View name;
    AST_Patterns args;
    AST_Node *body;
    AST_Type *ret_type;
    bool is_const;
    bool is_static;
} AST_NodeFuncDecl;

typedef struct {
    AST_FIELDS
    String_View name;
    AST_Patterns fields;
    AST_Nodes methods;
    bool constant;
} AST_NodeStructDecl;

typedef struct {
    AST_FIELDS
    String_View name;
    AST_Type *type;
    AST_Node *initializer;
    bool constant;
} AST_NodeLetStmt;

typedef struct {
    AST_FIELDS
    AST_Node *initializer;
    AST_Node *condition;
    AST_Node *next;
    AST_Node *body;
} AST_NodeForStmt;

typedef struct {
    AST_FIELDS
    AST_Nodes nodes;
} AST_NodeProgram;

typedef struct {
    AST_FIELDS
    AST_Nodes nodes;
} AST_NodeArray;

typedef struct {
    AST_FIELDS
    AST_Nodes nodes;
    AST_Node *ret_expr;
} AST_NodeBlock;

typedef struct {
    AST_FIELDS
    AST_Node *condition;
    AST_Node *body;
    AST_Nodes elif_branches;
    AST_Node *else_branch;
} AST_NodeBranch;

typedef struct {
    AST_FIELDS
} AST_NodeContinue;

typedef struct {
    AST_FIELDS
} AST_NodeBreak;

typedef struct {
    AST_FIELDS
    AST_Node *node;
} AST_NodeReturn;

#endif //AST_H
