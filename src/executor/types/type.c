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
    .members = {0},
    .constant = true,
};

const Type float_symb = (Type){
    .name = &float_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(FLOAT_CTYPE),
    .members = {0},
    .constant = true,
};

const Type bool_symb = (Type){
    .name = &bool_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(BOOL_CTYPE),
    .members = {0},
    .constant = true,
};

const Type char_symb = (Type){
    .name = &char_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(CHAR_CTYPE),
    .members = {0},
    .constant = true,
};

const Type str_symb = (Type){
    .name = &str_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(STR_CTYPE),
    .members = {0},
    .constant = true,
};

const Type any_symb = (Type){
    .name = &any_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = sizeof(PTR_CTYPE),
    .members = {0},
    .constant = true,
};

const Type void_symb = (Type){
    .name = &void_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_PRIMITIVE,
    .size = 0,
    .members = {0},
    .constant = true,
};

const Type variadic_symb = (Type){
    .name = &variadic_sb,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_ARRAY,
    .size = sizeof(Array),
    .el_type = NULL,
    .members = {0},
    .constant = true,
};

const Type array_any_symb = (Type){
    .name = NULL,
    .symb_kind = SYMB_TYPE,
    .kind = TYPE_ARRAY,
    .size = sizeof(Array),
    .el_type = ANY_TYPE,
    .members = {0},
    .constant = true,
};

const Type array_va_symb = (Type){
    .name = NULL,
    .symb_kind = SYMB_TYPE,
    .size = sizeof(Array),
    .el_type = VARIADIC_TYPE,
    .members = {0},
    .constant = true,
};

#define IS_INTLIKE(t) ((t)==INT_TYPE||(t)==BOOL_TYPE||(t)==CHAR_TYPE)
#define FIELD_ADDR(struct_addr, offset) ((char*)struct_addr + offset)

size_t hash_type(Type *t);
size_t hash_member(Member *member) {
    size_t h = 0;

    h = hash_combine(h, member->kind);
    h = hash_combine(h, hash_type(member->type));
    if (member->name != NULL && member->name->count > 0) {
        h = hash_combine(h, hash_nkey((unsigned char*)member->name->items, member->name->count));
    }
    
    return h;
}

size_t hash_members(Members members) {
    size_t h = 0;

    for (size_t i = 0; i < members.count; ++i) {
        h = hash_combine(h, hash_member(members.items[i]));
    }

    return h;
}

size_t hash_type(Type *t) {
    size_t h = 0;

    h = hash_combine(h, t->kind);
    h = hash_combine(h, hash_members(t->members));
    
    if (t->el_type != NULL) {
        h = hash_combine(h, hash_type(t->el_type));
    }

    return h;
}

