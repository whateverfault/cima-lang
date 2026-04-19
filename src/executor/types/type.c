#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "type.h"

#include <math.h>
#include "executor/executor.h"
#include "parser/parser.h"
#include "parser/parser_error.h"

static String_Builder int_sb = CSTR_TO_SB("int");
static String_Builder float_sb = CSTR_TO_SB("float");
static String_Builder bool_sb = CSTR_TO_SB("bool");
static String_Builder char_sb = CSTR_TO_SB("char");
static String_Builder str_sb = CSTR_TO_SB("str");
static String_Builder any_sb = CSTR_TO_SB("any");
static String_Builder void_sb = CSTR_TO_SB("void");
static String_Builder variadic_sb = CSTR_TO_SB("...");

const Type int_symb = (Type){
    .name = &int_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(INT_CTYPE),
    .alignment = ALIGNOF(INT_CTYPE),
    .members = {0},
    .constant = true,
};

const Type float_symb = (Type){
    .name = &float_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(FLOAT_CTYPE),
    .alignment = ALIGNOF(FLOAT_CTYPE),
    .members = {0},
    .constant = true,
};

const Type bool_symb = (Type){
    .name = &bool_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(BOOL_CTYPE),
    .alignment = ALIGNOF(BOOL_CTYPE),
    .members = {0},
    .constant = true,
};

const Type char_symb = (Type){
    .name = &char_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(CHAR_CTYPE),
    .alignment = ALIGNOF(CHAR_CTYPE),
    .members = {0},
    .constant = true,
};

const Type str_symb = (Type){
    .name = &str_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(STR_CTYPE),
    .alignment = ALIGNOF(STR_CTYPE),
    .members = {0},
    .constant = true,
};

const Type any_symb = (Type){
    .name = &any_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(ANY_CTYPE),
    .alignment = ALIGNOF(ANY_CTYPE),
    .members = {0},
    .constant = true,
};

const Type void_symb = (Type){
    .name = &void_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = 0,
    .alignment = 0,
    .members = {0},
    .constant = true,
};

const Type variadic_symb = (Type){
    .name = &variadic_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_ARRAY,
    .size = sizeof(Array),
    .alignment = ALIGNOF(Array),
    .el_type = NULL,
    .members = {0},
    .constant = true,
};

const Type array_any_symb = (Type){
    .name = NULL,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_ARRAY,
    .size = sizeof(Array),
    .alignment = ALIGNOF(Array),
    .el_type = ANY_TYPE,
    .members = {0},
    .constant = true,
};

const Type array_va_symb = (Type){
    .name = NULL,
    .symb_kind = SYMB_TYPE,
    .size = sizeof(Array),
    .alignment = ALIGNOF(Array),
    .el_type = VARIADIC_TYPE,
    .members = {0},
    .constant = true,
};

#define IS_INTLIKE(t) ((t)==INT_TYPE||(t)==BOOL_TYPE||(t)==CHAR_TYPE)
#define FIELD_ADDR(struct_addr, offset) ((char*)struct_addr + offset)

size_t hash_type(Type *t);
size_t hash_members(Members members) {
    size_t h = 0;

    for (size_t i = 0; i < members.count; ++i) {
        Member member = members.items[i];
        h = hash_combine(h, hash_type(member.type));
        
        if (member.name != NULL && member.name->count > 0) {
            h = hash_combine(h, hash_nkey((unsigned char*)member.name->items, member.name->count));
        }
    }

    return h;
}

size_t hash_type(Type *t) {
    size_t h = 0;

    h = hash_combine(h, t->kind);
    h = hash_combine(h, hash_members(t->members));
    
    if (t->el_type != NULL) {
        h = hash_combine(h, (size_t)t->el_type);
    }

    return h;
}

size_t hash_patterns(Patterns patterns) {
    size_t h = 0;

    for (size_t i = 0; i < patterns.count; ++i) {
        Pattern pattern = patterns.items[i];
        h = hash_combine(h, hash_type(pattern.type));
        
        if (pattern.name->count > 0) {
            h = hash_combine(h, hash_nkey((unsigned char*)pattern.name->items, pattern.name->count));
        }
    }

    return h;
}

