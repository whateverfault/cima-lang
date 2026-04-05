#ifndef TYPE_H
#define TYPE_H

#include "nothing/nothing.h"

#define INT_CTYPE int
#define FLOAT_CTYPE double
#define BOOL_CTYPE bool
#define CHAR_CTYPE char

typedef enum {
    TYPE_INT = 0,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_ANY,
} BasicType;

typedef enum {
    TYPE_BASIC = TYPE_ANY + 1,
    TYPE_ARRAY,
    TYPE_PTR,
    TYPE_VARIADIC,
} TypeKind;

#define VALUE_TYPE_FIELDS TypeKind kind;

typedef struct {
    VALUE_TYPE_FIELDS
} ValueType;

typedef struct {
    VALUE_TYPE_FIELDS
    BasicType type;
} ValueTypeBasic;

typedef struct {
    VALUE_TYPE_FIELDS
    ValueType *el_type;
} ValueTypeArray;

typedef struct {
    VALUE_TYPE_FIELDS
    ValueType *value_type;
} ValueTypePtr;

static const ValueTypeBasic int_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_INT,
};

static const ValueTypeBasic float_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_FLOAT,
};

static const ValueTypeBasic bool_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_BOOL,
};

static const ValueTypeBasic str_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_STR,
};

static const ValueTypeBasic char_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_CHAR,
};

static const ValueTypeBasic any_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_ANY,
};

static const ValueTypeBasic void_type = (ValueTypeBasic){
    .kind = TYPE_BASIC,
    .type = TYPE_VOID,
};

static const ValueTypeBasic variadic_type = (ValueTypeBasic){
    .kind = TYPE_VARIADIC,
    .type = TYPE_ANY,
};

#define INT_TYPE (void*)&int_type
#define FLOAT_TYPE (void*)&float_type
#define BOOL_TYPE (void*)&bool_type
#define STR_TYPE (void*)&str_type
#define CHAR_TYPE (void*)&char_type
#define ANY_TYPE (void*)&any_type
#define VOID_TYPE (void*)&void_type
#define VARIADIC_TYPE (void*)&variadic_type

typedef struct Variadic Variadic;
typedef struct Context Context;
typedef struct Value Value;

void format_str(String_Builder *sb, Context *context, String_View fmt_sv, Variadic *va_args);
void to_str(String_Builder *sb, Context *context, Value val);

#endif //TYPE_H
