#ifndef BUILT_IN_H
#define BUILT_IN_H

#include "executor/symbols/symbol.h"

#define builtin_funcs_count 15
#define builtin_types_count 8

extern FuncBuiltIn builtin_funcs[];
extern Type *builtin_types[];

#endif //BUILT_IN_H