size_t hash_func(Func *f) {
    size_t h = 0;

    h = hash_combine(h, f->kind);
    if (f->name->count > 0) {
        h = hash_combine(h, hash_nkey((unsigned char*)f->name->items, f->name->count));
    }
    
    h = hash_combine(h, hash_patterns(f->args));
    return h;
}

bool get_type_field(Type *type, String_View name, Member *field) {
    for (size_t i = 0; i < type->members.count; ++i) {
        if (sv_cmp_sb(&name, type->members.items[i].name)) {
            *field = type->members.items[i];
            return true;
        }
    }

    return false;
}

void store_value(void *dst, Type *type, Value val, Context *ctx) {
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            if (IS_INTLIKE(type)) {
                memcpy(dst, &val.as_int, sizeof(INT_CTYPE));
            }
            else if (type == FLOAT_TYPE) {
                memcpy(dst, &val.as_float, sizeof(FLOAT_CTYPE));
            }
            else if (type == STR_TYPE || type == VOID_TYPE) {
                memcpy(dst, &val.as_ptr, sizeof(void*));
            }
            else {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            }
        } break;

        case TYPE_STRUCT:
        case TYPE_ARRAY: {
            memcpy(dst, &val.as_ptr, sizeof(void*));
        } break;

        default: append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    }
}

void load_value(Value *out, void *src, Type *type, Context *ctx) {
    out->type = type;

    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            if (IS_INTLIKE(type)) {
                memcpy(&out->as_int, src, sizeof(INT_CTYPE));
            }
            else if (type == FLOAT_TYPE) {
                memcpy(&out->as_float, src, sizeof(FLOAT_CTYPE));
            }
            else if (type == BOOL_TYPE) {
                BOOL_CTYPE tmp;
                memcpy(&tmp, src, sizeof(tmp));
                out->as_int = tmp;
            }
            else if (type == CHAR_TYPE) {
                CHAR_CTYPE tmp;
                memcpy(&tmp, src, sizeof(tmp));
                out->as_int = tmp;
            }
            else if (type == STR_TYPE || type == VOID_TYPE) {
                memcpy(&out->as_ptr, src, sizeof(void*));
            }
            else {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            }
        } break;

        case TYPE_STRUCT:
        case TYPE_ARRAY: {
            memcpy(&out->as_ptr, src, sizeof(void*));
        } break;

        default: append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    }
}

void *alloc_type_value(Context *ctx, Type *type) {
    void *ptr = calloc(1, type->size);
    assert(ptr != NULL && "Memory allocation failed");

    for (size_t i = 0; i < type->members.count; ++i) {
        Member member = type->members.items[i];
        if (member.kind != MEMBER_FIELD) {
            continue;
        }

        if (member.type->kind == TYPE_STRUCT) {
            Value val = create_value(member.type);
            val.as_ptr = alloc_type_value(ctx, member.type);
            
            assign_field(ctx, ptr, member, val);
        }
    }
    
    return ptr;
}

void *alloc_type_value_from_value(Context *ctx, Value value) {
    void *ptr = calloc(1, value.type->size);
    assert(ptr != NULL && "Memory allocation failed");

    for (size_t i = 0; i < value.type->members.count; ++i) {
        Member member = value.type->members.items[i];
        if (member.kind != MEMBER_FIELD) {
            continue;
        }

        Value val = create_value(member.type);

        if (member.type->kind == TYPE_PRIMITIVE) {
            val = value;
        }
        else if (member.type->kind == TYPE_STRUCT || member.type->kind == TYPE_ARRAY) {
            val.as_ptr = alloc_type_value_from_value(ctx, value);
        }
        
        assign_field(ctx, ptr, member, val);
    }
    
    return ptr;
}

bool get_member(Type *type, String_View *name_sv, Member *member) {
    for (size_t i = 0; i < type->members.count; ++i) {
        Member m = type->members.items[i];

        if (m.name != NULL && sv_cmp_sb(name_sv, m.name)) {
            *member = m;
            return true;
        }
    }

    return false;
}

