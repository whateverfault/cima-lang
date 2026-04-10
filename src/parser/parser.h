#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "types/type.h"

#define AST_FIELDS AST_Kind kind;

typedef enum {
    ERROR_NONE,
    
    ERROR_WRONG_NUMBER_FORMAT,
    ERROR_VALUE_OUT_OF_RANGE,
    ERROR_DIV_BY_ZERO,
    ERROR_UNMATCHED_PAREN,
    ERROR_UNEXPECTED_TOKEN,
    ERROR_NOT_DEFINED,
    ERROR_CANNOT_ASSIGN_TO_CONST,
    ERROR_CANNOT_REASSIGN_CONST,
    ERROR_INDEX_OUT_OF_BOUNDS,

    ERROR_TOO_FEW_ARGS,
    ERROR_TOO_MANY_ARGS,
    ERROR_RECURSION_LIMIT_EXCEEDED,
    ERROR_UNEXPECTED_NAMED_ARG,
    ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT,
    ERROR_ARGS_AFTER_VA_ARG,

    ERROR_INCOMPATIBLE_TYPES,
    ERROR_EMPTY_CHAR_LIT,
    ERROR_MULTI_CHARACTER_CHAR_LIT,

    ERROR_CLOSED_STDIN,
} ErrorKind;

typedef enum {
    AST_BINOP,
    AST_UNOP,
    AST_LIT,
    AST_NAME,
    AST_CALL,
    AST_INDEX,
    AST_FN,
    AST_LET,
    AST_ARR,
    AST_BLOCK,
    AST_IF,
    AST_PROGRAM,
    AST_ERROR,
} AST_Kind;

typedef enum {
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_POW,
    BINOP_GT,
    BINOP_LT,
    BINOP_GTEQ,
    BINOP_LTEQ,
    BINOP_EQ,
    BINOP_NEQ,

    BINOP_ASIGN,
} BinaryOp;

typedef enum {
    UNOP_NOT,
    UNOP_PLUS,
    UNOP_MINUS,
} UnaryOp;

typedef struct AST_Node AST_Node;
typedef struct AST_NodeName AST_NodeName;

typedef struct Value {
    ValueType *type;

    union {
        INT_CTYPE as_int;
        FLOAT_CTYPE as_float;
        void *as_ptr;
    };
} Value;

typedef struct {
    AST_Node *node;
    bool has_name;
    String_View name;
} Arg;

typedef struct {
    Arg *items;
    size_t count;
    size_t capacity;
} Args;

typedef struct {
    String_Builder *name;
    ValueType *type;
} Pattern;

typedef struct {
    Pattern *items;
    size_t count;
    size_t capacity;
} Patterns;

typedef struct {
    AST_Node **items;
    size_t count;
    size_t capacity;
} Nodes;

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

typedef struct {
    AST_FIELDS
    Value val;
} AST_NodeLit;

typedef struct AST_NodeName {
    AST_FIELDS
    String_View name;
} AST_NodeName;

typedef struct {
    AST_FIELDS
    String_View name;
    Args args;
} AST_NodeCall;

typedef struct {
    AST_FIELDS
    AST_Node *node;
    AST_Node *index;
} AST_NodeIndex;

typedef struct {
    AST_FIELDS
    String_View name;
    Patterns args;
    AST_Node *body;
    ValueType *ret_type;
    bool constant;
} AST_NodeFunc;

typedef struct {
    AST_FIELDS
    String_View name;
    ValueType *type;
    AST_Node *initializer;
    bool has_initializer;
    bool constant;
} AST_NodeLet;

typedef struct {
    AST_FIELDS
    ErrorKind err;
} AST_NodeError;

typedef struct {
    AST_FIELDS
    Nodes nodes;
} AST_NodeProgram;

typedef struct {
    AST_FIELDS
    Nodes nodes;
} AST_NodeArray;

typedef struct {
    AST_FIELDS
    Nodes nodes;
    AST_Node *ret_expr;
} AST_NodeBlock;

typedef struct {
    AST_FIELDS
    AST_Node *condition;
    AST_Node *body;
    Nodes elif_branches;
    AST_Node *else_branch;
} AST_NodeBranch;

typedef struct {
    bool right_associative;
    size_t val;
} Precedence;

void patterns_free(Patterns patterns);
void args_free(Args args);
void ast_free(AST_Node *root);

ValueTypeArray *alloc_arr_type(ValueType *el_type);
Value create_value(ValueType *type);

AST_NodeBinOp *alloc_binop_expr(AST_Node *lhs, BinaryOp op, AST_Node *rhs);
AST_NodeUnOp *alloc_unop_expr(AST_Node *expr, UnaryOp op);
AST_NodeLit *alloc_lit_expr(Value val);
AST_NodeName *alloc_name_expr(String_View sb);
AST_NodeError *alloc_error(ErrorKind err);
AST_NodeArray *alloc_arr_node(Nodes nodes);

ErrorKind parse_type(Lexer *l, ValueType **type);

AST_Node *parse_prefix_expr(Lexer *l);
AST_Node *parse_expr(Lexer *l, size_t min_prec);
AST_Node *parse(Lexer *l);
AST_Node *parse_item(Lexer *l);
Nodes parse_nodes(Lexer *l);
AST_NodeProgram *parse_program(Lexer *l);

#endif //PARSER_H
