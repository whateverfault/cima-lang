#include "type.h"

#include "parser/parser.h"
#include "executor/executor.h"

void to_str(String_Builder *sb, Context *context, Value val) {
    // TODO: Implement printing of arrays
    switch (val.type->kind) {
        case TYPE_BASIC: {
            ValueTypeBasic *basic_type = (void*)val.type;

            switch (basic_type->type) {
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
                    sb_appendf(sb, "%s", (char*)val.as_ptr);
                } break;

                case TYPE_CHAR: {
                    sb_appendf(sb, "%c", (char)val.as_int);
                } break;

                case TYPE_VOID: {
                    sb_appendf(sb, "()");
                } break;
            
                default: {
                    da_append(context->errors, ERROR_INCOMPATIBLE_TYPES);
                } break;
            }
        } break;

        case TYPE_ARRAY: {
            da_append(context->errors, ERROR_INCOMPATIBLE_TYPES);
        } break;

        case TYPE_PTR: {
            sb_appendf(sb, "%p", val.as_ptr);
        } break;
            
        default: {
            da_append(context->errors, ERROR_INCOMPATIBLE_TYPES);
        } break;
    }
}
