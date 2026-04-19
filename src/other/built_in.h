#ifndef BUILT_IN_H
#define BUILT_IN_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#include "executor/symbols/symbol.h"

#define builtin_vars_count 3
#define builtin_funcs_count 15
#define builtin_types_count 8

extern Var builtin_vars[];
extern FuncBuiltIn builtin_funcs[];
extern Type *builtin_types[];

#endif //BUILT_IN_H
