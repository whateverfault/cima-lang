#ifndef VALUE_H
#define VALUE_H

#define INT_CTYPE int
#define FLOAT_CTYPE double
#define BOOL_CTYPE bool
#define CHAR_CTYPE char
#define STR_CTYPE String_Builder
#define PTR_CTYPE void*

#define INT_CTYPE_MIN INT_MIN
#define INT_CTYPE_MAX INT_MAX

#define VALUE_OFFSET 

typedef struct Type Type;
typedef struct Struct {
    Type *type;
    HashMap *members;
} Struct;

typedef struct Value {
    union {
        INT_CTYPE as_int;
        FLOAT_CTYPE as_float;
        Struct as_struct;
        void *as_ptr;
    };
    
    Type *type;
} Value;

typedef struct Array {
    Value *items;
    size_t count;
    size_t capacity;
    
    Type *el_type;
} Array;

typedef struct AST_Node AST_Node;
typedef struct Pattern {
    String_Builder *name;
    Type *type;
    AST_Node *initializer;
    bool constant;
} Pattern;

typedef struct Patterns {
    Pattern *items;
    size_t count;
    size_t capacity;
} Patterns;

typedef struct Func Func;
typedef struct Member {
    Type *type;
    bool is_const;
    bool is_static;
    String_Builder *name;

    enum {
        MEMBER_FIELD,
        MEMBER_METHOD,
    } kind;
    
    union {
        struct {
            union {
                AST_Node *initializer;
                Value *static_initializer;
            };
        } field;

        struct {
            Func *func;
        } method;
    };
} Member;

typedef struct Members {
    Member **items;
    size_t count;
    size_t capacity;
} Members;

typedef struct Funcs {
    Func **items;
    size_t count;
    size_t capacity;
} Funcs;

#endif //VALUE_H