Value get_static_member_val(Member member) {
    Value member_val = create_value(member.type);

    if (member.kind == MEMBER_METHOD) {
        member_val.as_ptr = member.method.func;
        member_val.type = member.method.func->type;
        return member_val;
    }

    member_val = member.field.static_initializer;
    return member_val;
}

Value get_member_val(Context *ctx, void *struct_ptr, Member member) {
    Value member_val = create_value(member.type);

    if (member.kind == MEMBER_METHOD) {
        member_val.as_ptr = member.method.func;
        member_val.type = member.method.func->type;
        return member_val;
    }

    if (struct_ptr == NULL) {
        append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        return member_val;
    }

    void *addr = FIELD_ADDR(struct_ptr, member.field.offset);
    load_value(&member_val, addr, member.type, ctx);
    return member_val;
}

void assign_field(Context *ctx, void* struct_ptr, Member member, Value val) {
    if (member.kind != MEMBER_FIELD) {
        return;
    }
    
    void *field_ptr = (char*)struct_ptr + member.field.offset;
    Value value = cast_value(ctx, val, member.type);
    if (has_errors(ctx)) {
        return;
    }
    
    store_value(field_ptr, member.type, value, ctx);
}

size_t get_struct_fields_size(Members fields) {
    size_t size = 0;
    size_t max_align = 1;
    
    for (size_t i = 0; i < fields.count; ++i) {
        size_t field_size  = fields.items[i].type->size;
        size_t field_align = fields.items[i].type->alignment;
        
        if (field_align > max_align) {
            max_align = field_align;
        }
        
        size_t padding = PADDING(size, field_align);
        size += padding;
        
        size += field_size;
    }
    
    size += PADDING(size, max_align);
    return size;
}

size_t get_fields_alignment(Members fields) {
    size_t max_align = 1;

    for (size_t i = 0; i < fields.count; ++i) {
        size_t field_align = fields.items[i].type->alignment;
        if (field_align > max_align) {
            max_align = field_align;
        }
    }

    return max_align;
}

Type *alloc_struct_type(String_Builder *name_sb, Members fields, Type *el_type, bool constant) {
    Type *type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");

    size_t type_size = get_struct_fields_size(fields);
    size_t type_alignment = get_fields_alignment(fields);

    *type = (Type){
        .name = name_sb,
        .symb_kind = SYMB_TYPE,
        .constant = constant,
        .kind = TYPE_STRUCT,
        .size = type_size,
        .alignment = type_alignment,
        .el_type = el_type,
        .members = fields,
    };

    return type;
}

Type *alloc_type_type(Context *ctx, Type *t) {
    Type *type = hm_get_hashed(ctx->type_cache, hash_type(t));
    if (type != NULL) {
        return type;
    }
    
    type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");
    
    *type = (Type){
        .name = NULL,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_TYPE,
        .size = sizeof(Func),
        .alignment = ALIGNOF(Func),
        .el_type = t,
        .members = (Members){0},
    };

    return type;
}

Type *alloc_func_type(Context *ctx, Func *func) {
    Type *type = hm_get_hashed(ctx->type_cache, hash_func(func));
    if (type != NULL) {
        return type;
    }
    
    type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");
    
    *type = (Type){
        .name = NULL,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_FUNC,
        .size = sizeof(Func),
        .alignment = ALIGNOF(Func),
        .el_type = NULL,
        .members = (Members){0},
        .func = func,
    };

    return type;
}

Type *alloc_array_type(Context *ctx, Type *el_type) {
    Type *type = hm_get_hashed(ctx->type_cache, hash_type(el_type));
    if (type != NULL) {
        return type;
    }
        
    type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");
    
    *type = (Type){
        .name = NULL,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_ARRAY,
        .size = sizeof(Array),
        .alignment = ALIGNOF(Array),
        .el_type = el_type,
        .members = (Members){0},
    };

    return type;
}

