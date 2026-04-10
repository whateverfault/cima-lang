#ifndef TYPE_H
#define TYPE_H

#include "nothing/nothing.h"

#define INT_CTYPE int
#define FLOAT_CTYPE double
#define BOOL_CTYPE bool
#define CHAR_CTYPE char

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_CHAR,
    TYPE_ARRAY,
    TYPE_PTR,
    TYPE_VOID,
    TYPE_ANY,
    TYPE_VARIADIC,
} TypeTag;

#define VALUE_TYPE_FIELDS TypeTag tag;

typedef struct {
    VALUE_TYPE_FIELDS
} ValueType;

typedef struct {
    VALUE_TYPE_FIELDS
    ValueType *el_type;
} ValueTypeArray;

typedef struct {
    VALUE_TYPE_FIELDS
    ValueType *value_type;
} ValueTypePtr;

static const ValueType int_type = (ValueType){
    .tag = TYPE_INT,
};

static const ValueType float_type = (ValueType){
    .tag = TYPE_FLOAT,
};

static const ValueType bool_type = (ValueType){
    .tag = TYPE_BOOL,
};

static const ValueType str_type = (ValueType){
    .tag = TYPE_STR,
};

static const ValueType char_type = (ValueType){
    .tag = TYPE_CHAR,
};

static const ValueType any_type = (ValueType){
    .tag = TYPE_ANY,
};

static const ValueType void_type = (ValueType){
    .tag = TYPE_VOID,
};

static const ValueType variadic_type = (ValueType){
    .tag = TYPE_VARIADIC,
};

#define INT_TYPE (void*)&int_type
#define FLOAT_TYPE (void*)&float_type
#define BOOL_TYPE (void*)&bool_type
#define STR_TYPE (void*)&str_type
#define CHAR_TYPE (void*)&char_type
#define ANY_TYPE (void*)&any_type
#define VOID_TYPE (void*)&void_type
#define VARIADIC_TYPE (void*)&variadic_type

static const ValueTypeArray array_any_type = (ValueTypeArray){
    .tag = TYPE_ARRAY,
    .el_type = ANY_TYPE,
};

static const ValueTypeArray array_va_type = (ValueTypeArray){
    .tag = TYPE_ARRAY,
    .el_type = VARIADIC_TYPE,
};

#define ARRAY_ANY_TYPE (void*)&array_any_type
#define ARRAY_VA_TYPE (void*)&array_va_type

typedef struct Variadic Variadic;
typedef struct Context Context;
typedef struct Value Value;

typedef struct ArrayElements {
    Value *items;
    size_t count;
    size_t capacity;
} ArrayElements;

typedef struct Array {
    ArrayElements els;
    ValueType *el_type;
} Array;

void format_str(String_Builder *sb, Context *context, String_View fmt_sv, Array *va_args);
void to_str(String_Builder *sb, Context *context, Value val);

#endif //TYPE_H
