#ifndef TYPE_H
#define TYPE_H

#include "nothing/nothing.h"
#include "executor/symbols/symbol.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define ALIGNOF(T) alignof(T)
#else
    #define ALIGNOF(T) _Alignof(T)
#endif

#define PADDING(size, align) (align - (size % align)) % align
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

extern const Type int_symb;
extern const Type float_symb;
extern const Type bool_symb;
extern const Type char_symb;
extern const Type str_symb;
extern const Type any_symb;
extern const Type void_symb;
extern const Type variadic_symb;

extern const Type array_any_symb;

#define INT_TYPE (void*)&int_symb
#define FLOAT_TYPE (void*)&float_symb
#define BOOL_TYPE (void*)&bool_symb
#define CHAR_TYPE (void*)&char_symb
#define STR_TYPE (void*)&str_symb
#define ANY_TYPE (void*)&any_symb
#define VOID_TYPE (void*)&void_symb

#define ARRAY_ANY_TYPE (void*)&array_any_symb
#define VARIADIC_TYPE (void*)&variadic_symb

Value get_static_member_val(Member *member);
Value get_static_member_ref(Context *ctx, Member *member);
Value get_member_val(Struct strct, Member *member);
Value get_member_ref(Context *ctx, Struct strct, Member *member);
bool get_member(Type *type, String_View *name_sv, Member **member);
void assign_field(Context *ctx, Struct strct, Member *member, Value val);
Value copy_value(Value *value);

void alloc_type_value(Value *val, Type *type);

Array *alloc_array_value(Type *el_type);
Struct alloc_struct_value(Type *type);

Type *alloc_struct_type(String_Builder *name_sb, Members *members, bool initialized);
Type *alloc_enum_el_type(Context *ctx, Type *t, Type *enum_type);
Type *alloc_enum_type(String_Builder *name_sb, Members *members, bool initialized);
Type *alloc_type_type(Context *ctx, Type *t);
Type *alloc_ref_type(Context *ctx, Type *t);
Type *alloc_func_type(Context *ctx, Func *func);
Type *alloc_array_type(Context *ctx, Type *el_type);

bool compatible_types(Type *type_1, Type *type_2);
Value cast_value(Context *ctx, Value val, Type *type);

void format_str(String_Builder *sb, Context *ctx, String_View fmt_sv, Array *va_args);
void to_str(String_Builder *sb, Context *ctx, Value val, size_t depth);
bool to_bool(Context *ctx, Value val);

Value binary_plus(Context *ctx, Value lhs, Value rhs);
Value binary_minus(Context *ctx, Value lhs, Value rhs);
Value binary_mul(Context *ctx, Value lhs, Value rhs);
Value binary_div(Context *ctx, Value lhs, Value rhs);
Value binary_mod(Context *ctx, Value lhs, Value rhs);
Value binary_pow(Context *ctx, Value lhs, Value rhs);

Value binary_logic_and(Context *ctx, Value lhs, Value rhs);
Value binary_logic_or(Context *ctx, Value lhs, Value rhs);

Value binary_bitwise_and(Context *ctx, Value lhs, Value rhs);
Value binary_bitwise_or(Context *ctx, Value lhs, Value rhs);

Value binary_gt(Context *ctx, Value lhs, Value rhs);
Value binary_lt(Context *ctx, Value lhs, Value rhs);
Value binary_gteq(Context *ctx, Value lhs, Value rhs);
Value binary_lteq(Context *ctx, Value lhs, Value rhs);

Value binary_eq(Context *ctx, Value lhs, Value rhs);
Value binary_neq(Context *ctx, Value lhs, Value rhs);

Value unary_plus(Context *ctx, Value x);
Value unary_minus(Context *ctx, Value x);
Value unary_increment(Context *ctx, Value x);
Value unary_decrement(Context *ctx, Value x);
Value unary_not(Context *ctx, Value x);

#endif //TYPE_H