void format_str(String_Builder *sb, Context *ctx, String_View fmt_sv, Array *va_args) {
    Lexer l = {
        .source = fmt_sv,
        .skipped = {0},
        .pos = 0,
    };

    lexer_init(&l);
    size_t va_arg_pos = 0;

    while (l.cur.kind != TOKEN_EOF) {
        sb_append_sv(sb, &l.skipped);
        
        if (l.cur.kind != TOKEN_LBRACE) {
            if (l.cur.kind == TOKEN_CHAR) {
                sb_appendc(sb, '\'');
            }
            else if (l.cur.kind == TOKEN_STR) {
                sb_appendc(sb, '\"');
            }
            
            sb_append_sv(sb, &l.cur.val);

            if (l.cur.kind == TOKEN_CHAR) {
                sb_appendc(sb, '\'');
            }
            else if (l.cur.kind == TOKEN_STR) {
                sb_appendc(sb, '\"');
            }
            
            lexer_next(&l);
            continue;
        }
        
        lexer_next(&l);
        
        if (l.cur.kind == TOKEN_RBRACE) {
            if (va_args == NULL || va_arg_pos >= va_args->count) {
                append_error(ctx, ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT);
                return;
            }
            
            to_str(sb, ctx, va_args->items[va_arg_pos]);
            ++va_arg_pos;
        }
        else {
            AST_Node *expr;
            ParserError err = parse(&l, &expr);
            if (err != PERROR_NONE) {
                append_error(ctx, ERROR_WRONG_FORMAT_STR);
                return;
            }
            
            EvalResult result = execute_expr(ctx, expr);
            append_error(ctx, get_signal_error(result.sig));
            if (has_errors(ctx)) {
                ast_free(expr);
                return;
            }

            if (result.val.type == STR_TYPE) {
                String_View sv = {0};
                sv_from_sb(&sv, result.val.as_ptr);
                format_str(sb, ctx, sv, va_args);
                if (has_errors(ctx)) {
                    ast_free(expr);
                    return;
                }
            }
            else {
                to_str(sb, ctx, result.val);
                if (has_errors(ctx)) {
                    ast_free(expr);
                    return;
                }
            }
            ast_free(expr);
        }

        if (l.cur.kind != TOKEN_RBRACE) {
            append_error(ctx, ERROR_WRONG_FORMAT_STR);
            return;
        }
        
        lexer_next(&l);
    }

    if (va_args != NULL && va_arg_pos != va_args->count) {
        append_error(ctx, ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT);
        return;
    }
    
    sb_append_sv(sb, &l.skipped);
}

// TODO: Force types to have to_str method (Implement traits)
// TODO: Implement any type

void to_str(String_Builder *sb, Context *ctx, Value val) {
    switch (val.type->kind) {
        case TYPE_PRIMITIVE: {
            if (val.type == INT_TYPE) {
                sb_appendf(sb, "%d", val.as_int);
            }
            else if (val.type == FLOAT_TYPE) {
                sb_appendf(sb, "%f", val.as_float);
            }
            else if (val.type == BOOL_TYPE) {
                sb_appendf(sb, "%s", val.as_int? "true" : "false");
            }
            else if (val.type == CHAR_TYPE) {
                sb_appendf(sb, "%c", (char)val.as_int);
            }
            else if (val.type == STR_TYPE) {
                sb_append_sb(sb, val.as_ptr);
            }
            else if (val.type == VOID_TYPE) {
                sb_appendf(sb, "()");
            }
            else {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            }
        } break;

        case TYPE_ARRAY: {
            sb_appendc(sb, '[');

            Array *arr = (void*)val.as_ptr;

            if (arr != NULL) {
                for (size_t i = 0; i < arr->count; ++i) {
                    to_str(sb, ctx, arr->items[i]);

                    if (i < arr->count - 1) {
                        sb_appendf(sb, ", ");
                    }
                }
            }
            
            sb_appendc(sb, ']');
        } break;

        case TYPE_STRUCT: {
            sb_appendc(sb, '{');

            void *struct_ptr = val.as_ptr;

            if (struct_ptr != NULL) {
                for (size_t i = 0; i < val.type->members.count; ++i) {
                    Member field = val.type->members.items[i];

                    sb_append_sb(sb, field.name);
                    sb_appendf(sb, ": ");

                    Value field_val = get_member_val(ctx, struct_ptr, field);
                    if (has_errors(ctx)) {
                        break;
                    }
                    
                    to_str(sb, ctx, field_val);

                    if (i < val.type->members.count - 1) {
                        sb_appendf(sb, ", ");
                    }
                }
            }
            
            sb_appendc(sb, '}');
        } break;

        case TYPE_FUNC: {
            Func *func = val.as_ptr;

            if (func != NULL) {
                sb_appendf(sb, "func ");

                sb_append_sb(sb, func->name);

                sb_appendc(sb, '(');
                
                for (size_t i = 0; i < func->args.count; ++i) {
                    Pattern arg = func->args.items[i];

                    sb_append_sb(sb, arg.name);
                    // TODO: print types
                    /*sb_appendf(sb, ": ");

                    Value field_val = get_member_val(ctx, func, arg);
                    if (has_errors(ctx)) {
                        break;
                    }

                    // TODO: print default values
                    to_str(sb, ctx, field_val);*/

                    if (i < func->args.count - 1) {
                        sb_appendf(sb, ", ");
                    }
                }
            }

            sb_appendc(sb, ')');
        } break;
            
        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
        } break;
    }
}

