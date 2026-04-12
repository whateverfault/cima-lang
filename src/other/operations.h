#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <math.h>

#include "executor/executor.h"

Value binary_plus(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_CHAR:
                case TYPE_BOOL:
                case TYPE_INT: {
                    val.as_int = lhs.as_int + rhs.as_int;
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_int + rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_INT: {
                    val.as_float = lhs.as_float + rhs.as_int;
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float + rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_CHAR: {
            switch (rhs_type->tag) {
                case TYPE_INT: {
                    val.as_int = lhs.as_int + rhs.as_int;
                    val.type = CHAR_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_int + rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                
                case TYPE_BOOL: {
                    val.as_int = lhs.as_int + rhs.as_int;
                    val.type = CHAR_TYPE;
                    return val;
                }

                case TYPE_CHAR: {
                    String_Builder *sb = sb_alloc();
                    sb_appendc(sb, lhs.as_int);
                    sb_appendc(sb, rhs.as_int);
                    val.as_ptr = sb;
                    val.type = STR_TYPE;
                    return val;
                }
                    
                case TYPE_STR: {
                    sb_insertc(rhs.as_ptr, lhs.as_int, 0);
                    val.as_ptr = rhs.as_ptr;
                    val.type = STR_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        } break;

        case TYPE_STR: {
            switch (rhs_type->tag) {
                case TYPE_CHAR: {
                    sb_appendc(lhs.as_ptr, rhs.as_int);
                    val.as_ptr = lhs.as_ptr;
                    val.type = STR_TYPE;
                    return val;
                }
                    
                case TYPE_STR: {
                    sb_append_sb(lhs.as_ptr, rhs.as_ptr);
                    val.as_ptr = lhs.as_ptr;
                    val.type = STR_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_minus(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int - rhs.as_int;
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_int - rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_CHAR: {
                    val.as_int = lhs.as_int - rhs.as_int;
                    val.type = CHAR_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float - rhs.as_int;
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float - rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_mul(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int * rhs.as_int;
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_int * rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float * rhs.as_int;
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float * rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_div(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    if (rhs.as_int == 0) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_int = lhs.as_int / rhs.as_int;
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0.0f) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = lhs.as_int / rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = lhs.as_float / rhs.as_int;
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0.0f) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = lhs.as_float / rhs.as_float;
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_mod(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    if (rhs.as_int == 0) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_int = lhs.as_int % rhs.as_int;
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0.0f) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = fmod(lhs.as_int, rhs.as_float);
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = fmod(lhs.as_float, rhs.as_int);
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0.0f) {
                        append_error(context, ERROR_DIV_BY_ZERO);
                        return val;
                    }
                    
                    val.as_float = fmod(lhs.as_float, rhs.as_float);
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_pow(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = pow(lhs.as_int, rhs.as_int);
                    val.type = INT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = pow(lhs.as_int, rhs.as_float);
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = pow(lhs.as_float, rhs.as_int);
                    val.type = FLOAT_TYPE;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = pow(lhs.as_float, rhs.as_float);
                    val.type = FLOAT_TYPE;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_logic_and(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);
    
    bool lhs_val = to_bool(context, lhs);
    if (has_errors(context)) {
        return val;
    }

    bool rhs_val = to_bool(context, rhs);
    if (has_errors(context)) {
        return val;
    }

    val.as_int = lhs_val && rhs_val;
    return val;
}

Value binary_bitwise_and(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;

    switch (lhs_type->tag) {
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT: {
                    val.as_int = val.as_int = lhs.as_int & rhs.as_int;
                    val.type = INT_TYPE;
                }
                
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
        
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_logic_or(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);
    
    bool lhs_val = to_bool(context, lhs);
    if (has_errors(context)) {
        return val;
    }

    bool rhs_val = to_bool(context, rhs);
    if (has_errors(context)) {
        return val;
    }

    val.as_int = lhs_val || rhs_val;
    return val;
}

Value binary_bitwise_or(Context *context, Value lhs, Value rhs) {
    Value val = create_value(VOID_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;

    switch (lhs_type->tag) {
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT: {
                    val.as_int = val.as_int = lhs.as_int | rhs.as_int;
                    val.type = INT_TYPE;
                }
                
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
        
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_gt(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int > rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_int = lhs.as_int > rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float > rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float > rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_lt(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int < rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_int = lhs.as_int < rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float < rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float < rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_gteq(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int >= rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_int = lhs.as_int >= rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float >= rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float >= rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_lteq(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int <= rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_int = lhs.as_int <= rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float <= rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float <= rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_eq(Context *context, Value lhs, Value rhs) {
    Value val = create_value(BOOL_TYPE);

    ValueType *lhs_type = (void*)lhs.type;
    ValueType *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->tag) {
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_INT: {
            switch (rhs_type->tag) {
                case TYPE_INT:
                case TYPE_CHAR:
                case TYPE_BOOL:{
                    val.as_int = lhs.as_int == rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_int = lhs.as_int == rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->tag) {
                case TYPE_BOOL:
                case TYPE_CHAR:
                case TYPE_INT: {
                    val.as_float = lhs.as_float == rhs.as_int;
                    return val;
                }

                case TYPE_FLOAT: {
                    val.as_float = lhs.as_float == rhs.as_float;
                    return val;
                }
                    
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }

        case TYPE_STR: {
            switch (rhs_type->tag) {
                case TYPE_STR: {
                    val.as_int = true;
                    
                    String_Builder *lhs_str = lhs.as_ptr;
                    String_Builder *rhs_str = rhs.as_ptr;

                    if (lhs_str->count != rhs_str->count) {
                        val.as_int = false;
                        return val;
                    }

                    for (size_t i = 0; i < lhs_str->count; ++i) {
                        if (lhs_str->items[i] != rhs_str->items[i]) {
                            val.as_int = false;
                            return val;
                        }
                    }
                    
                    return val;
                }
                
                case TYPE_CHAR: {
                    String_Builder *str = lhs.as_ptr;

                    val.as_int = str->count == 1 && str->items[0] == (char)rhs.as_int;
                    return val;
                }
                
                default: {
                    append_error(context, ERROR_INCOMPATIBLE_TYPES);
                    return val;
                }
            }
        }
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
            return val;
        }
    }
}

Value binary_neq(Context *context, Value lhs, Value rhs) {
    Value val = binary_eq(context, lhs, rhs);
    val.as_int = !val.as_int;
    return val;
}

Value unary_plus(Context *context, Value x) {
    Value val = create_value(VOID_TYPE);
    
    ValueType *val_type = (void*)val.type;
    
    switch (val_type->tag) {
        case TYPE_INT: {
            val.as_int = x.as_int;
            val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            val.as_float = x.as_float;
            val.type = FLOAT_TYPE;
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
        }
    }

    return val;
}

Value unary_minus(Context *context, Value x) {
    Value val = create_value(VOID_TYPE);
    
    switch (x.type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            val.as_int = -x.as_int;
            val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            val.as_float = -x.as_float;
            val.type = FLOAT_TYPE;
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
        }
    }

    return val;
}

Value unary_increment(Context *context, Value x) {
    Value val = create_value(VOID_TYPE);

    switch (x.type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            val.as_int = x.as_int + 1;
            val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            val.as_float = x.as_float + 1;
            val.type = FLOAT_TYPE;
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
        }
    }
    
    return val;
}

Value unary_decrement(Context *context, Value x) {
    Value val = create_value(VOID_TYPE);

    switch (x.type->tag) {
        case TYPE_CHAR:
        case TYPE_BOOL:
        case TYPE_INT: {
            val.as_int = x.as_int - 1;
            val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            val.as_float = x.as_float - 1;
            val.type = FLOAT_TYPE;
        } break;
            
        default: {
            append_error(context, ERROR_INCOMPATIBLE_TYPES);
        }
    }
    
    return val;
}

Value unary_not(Context *context, Value x) {
    Value val = create_value(BOOL_TYPE);
    val.as_int = !to_bool(context, x);
    return val;
}

#endif //OPERATIONS_H
