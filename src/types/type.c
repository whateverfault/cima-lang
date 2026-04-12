#include "type.h"

#include "parser/parser.h"
#include "executor/executor.h"

void format_str(String_Builder *sb, Context *context, String_View fmt_sv, Array *va_args) {
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
            if (va_args == NULL || va_arg_pos >= va_args->els.count) {
                append_error(context, ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT);
                return;
            }
            
            to_str(sb, context, va_args->els.items[va_arg_pos]);
            ++va_arg_pos;
        }
        else {
            AST_Node *expr = parse(&l);
            if (expr->kind == AST_ERROR) {
                append_error(context, ((AST_NodeError*)expr)->err);
                ast_free(expr);
                return;
            }
            
            EvalResult result = execute_expr(context, expr);
            append_error(context, get_signal_error(result.sig));
            if (has_errors(context)) {
                ast_free(expr);
                return;
            }

            if (result.val.type->tag == TYPE_STR) {
                String_View sv = {0};
                sv_from_sb(&sv, result.val.as_ptr);
                format_str(sb, context, sv, va_args);
                if (has_errors(context)) {
                    ast_free(expr);
                    return;
                }
            }
            else {
                to_str(sb, context, result.val);
                if (has_errors(context)) {
                    ast_free(expr);
                    return;
                }
            }
            ast_free(expr);
        }

        if (l.cur.kind != TOKEN_RBRACE) {
            append_error(context, ERROR_UNEXPECTED_TOKEN);
            return;
        }
        
        lexer_next(&l);
    }

    if (va_args != NULL && va_arg_pos != va_args->els.count) {
        append_error(context, ERROR_FORMAT_MISMATCHES_VA_ARGS_COUNT);
        return;
    }
    
    sb_append_sv(sb, &l.skipped);
}

void to_str(String_Builder *sb, Context *context, Value val) {
    switch (val.type->tag) {
        case TYPE_INT: {
            sb_appendf(sb, "%d", val.as_int);
        } break;

        case TYPE_FLOAT: {
            sb_appendf(sb, "%f", val.as_float);
        } break;

        case TYPE_BOOL: {
            sb_appendf(sb, "%s", val.as_int == 0? "false" : "true");
        } break;
                    
        case TYPE_STR: {
            sb_append_sb(sb, val.as_ptr);
        } break;

        case TYPE_CHAR: {
            sb_appendf(sb, "%c", (char)val.as_int);
        } break;

        case TYPE_VOID: {
            sb_appendf(sb, "()");
        } break;

        case TYPE_ARRAY: {
            sb_appendc(sb, '[');

            Array *arr = (void*)val.as_ptr;

            if (arr != NULL) {
                for (size_t i = 0; i < arr->els.count; ++i) {
                    to_str(sb, context, arr->els.items[i]);

                    if (i < arr->els.count - 1) {
                        sb_appendf(sb, ", ");
                    }
                }
            }
            
            sb_appendc(sb, ']');
        } break;

        case TYPE_PTR: {
            sb_appendf(sb, "%p", val.as_ptr);
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
        } break;
    }
}

bool to_bool(Context *context, Value val) {
    switch (val.type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            return val.as_int != 0;
        }

        case TYPE_FLOAT: {
            return val.as_float != 0.0;
        }
            
        case TYPE_STR: {
            String_Builder *sb = val.as_ptr;
            return sb != NULL && sb->count != 0;
        }

        case TYPE_VARIADIC:
        case TYPE_ARRAY: {
            Array *arr = val.as_ptr;
            return arr != NULL && arr->els.count != 0;
        }
        
        case TYPE_ANY:
        case TYPE_PTR: {
            return val.as_ptr != NULL;
        }

        case TYPE_VOID: {
            return false;
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return false;
        }
    }
}