INT_CTYPE to_int(Context *ctx, Value val) {
    switch (val.type->kind) {
        case TYPE_PRIMITIVE: {
            if (val.type == INT_TYPE) {
                return val.as_int;
            }
            if (val.type == FLOAT_TYPE) {
                return (INT_CTYPE)val.as_float;
            }
            if (val.type == BOOL_TYPE) {
                return val.as_int;
            }
            if (val.type == CHAR_TYPE) {
                return val.as_int;
            }
            if (val.type == VOID_TYPE) {
                return 0;
            }
            
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return 0;
        }
            
        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return 0;
        }
    }
}

FLOAT_CTYPE to_float(Context *ctx, Value val) {
    switch (val.type->kind) {
        case TYPE_PRIMITIVE: {
            if (val.type == INT_TYPE) {
                return (FLOAT_CTYPE)val.as_int;
            }
            if (val.type == FLOAT_TYPE) {
                return val.as_float;
            }
            if (val.type == BOOL_TYPE) {
                return (FLOAT_CTYPE)val.as_int;
            }
            if (val.type == CHAR_TYPE) {
                return (FLOAT_CTYPE)val.as_int;
            }
            if (val.type == VOID_TYPE) {
                return 0.0;
            }
            
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return 0.0;
        }
        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return 0.0;
        }
    }
}

BOOL_CTYPE to_bool(Context *ctx, Value val) {
    switch (val.type->kind) {
        case TYPE_PRIMITIVE: {
            if (val.type == INT_TYPE) {
                return val.as_int != 0;
            }
            if (val.type == FLOAT_TYPE) {
                return val.as_float != 0.0;
            }
            if (val.type == BOOL_TYPE) {
                return val.as_int != 0;
            }
            if (val.type == CHAR_TYPE) {
                return val.as_int != 0;
            }
            if (val.type == STR_TYPE) {
                String_Builder *sb = val.as_ptr;
                return sb != NULL && sb->count != 0;
            }
            if (val.type == VOID_TYPE) {
                return false;
            }

            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return false;
        }

        case TYPE_ARRAY: {
            Array *arr = val.as_ptr;
            return arr != NULL && arr->count != 0;
        }

        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return false;
        }
    }
}

bool compatible_types(Type *type_1, Type *type_2) {
    if (type_1 == type_2) {
        return true;
    }
    
    switch (type_2->kind) {
        case TYPE_PRIMITIVE: {
            if (type_2 == INT_TYPE || type_2 == CHAR_TYPE || type_2 == FLOAT_TYPE) {
                return IS_INTLIKE(type_1) || type_1 == FLOAT_TYPE || type_1 == VOID_TYPE;
            }
            if (type_2 == BOOL_TYPE) {
                return IS_INTLIKE(type_1) || type_1 == FLOAT_TYPE || type_1 == VOID_TYPE || type_1 == STR_TYPE;
            }
            if (type_2 == STR_TYPE || type_2 == ANY_TYPE) {
                return true;
            }
            
            return false;
        }

        case TYPE_ARRAY: {
            if (type_1->el_type == ANY_TYPE || type_2->el_type == ANY_TYPE) {
                return true;
            }
        }
            
        default: {
            return false;
        }
    }
}

