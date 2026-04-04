#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "executor/executor.h"

Output binary_plus(Value lhs, Value rhs) {
    // TODO: Implement string concatenation
    
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_int = lhs.as_int + rhs.as_int;
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_int + rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_float = lhs.as_float + rhs.as_int;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_float + rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output binary_minus(Value lhs, Value rhs) {
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_int = lhs.as_int - rhs.as_int;
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_int - rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_float = lhs.as_float - rhs.as_int;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_float - rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output binary_mul(Value lhs, Value rhs) {
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_int = lhs.as_int * rhs.as_int;
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_int * rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_float = lhs.as_float * rhs.as_int;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = lhs.as_float * rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output binary_div(Value lhs, Value rhs) {
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_int = lhs.as_int / rhs.as_int;
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = lhs.as_int / rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = lhs.as_float / rhs.as_int;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = lhs.as_float / rhs.as_float;
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output binary_mod(Value lhs, Value rhs) {
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_int = lhs.as_int % rhs.as_int;
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = fmod(lhs.as_int, rhs.as_float);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    if (rhs.as_int == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = fmod(lhs.as_float, rhs.as_int);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    if (rhs.as_float == 0) {
                        output.err = ERROR_DIV_BY_ZERO;
                        return output;
                    }
                    
                    output.val.as_float = fmod(lhs.as_float, rhs.as_float);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output binary_pow(Value lhs, Value rhs) {
    Output output = alloc_output(VOID_TYPE);
    
    if (lhs.type->kind != TYPE_BASIC && rhs.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *lhs_type = (void*)lhs.type;
    ValueTypeBasic *rhs_type = (void*)rhs.type;
    
    switch (lhs_type->type) {
        case TYPE_INT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_int = pow(lhs.as_int, rhs.as_int);
                    output.val.type = INT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = pow(lhs.as_int, rhs.as_float);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }

        case TYPE_FLOAT: {
            switch (rhs_type->type) {
                case TYPE_INT: {
                    output.val.as_float = pow(lhs.as_float, rhs.as_int);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }

                case TYPE_FLOAT: {
                    output.val.as_float = pow(lhs.as_float, rhs.as_float);
                    output.val.type = FLOAT_TYPE;
                    return output;
                }
                    
                default: {
                    output.err = ERROR_INCOMPATIBLE_TYPES;
                    return output;
                }
            }
        }
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output unary_plus(Value val) {
    Output output = alloc_output(VOID_TYPE);
    
    if (val.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *val_type = (void*)val.type;
    
    switch (val_type->type) {
        case TYPE_INT: {
            output.val.as_int = val.as_int;
            output.val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            output.val.as_float = val.as_float;
            output.val.type = FLOAT_TYPE;
        } break;
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

Output unary_minus(Value val) {
    Output output = alloc_output(VOID_TYPE);
    
    if (val.type->kind != TYPE_BASIC) {
        output.err = ERROR_INCOMPATIBLE_TYPES;
        return output;
    }

    ValueTypeBasic *val_type = (void*)val.type;
    
    switch (val_type->type) {
        case TYPE_INT: {
            output.val.as_int = -val.as_int;
            output.val.type = INT_TYPE;
        } break;

        case TYPE_FLOAT: {
            output.val.as_float = -val.as_float;
            output.val.type = FLOAT_TYPE;
        } break;
            
        default: {
            output.err = ERROR_INCOMPATIBLE_TYPES;
            return output;
        }
    }
}

#endif //OPERATIONS_H