size_t hash_patterns(Patterns patterns) {
    size_t h = 0;

    for (size_t i = 0; i < patterns.count; ++i) {
        Pattern pattern = patterns.items[i];
        h = hash_combine(h, hash_type(pattern.type));
        
        if (pattern.name != NULL && pattern.name->count > 0) {
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

bool get_type_field(Type *type, String_View name, Member **field) {
    for (size_t i = 0; i < type->members.count; ++i) {
        if (sv_cmp_sb(&name, type->members.items[i]->name)) {
            *field = type->members.items[i];
            return true;
        }
    }

    return false;
}

void alloc_type_value(Value *val, Type *type) {
    val->type = type;
    
    if (type->kind == TYPE_ARRAY) {
        val->as_ptr = alloc_array_value(type->el_type);
    }
    else if (type->kind == TYPE_STRUCT) {
        val->as_struct = alloc_struct_value(type);
    }
    else if (type == STR_TYPE) {
        val->as_ptr = calloc(1, sizeof(STR_CTYPE));
    }
}

Array *alloc_array_value(Type *el_type) {
    Array *arr = calloc(1, sizeof(Array));
    assert(arr != NULL && "Failed to allocate array");
    arr->el_type = el_type;

    return arr;
}

Struct alloc_struct_value(Type *type) {
    Struct strct = {
        .type = type,
        .members = hm_alloc(),
    };

    for (size_t i = 0; i < type->members.count; ++i) {
        Member *member = type->members.items[i];
        Value val = create_value(member->type);
        alloc_type_value(&val, member->type);
        hm_put_hashed(strct.members, hash_member(member), alloc_value(val));
    }
    
    assert(strct.members != NULL && "Failed to allocate members");
    return strct;
}

bool get_member(Type *type, String_View *name_sv, Member **member) {
    for (size_t i = 0; i < type->members.count; ++i) {
        Member *m = type->members.items[i];

        if (m->name != NULL && sv_cmp_sb(name_sv, m->name)) {
            *member = m;
            return true;
        }
    }

    return false;
}

Value get_static_member_val(Member *member) {
    Value member_val = create_value(member->type);

    if (member->kind == MEMBER_METHOD) {
        member_val.as_ptr = member->method.func;
        member_val.type = member->method.func->type;
        return member_val;
    }

    member_val = *member->field.static_initializer;
    return member_val;
}

Value get_static_member_ref(Context *ctx, Member *member) {
    Value member_ref = create_value(VOID_TYPE);

    if (member->kind == MEMBER_METHOD) {
        Value val = create_value(member->method.func->type);
        val.as_ptr = member->method.func;
        
        member_ref.type = alloc_ref_type(ctx, member->method.func->type);
        member_ref.as_ptr = alloc_value(val);
        return member_ref;
    }

    member_ref.type = alloc_ref_type(ctx, member->type);
    member_ref.as_ptr = member->field.static_initializer;
    
    return member_ref;
}

Value get_member_val(Struct strct, Member *member) {
    Value member_val = create_value(member->type);

    if (member->kind == MEMBER_METHOD) {
        member_val.as_ptr = member->method.func;
        member_val.type = member->method.func->type;
        return member_val;
    }

    Value *val = hm_get_hashed(strct.members, hash_member(member));
    return *val;
}

Value get_member_ref(Context *ctx, Struct strct, Member *member) {
    Value member_ref = create_value(VOID_TYPE);

    if (member->kind == MEMBER_METHOD) {
        Value val = create_value(member->type);
        val.as_ptr = member->method.func;
        
        member_ref.type = alloc_ref_type(ctx, member->method.func->type);
        member_ref.as_ptr = alloc_value(val);
        return member_ref;
    }

    member_ref.type = alloc_ref_type(ctx, member->type);
    member_ref.as_ptr = hm_get_hashed(strct.members, hash_member(member));
    
    return member_ref;
}

Value copy_value(Value *value) {
    switch (value->type->kind) {
        case TYPE_FUNC:
        case TYPE_TYPE:
        case TYPE_REF:
        case TYPE_ARRAY:
        case TYPE_PRIMITIVE: {
            return *value;
        }

        case TYPE_STRUCT: {
            Value val = create_value(value->type);
            val.as_struct = alloc_struct_value(value->type);

            for (size_t i = 0; i < value->type->members.count; ++i) {
                Member *member = value->type->members.items[i];
                size_t hash = hash_member(member);

                Value *src = hm_get_hashed(value->as_struct.members, hash);
                Value *copied = alloc_value(copy_value(src));
                hm_put_hashed(val.as_struct.members, hash, copied);
            }

            return val;
        }
    }

    assert(0 && "UNREACHABLE");
}

void assign_field(Context *ctx, Struct strct, Member *member, Value val) {
    if (member->kind != MEMBER_FIELD) {
        return;
    }
    
    Value *field_val = hm_get_hashed(strct.members, hash_member(member));
    if (field_val == NULL) {
        append_error(ctx, ERROR_NOT_DEFINED);
        return;
    }

    Value value = cast_value(ctx, val, member->type);
    if (has_errors(ctx)) {
        return;
    }

    *field_val = copy_value(&value);
}

Type *alloc_struct_type(String_Builder *name_sb, Members members) {
    Type *type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");
    
    *type = (Type){
        .name = name_sb,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_STRUCT,
        .size = sizeof(Value) * members.count,
        .el_type = NULL,
        .members = members,
    };

    return type;
}

Type *alloc_ref_type(Context *ctx, Type *t) {
    size_t h = hash_type(t);
    Type *type = hm_get_hashed(ctx->type_cache.ref_cache, h);
    if (type != NULL) {
        return type;
    }
    
    type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");
    
    *type = (Type){
        .name = NULL,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_REF,
        .size = sizeof(PTR_CTYPE),
        .el_type = t,
        .members = (Members){0},
    };

    hm_put_hashed(ctx->type_cache.ref_cache, h, type);
    return type;
}

Type *alloc_type_type(Context *ctx, Type *t) {
    size_t h = hash_type(t);
    Type *type = hm_get_hashed(ctx->type_cache.type_cache, h);
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
        .el_type = t,
        .members = (Members){0},
    };

    hm_put_hashed(ctx->type_cache.type_cache, h, type);
    return type;
}

Type *alloc_func_type(Context *ctx, Func *func) {
    size_t h = hash_func(func);
    Type *type = hm_get_hashed(ctx->type_cache.func_cache, h);
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
        .el_type = NULL,
        .members = (Members){0},
        .func = func,
    };

    hm_put_hashed(ctx->type_cache.func_cache, h, type);
    return type;
}

Type *alloc_array_type(Context *ctx, Type *el_type) {
    Type tmp = {
        .kind = TYPE_ARRAY,
        .el_type = el_type,
        .members = {0}
    };
    
    size_t h = hash_type(&tmp);
    Type *type = hm_get_hashed(ctx->type_cache.array_cache, h);
    if (type != NULL) {
        return type;
    }
        
    type = (Type*)calloc(1, sizeof(Type));
    assert(type != NULL && "Failed to allocate type");

    if (el_type == NULL) {
        el_type = ANY_TYPE;
    }
    
    *type = (Type){
        .name = NULL,
        .symb_kind = SYMB_TYPE,
        .constant = true,
        .kind = TYPE_ARRAY,
        .size = sizeof(Array),
        .el_type = el_type,
        .members = (Members){0},
    };

    hm_put_hashed(ctx->type_cache.array_cache, h, type);
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
            
            to_str(sb, ctx, va_args->items[va_arg_pos], 0);
            if (has_errors(ctx)) {
                return;
            }
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
                to_str(sb, ctx, result.val, 0);
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

void to_str(String_Builder *sb, Context *ctx, Value val, size_t depth) {
    if (depth >= 1024) {
        append_error(ctx, ERROR_RECURSION_LIMIT_EXCEEDED);
        return;
    }
    
    ++depth;
    
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
                    to_str(sb, ctx, arr->items[i], depth);
                    if (has_errors(ctx)) {
                        return;
                    }

                    if (i < arr->count - 1) {
                        sb_appendf(sb, ", ");
                    }
                }
            }
            
            sb_appendc(sb, ']');
        } break;

        case TYPE_STRUCT: {
            sb_appendc(sb, '{');

            Struct strct = val.as_struct;

            size_t fields_count = 0;
            for (size_t i = 0; i < val.type->members.count; ++i) {
                Member *member = val.type->members.items[i];
                if (member->kind == MEMBER_FIELD) {
                    ++fields_count;
                }
            }

            size_t fields_printed = 0;
            for (size_t i = 0; i < val.type->members.count; ++i) {
                Member *member = val.type->members.items[i];
                if (member->kind == MEMBER_METHOD) {
                    continue;
                }
                
                sb_append_sb(sb, member->name);
                sb_appendf(sb, ": ");

                Value field_val = get_member_val(strct, member);
                if (has_errors(ctx)) {
                    break;
                }
                
                to_str(sb, ctx, field_val, depth);
                if (has_errors(ctx)) {
                    return;
                }

                ++fields_printed;
                
                if (fields_printed < fields_count) {
                    sb_appendf(sb, ", ");
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

        case TYPE_REF: {
            while (val.type->kind == TYPE_REF) {
                val = *(Value*)val.as_ptr;
            }

            to_str(sb, ctx, val, depth);
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
    
    switch (type_1->kind) {
        case TYPE_PRIMITIVE: {
            if (IS_INTLIKE(type_1) || type_1 == FLOAT_TYPE || type_1 == VOID_TYPE) {
                return IS_INTLIKE(type_2) || type_2 == FLOAT_TYPE || type_2 == VOID_TYPE || type_2 == STR_TYPE;
            }
            if (type_1 == STR_TYPE || type_1 == ANY_TYPE || type_2 == STR_TYPE || type_2 == ANY_TYPE) {
                return true;
            }
        } break;

        case TYPE_ARRAY: {
            if (type_1->el_type == ANY_TYPE || type_2->el_type == ANY_TYPE) {
                return true;
            }
        } break;

        default: break;
    }

    return false;
}

Value cast_value(Context *ctx, Value val, Type *type) {
    if (val.type == type) {
        return val;
    }
    
    Value casted = create_value(type);

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
                to_str(sb, ctx, val, 0);
                if (has_errors(ctx)) {
                    return casted;
                }
                
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
            if (val.type->kind != TYPE_ARRAY) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return casted;
            }

            if (val.type->el_type == ANY_TYPE) {
                casted.as_ptr = val.as_ptr;
                return casted;
            }
            if (type->el_type == ANY_TYPE) {
                return val;
            }
        } break;

        case TYPE_STRUCT: {
            while (val.type->kind == TYPE_REF) {
                val = *(Value*)val.as_ptr;
            }
            
            if (!compatible_types(val.type, type)) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return val;
            }

            return val;
        }

        case TYPE_REF: {
            size_t val_ref_depth = 1;
            Value temp_val = val;
            while (temp_val.type->kind == TYPE_REF) {
                temp_val = *(Value*)temp_val.as_ptr;
                ++val_ref_depth;
            }

            size_t type_ref_depth = 1;
            Type *temp_type = type;
            while (temp_type->kind == TYPE_REF) {
                temp_type = temp_type->el_type;
                ++type_ref_depth;
            }

            if (val_ref_depth < type_ref_depth) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return val;
            }
            
            while (val_ref_depth > type_ref_depth) {
                temp_val = *(Value*)temp_val.as_ptr;
                --val_ref_depth;
            }

            if (!compatible_types(temp_val.type, temp_type)) {
                append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
                return val;
            }

            return val;
        }
            
        default: {
            append_error(ctx, ERROR_INCOMPATIBLE_TYPES);
            return casted;
        }
    }

    return casted;
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
            String_Builder *sb = sb_alloc();
            sb_appendc(sb, lhs.as_int);
            sb_append_sb(sb, rhs.as_ptr);
            val.as_ptr = sb;
            val.type = STR_TYPE;
            return val;
        }
    }

    if (lhs.type == STR_TYPE) {
        if (rhs.type == CHAR_TYPE) {
            String_Builder *sb = sb_alloc();
            sb_append_sb(sb, lhs.as_ptr);
            sb_appendc(sb, rhs.as_int);
            val.as_ptr = sb;
            val.type = STR_TYPE;
            return val;
        }
        if (rhs.type == STR_TYPE) {
            String_Builder *sb = sb_alloc();
            sb_append_sb(sb, lhs.as_ptr);
            sb_append_sb(sb, rhs.as_ptr);
            val.as_ptr = sb;
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
            val.as_float = (float)lhs.as_int / (float)rhs.as_int;
            val.type = FLOAT_TYPE;
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