Value cast_value(Context *ctx, Value val, Type *type) {
    Value casted = create_value(type);
    
    if (val.type == type) {
        return val;
    }

    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            if (type == INT_TYPE) {
                casted.as_int = to_int(ctx, val);
                return casted;
            }
            if (type == FLOAT_TYPE) {
                casted.as_float = to_float(ctx, val);
                return casted;
            }
            if (type == BOOL_TYPE) {
                casted.as_int = to_bool(ctx, val);
                return casted;
            }
            if (type == CHAR_TYPE) {
                casted.as_int = to_int(ctx, val);
                return casted;
            }
            if (type == STR_TYPE) {
                String_Builder *sb = sb_alloc();
                to_str(sb, ctx, val);
                casted.as_ptr = sb;
                return casted;
            }
            if (type == ANY_TYPE) {
                return val;
            }
            
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return casted;
        }

        case TYPE_ARRAY: {
            if (val.type->el_type == ANY_TYPE) {
                casted.as_ptr = val.as_ptr;
                return casted;
            }
            if (type->el_type == ANY_TYPE) {
                return val;
            }
        }
            
        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return casted;
        }
    }
}

Value binary_plus(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int + rhs.as_int;
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_int + rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_float = lhs.as_float + rhs.as_int;
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_float + rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == CHAR_TYPE) {
        if (rhs.type == CHAR_TYPE) {
            String_Builder *sb = sb_alloc();
            sb_appendc(sb, lhs.as_int);
            sb_appendc(sb, rhs.as_int);
            val.as_ptr = sb;
            val.type = STR_TYPE;
            return val;
        }
        if (rhs.type == STR_TYPE) {
            sb_insertc(rhs.as_ptr, lhs.as_int, 0);
            val.as_ptr = rhs.as_ptr;
            val.type = STR_TYPE;
            return val;
        }
    }

    if (lhs.type == STR_TYPE) {
        if (rhs.type == CHAR_TYPE) {
            sb_appendc(lhs.as_ptr, rhs.as_int);
            val.as_ptr = lhs.as_ptr;
            val.type = STR_TYPE;
            return val;
        }
        if (rhs.type == STR_TYPE) {
            sb_append_sb(lhs.as_ptr, rhs.as_ptr);
            val.as_ptr = lhs.as_ptr;
            val.type = STR_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_minus(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (rhs.type == INT_TYPE || rhs.type == BOOL_TYPE) {
            val.as_int = lhs.as_int - rhs.as_int;
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == CHAR_TYPE) {
            val.as_int = lhs.as_int - rhs.as_int;
            val.type = CHAR_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_int - rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_float = lhs.as_float - rhs.as_int;
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_float - rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_mul(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int * rhs.as_int;
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_int * rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_float = lhs.as_float * rhs.as_int;
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = lhs.as_float * rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_div(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            if (rhs.as_int == 0) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_int = lhs.as_int / rhs.as_int;
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            if (rhs.as_float == 0.0f) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = lhs.as_int / rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            if (rhs.as_int == 0) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = lhs.as_float / rhs.as_int;
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            if (rhs.as_float == 0.0f) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = lhs.as_float / rhs.as_float;
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_mod(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (lhs.type == INT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            if (rhs.as_int == 0) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_int = lhs.as_int % rhs.as_int;
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            if (rhs.as_float == 0.0f) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = fmod(lhs.as_int, rhs.as_float);
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            if (rhs.as_int == 0) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = fmod(lhs.as_float, rhs.as_int);
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            if (rhs.as_float == 0.0f) {
                append_error(ctx, ERROR_DIV_BY_ZERO);
                return val;
            }
            val.as_float = fmod(lhs.as_float, rhs.as_float);
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_pow(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (lhs.type == INT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = pow(lhs.as_int, rhs.as_int);
            val.type = INT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = pow(lhs.as_int, rhs.as_float);
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_float = pow(lhs.as_float, rhs.as_int);
            val.type = FLOAT_TYPE;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_float = pow(lhs.as_float, rhs.as_float);
            val.type = FLOAT_TYPE;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_logic_and(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    bool l = to_bool(ctx, lhs);
    if (has_errors(ctx)) return val;

    bool r = to_bool(ctx, rhs);
    if (has_errors(ctx)) return val;

    val.as_int = l && r;
    return val;
}

Value binary_bitwise_and(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (lhs.type == INT_TYPE && rhs.type == INT_TYPE) {
        val.as_int = lhs.as_int & rhs.as_int;
        val.type = INT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_logic_or(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    bool l = to_bool(ctx, lhs);
    if (has_errors(ctx)) return val;

    bool r = to_bool(ctx, rhs);
    if (has_errors(ctx)) return val;

    val.as_int = l || r;
    return val;
}

Value binary_bitwise_or(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    if (lhs.type == INT_TYPE && rhs.type == INT_TYPE) {
        val.as_int = lhs.as_int | rhs.as_int;
        val.type = INT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_gt(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int > rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_int > rhs.as_float;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_float > rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_float > rhs.as_float;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_lt(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int < rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_int < rhs.as_float;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_float < rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_float < rhs.as_float;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_gteq(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int >= rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_int >= rhs.as_float;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_float >= rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_float >= rhs.as_float;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_lteq(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int <= rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_int <= rhs.as_float;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_float <= rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_float <= rhs.as_float;
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_eq(Context *ctx, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    if (IS_INTLIKE(lhs.type)) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_int == rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_int == rhs.as_float;
            return val;
        }
    }

    if (lhs.type == FLOAT_TYPE) {
        if (IS_INTLIKE(rhs.type)) {
            val.as_int = lhs.as_float == rhs.as_int;
            return val;
        }
        if (rhs.type == FLOAT_TYPE) {
            val.as_int = lhs.as_float == rhs.as_float;
            return val;
        }
    }

    if (lhs.type == STR_TYPE) {
        if (rhs.type == STR_TYPE) {
            String_Builder *a = lhs.as_ptr;
            String_Builder *b = rhs.as_ptr;

            if (a->count != b->count) {
                val.as_int = false;
                return val;
            }

            for (size_t i = 0; i < a->count; i++) {
                if (a->items[i] != b->items[i]) {
                    val.as_int = false;
                    return val;
                }
            }

            val.as_int = true;
            return val;
        }

        if (rhs.type == CHAR_TYPE) {
            String_Builder *s = lhs.as_ptr;
            val.as_int = (s->count == 1 && s->items[0] == (char)rhs.as_int);
            return val;
        }
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value binary_neq(Context *ctx, Value lhs, Value rhs) {
    Value val = binary_eq(ctx, lhs, rhs);
    val.as_int = !val.as_int;
    return val;
}

Value unary_plus(Context *ctx, Value x) {
    Value val = create_value(VOID_TYPE);

    if (x.type == INT_TYPE) {
        val.as_int = x.as_int;
        val.type = INT_TYPE;
        return val;
    }

    if (x.type == FLOAT_TYPE) {
        val.as_float = x.as_float;
        val.type = FLOAT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value unary_minus(Context *ctx, Value x) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(x.type)) {
        val.as_int = -x.as_int;
        val.type = INT_TYPE;
        return val;
    }

    if (x.type == FLOAT_TYPE) {
        val.as_float = -x.as_float;
        val.type = FLOAT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value unary_increment(Context *ctx, Value x) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(x.type)) {
        val.as_int = x.as_int + 1;
        val.type = INT_TYPE;
        return val;
    }

    if (x.type == FLOAT_TYPE) {
        val.as_float = x.as_float + 1;
        val.type = FLOAT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value unary_decrement(Context *ctx, Value x) {
    Value val = create_value(VOID_TYPE);

    if (IS_INTLIKE(x.type)) {
        val.as_int = x.as_int - 1;
        val.type = INT_TYPE;
        return val;
    }

    if (x.type == FLOAT_TYPE) {
        val.as_float = x.as_float - 1;
        val.type = FLOAT_TYPE;
        return val;
    }

    append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
    return val;
}

Value unary_not(Context *ctx, Value x) {
    Value val = create_value(BOOL_TYPE);
    val.as_int = !to_bool(ctx, x);
    return val;
}

#endif