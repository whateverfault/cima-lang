#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "parser.h"
#include "nothing/nothing.h"

void patterns_free(Patterns patterns) {
    for (size_t i = 0; i < patterns.count; ++i) {
        free(patterns.items[i].name);
    }

    da_free(patterns);
}

void args_free(Args args) {
    for (size_t i = 0; i < args.count; ++i) {
        ast_free(args.items[i].node);
        
        if (args.items[i].has_name) {
            free(args.items[i].name);
        }
    }

    da_free(args);
}


void ast_free(AST_Node *root) {
    switch (root->kind) {
        case AST_BINOP: {
            AST_NodeBinOp *binop = (void*)root;
            ast_free(binop->lhs);
            ast_free(binop->rhs);
        } break;

        case AST_UNOP: {
            AST_NodeUnOp *unop = (void*)root;
            ast_free(unop->expr);
        } break;

        case AST_NAME: {
            AST_NodeName *name_node = (void*)root;
            
            free(name_node->name);
        } break;

        case AST_CALL: {
            AST_NodeCall *call = (void*)root;
            free(call->name);
            args_free(call->args);
        } break;
            
        case AST_FUNC: {
            AST_NodeFunc *func_node = (void*)root;
            
            free(func_node->name);
        } break;

        case AST_LET: {
            AST_NodeLet *let_node = (void*)root;
            
            ast_free(let_node->initializer);
            free(let_node->name);
        } break;

        case AST_PROGRAM: {
            AST_NodeProgram *program = (void*)root;

            for (size_t i = 0; i < program->nodes.count; ++i) {
                ast_free(program->nodes.items[i]);
            }
        } break;
            
        default: break;
    }

    free(root);
}

Value alloc_value(ValueType *type) {
    return (Value){
        .type = type,
        .as_int = 0,
    };
}

Output alloc_output(ValueType *type) {
    return (Output){
        .val = alloc_value(type),
        .err = ERROR_NONE,
    };
}

AST_NodeError *alloc_error(ErrorKind err) {
    AST_NodeError *tok = (AST_NodeError*)malloc(sizeof(AST_NodeError));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_ERROR;
    tok->err = err;
    
    return tok;
}

AST_NodeLit *alloc_lit_expr(Value val) {
    AST_NodeLit *tok = (AST_NodeLit*)malloc(sizeof(AST_NodeLit));
    assert(tok != NULL && "Memory allocation failed");
    
    tok->kind = AST_LIT;
    tok->val = val;
    
    return tok;
}

AST_NodeBinOp *alloc_binop_expr(AST_Node *lhs, BinaryOp op, AST_Node *rhs) {
    AST_NodeBinOp *tok = (AST_NodeBinOp*)malloc(sizeof(AST_NodeBinOp));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_BINOP;
    tok->lhs = lhs;
    tok->op = op;
    tok->rhs = rhs;
    
    return tok;
}

AST_NodeUnOp *alloc_unop_expr(AST_Node *expr, UnaryOp op) {
    AST_NodeUnOp *tok = (AST_NodeUnOp*)malloc(sizeof(AST_NodeUnOp));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_UNOP;
    tok->expr = expr;
    tok->op = op;
    
    return tok;
}

AST_NodeName *alloc_name_expr(char *name) {
    AST_NodeName *tok = (AST_NodeName*)malloc(sizeof(AST_NodeName));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_NAME;
    tok->name = name;
    
    return tok;
}

AST_NodeCall *alloc_call_expr(char *name, Args args) {
    AST_NodeCall *tok = (AST_NodeCall*)malloc(sizeof(AST_NodeCall));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_CALL;
    tok->name = name;
    tok->args = args;
    
    return tok;
}

AST_NodeFunc *alloc_func_stmt(char *name, Patterns args, AST_Node *body, ValueType *ret_type, bool constant) {
    AST_NodeFunc *tok = (AST_NodeFunc*)malloc(sizeof(AST_NodeFunc));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_FUNC;
    tok->name = name;
    tok->args = args;
    tok->body = body;
    tok->ret_type = ret_type;
    tok->constant = constant;
    
    return tok;
}

AST_NodeLet *alloc_let_stmt(char *name, AST_Node *initializer, bool has_initializer, bool constant) {
    AST_NodeLet *tok = (AST_NodeLet*)malloc(sizeof(AST_NodeLet));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_LET;
    tok->name = name;
    tok->initializer = initializer;
    tok->has_initializer = has_initializer;
    tok->constant = constant;
    
    return tok;
}

AST_NodeBlock *alloc_block_expr(Nodes nodes, AST_Node *ret_expr) {
    AST_NodeBlock *tok = (AST_NodeBlock*)malloc(sizeof(AST_NodeBlock));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_BLOCK;
    tok->nodes = nodes;
    tok->ret_expr = ret_expr;
    
    return tok;
}

AST_NodeProgram *alloc_program(Nodes nodes) {
    AST_NodeProgram *tok = (AST_NodeProgram*)malloc(sizeof(AST_NodeProgram));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_PROGRAM;
    tok->nodes = nodes;
    
    return tok;
}

ValueTypeArray *alloc_arr_type(ValueType *el_type) {
    ValueTypeArray *arr_type = (ValueTypeArray*)malloc(sizeof(ValueTypeArray));
    arr_type->kind = TYPE_ARRAY;
    arr_type->el_type = el_type;
    return arr_type;
}

bool skip(Lexer *l, TokenKind kind) {
    if (l->cur.kind != kind) {
        return false;
    }

    while (l->cur.kind == kind) {
        lexer_next(l);
    }

    return true;
}

BinaryOp get_binop(Token tok) {
    assert(is_binop(tok) && "Expected binary operation");

    switch (tok.kind) {
        case TOKEN_PLUS: return BINOP_PLUS;
        case TOKEN_MINUS: return BINOP_MINUS;
        case TOKEN_STAR: return BINOP_MUL;
        case TOKEN_SLASH: return BINOP_DIV;
        case TOKEN_PERCENT: return BINOP_MOD;
        case TOKEN_CARET: return BINOP_POW;
        case TOKEN_EQ: return BINOP_EQ;
        case TOKEN_GT: return BINOP_GT;
        case TOKEN_LT: return BINOP_LT;
        case TOKEN_GTEQ: return BINOP_GTEQ;
        case TOKEN_LTEQ: return BINOP_LTEQ;
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

UnaryOp get_unop(Token tok) {
    assert(is_unop(tok) && "Expected unary operation");
    
    switch (tok.kind) {
        case TOKEN_PLUS: return UNOP_PLUS;
        case TOKEN_MINUS: return UNOP_MINUS;
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

Precedence get_precedence(Token tok) {
    Precedence prec = {0};

    if (is_binop(tok)) {
        BinaryOp binop = get_binop(tok);
        
        switch (binop) {
            case BINOP_EQ: {
                prec.val = 0;
                prec.right_associative = true;
            } break;
            
            case BINOP_PLUS:
            case BINOP_MINUS:{
                prec.val = 1;
                prec.right_associative = false;
            } break;

            case BINOP_MUL:
            case BINOP_DIV:
            case BINOP_MOD: {
                prec.val = 2;
                prec.right_associative = false;
            } break;
            
            case BINOP_POW: {
                prec.val = 4;
                prec.right_associative = true;
            } break;

            default: break;
        }
    }
    else if (is_unop(tok)) {
        UnaryOp unop = get_unop(tok);
        
        switch (unop) {
            case UNOP_PLUS:
            case UNOP_MINUS:{
                prec.val = 3;
                prec.right_associative = false;
            } break;

            default: break;
        }
    }

    return prec;
}

AST_Node *parse_unop(Lexer *l) {
    assert(is_unop(l->cur) && "Expected unary operation");

    UnaryOp op = get_unop(l->cur);
    Precedence prec = get_precedence(l->cur);
    
    lexer_next(l);
    
    AST_Node *expr = parse_expr(l, prec.val + prec.right_associative);
    return (void*)alloc_unop_expr(expr, op);
}

AST_Node *parse_num(Lexer *l) {
    assert(l->cur.kind == TOKEN_INT || l->cur.kind == TOKEN_FLOAT);

    Output output = alloc_output(VOID_TYPE);
    
    errno = 0;
    
    char *end;
    double num = strtod(l->cur.val, &end);

    if (l->cur.val == end) {
        return (void*)alloc_error(ERROR_WRONG_NUMBER_FORMAT);
    }
    if (errno == ERANGE) {
        return (void*)alloc_error(ERROR_VALUE_OUT_OF_RANGE);
    }

    if (l->cur.kind == TOKEN_INT) {
        output.val.as_int = (INT_CTYPE)num;
        output.val.type = INT_TYPE;
    }
    else {
        output.val.as_float = num;
        output.val.type = FLOAT_TYPE;
    }
    
    lexer_next(l);
    
    if (output.err != ERROR_NONE) {
        return (void*)alloc_error(output.err);
    }
    
    return (void*)alloc_lit_expr(output.val);
}

AST_Node *parse_bool(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_TRUE || l->cur.kind == TOKEN_KW_FALSE);

    Value val = alloc_value(BOOL_TYPE);
    val.as_int = (l->cur.kind == TOKEN_KW_TRUE);
    
    lexer_next(l);

    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_str(Lexer *l) {
    assert(l->cur.kind == TOKEN_STR);

    Value val = alloc_value(STR_TYPE);
    val.as_ptr = strdup(l->cur.val);
    
    lexer_next(l);

    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_char(Lexer *l) {
    assert(l->cur.kind == TOKEN_CHAR);

    if (strlen(l->cur.val) == 0) {
        lexer_next(l);
        return (void*)alloc_error(ERROR_EMPTY_CHAR_LIT);
    }
    
    if (strlen(l->cur.val) > 1) {
        lexer_next(l);
        return (void*)alloc_error(ERROR_MULTI_CHARACTER_CHAR_LIT);
    }
    
    Value val = alloc_value(CHAR_TYPE);
    val.as_int = l->cur.val[0];
    
    lexer_next(l);

    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_lit_expr(Lexer *l) {
    switch (l->cur.kind) {
        case TOKEN_INT:
        case TOKEN_FLOAT: {
            return parse_num(l);
        }

        case TOKEN_STR: {
            return parse_str(l);
        }

        case TOKEN_CHAR: {
            return parse_char(l);
        }

        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE: {
            return parse_bool(l);
        }
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

AST_Node *parse_name_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_NAME);
    AST_Node *name = (void*)alloc_name_expr(strdup(l->cur.val));
    lexer_next(l);
    return name;
}

AST_Node *parse_block_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_LBRACE);
    lexer_next(l);

    Nodes nodes = {0};
    AST_Node *ret_expr = NULL;

    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            da_free(nodes);
            return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        }

        AST_Node *node = parse(l);
        if (!skip(l, TOKEN_SEMICOLON)) {
            ret_expr = node;
            if (l->cur.kind != TOKEN_RBRACE) {
                ast_free(node);
                return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
            }

            break;
        }
        
        da_append(&nodes, node);
    }

    lexer_next(l);
    
    return (void*)alloc_block_expr(nodes, ret_expr);
}

AST_Node *parse_prim_expr(Lexer *l) {
    switch (l->cur.kind) {
        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE:
        case TOKEN_CHAR:
        case TOKEN_STR:
        case TOKEN_INT:
        case TOKEN_FLOAT:{
            return parse_lit_expr(l);
        }

        case TOKEN_NAME: {
            return parse_name_expr(l);
        }
            
        case TOKEN_LPAREN: {
            lexer_next(l);
            AST_Node *node = parse_expr(l, 0);

            if (l->cur.kind != TOKEN_RPAREN) {
                return (void*)alloc_error(ERROR_UNMATCHED_PAREN);
            }

            lexer_next(l);
            return node;
        }

        case TOKEN_LBRACE: {
            return parse_block_expr(l);
        }
            
        default: {
            lexer_next(l);
            return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        }
    }
}

AST_Node *parse_prefix_expr(Lexer *l) {
    if (is_unop(l->cur)) {
        return parse_unop(l);
    }

    return parse_prim_expr(l);
}

ErrorKind parse_args(Lexer *l, Args *args) {
    Args parsed = {0};
    
    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            args_free(parsed);
            return ERROR_UNMATCHED_PAREN;
        }
        
        AST_Node *expr = parse_expr(l, 0);

        Arg arg = {
            .node = expr,
            .has_name = false,
        };
        
        if (expr->kind == AST_NAME && l->cur.kind == TOKEN_COLON) {
            lexer_next(l);
            
            arg.node = parse_expr(l, 0);
            if (arg.node->kind == AST_ERROR) {
                ErrorKind err = ((AST_NodeError*)arg.node)->err;
                ast_free(arg.node);
                args_free(parsed);
                return err;
            }
            
            arg.name = ((AST_NodeName*)expr)->name;
            arg.has_name = true;
        }
        
        da_append(&parsed, arg);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    lexer_next(l);

    *args = parsed;
    return ERROR_NONE;
}

AST_Node *parse_call_expr(Lexer *l, AST_Node *node) {
    assert(l->cur.kind == TOKEN_LPAREN);
    lexer_next(l);
            
    if (node->kind != AST_NAME) {
        ast_free(node);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    char *name = ((AST_NodeName*)node)->name;

    Args args;
    ErrorKind err = parse_args(l, &args);
    if (err != ERROR_NONE) {
        ast_free(node);
        return (void*)alloc_error(err);
    }
            
    AST_Node *call = (void*)alloc_call_expr(strdup(name), args);
    ast_free(node);
    return call;
}

AST_Node *parse_postfix_expr(Lexer *l, AST_Node *node) {
    switch (l->cur.kind) {
        case TOKEN_LPAREN: {
            node = parse_call_expr(l, node);
        } break;

        default: break;
    }

    return node;
}

AST_Node *parse_expr(Lexer *l, size_t min_prec) {
    AST_Node *lhs = parse_prefix_expr(l);
    lhs = parse_postfix_expr(l, lhs);

    while (is_binop(l->cur)) {
        Precedence prec = get_precedence(l->cur);
        if (prec.val < min_prec) break;

        Token binop_tok = l->cur;
        lexer_next(l);

        AST_Node *rhs = parse_expr(l, prec.right_associative? prec.val : prec.val + 1);
        lhs = (void*)alloc_binop_expr(lhs, get_binop(binop_tok), rhs);
    }
    
    return lhs;
}

ErrorKind parse_basic_type(Lexer *l, ValueType **type) {
    if (l->cur.kind != TOKEN_NAME) {
        return ERROR_UNEXPECTED_TOKEN;
    }

    char *type_name = l->cur.val;
    size_t type_len = strlen(type_name);

    bool parsed = false;
    
    if (strncmp("int", type_name, type_len)) {
        *type = INT_TYPE;
        parsed = true;
    }

    if (strncmp("float", type_name, type_len)) {
        *type = FLOAT_TYPE;
        parsed = true;
    }

    if (strncmp("str", type_name, type_len)) {
        *type = STR_TYPE;
        parsed = true;
    }

    if (strncmp("any", type_name, type_len)) {
        *type = ANY_TYPE;
        parsed = true;
    }
    
    lexer_next(l);
    return parsed? ERROR_NONE : ERROR_UNEXPECTED_TOKEN;
}

ErrorKind parse_array_type(Lexer *l, ValueType **type) {
    if (l->cur.kind != TOKEN_LBRACK) {
        return ERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    ValueType *el_type = ANY_TYPE; 
    ErrorKind err = parse_type(l, &el_type);
    if (err != ERROR_NONE) {
        return err;
    }

    if (l->cur.kind != TOKEN_RBRACK) {
        return ERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    *type = (void*)alloc_arr_type(el_type);
    
    return ERROR_NONE;
}

ErrorKind parse_type(Lexer *l, ValueType **type) {
    switch (l->cur.kind) {
        case TOKEN_NAME: {
            return parse_basic_type(l, type);
        }
            
        case TOKEN_LBRACK: {
            return parse_array_type(l, type);
        }

        default: {
            lexer_next(l);
            return ERROR_UNEXPECTED_TOKEN;
        }
    }
}

ErrorKind parse_patterns(Lexer *l, Patterns *args) {
    assert(l->cur.kind == TOKEN_LPAREN);

    lexer_next(l);
    
    Patterns parsed = {0};
    
    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            patterns_free(parsed);
            return ERROR_UNMATCHED_PAREN;
        }

        if (l->cur.kind == TOKEN_ELLIPSIS) {
            Pattern pattern = (Pattern){
                .name = strdup(l->cur.val),
                .type = VARIADIC_TYPE,
            };
        
            da_append(&parsed, pattern);
            
            lexer_next(l);
            if (l->cur.kind == TOKEN_COMMA) {
                lexer_next(l);
            }
            continue;
        }
        
        if (l->cur.kind != TOKEN_NAME) {
            patterns_free(parsed);
            return ERROR_UNEXPECTED_TOKEN;
        }
        
        char *name = strdup(l->cur.val);

        lexer_next(l);
        
        ValueType *type = ANY_TYPE;
        if (l->cur.kind == TOKEN_COLON) {
            lexer_next(l);
            
            ErrorKind err = parse_type(l, &type);
            if (err != ERROR_NONE) {
                free(name);
                patterns_free(parsed);
                return err;
            }
        }

        Pattern pattern = (Pattern){
            .name = name,
            .type = type,
        };
        
        da_append(&parsed, pattern);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    lexer_next(l);

    *args = parsed;
    return ERROR_NONE;
}

AST_Node *parse_func_stmt(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_FN);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }
    
    char *name = strdup(l->cur.val);

    lexer_next(l);

    if (l->cur.kind != TOKEN_LPAREN) {
        free(name);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }
    
    Patterns args;
    ErrorKind err = parse_patterns(l, &args);
    if (err != ERROR_NONE) {
        free(name);
        return (void*)alloc_error(err);
    }

    ValueType *ret_type = ANY_TYPE;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        err = parse_type(l, &ret_type);
        if (err != ERROR_NONE) {
            free(name);
            return (void*)alloc_error(err);
        }
    }

    AST_Node *body = NULL;
    if (l->cur.kind == TOKEN_ARROW) {
        lexer_next(l);
        body = parse(l);
    }
    else if (l->cur.kind == TOKEN_LBRACE) {
        body = parse_block_expr(l);
    }
    else {
        free(name);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    if (body->kind == AST_ERROR){
        free(name);
        return body;
    }

    AST_Node *func = (void*)alloc_func_stmt(name, args, body, ret_type, constant);
    return func;
}

AST_Node *parse_let_stmt(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_LET);
    
    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    char *name = strdup(l->cur.val);
    
    if (l->cur.kind == TOKEN_SEMICOLON) {
        AST_NodeLet *let_node = alloc_let_stmt(name, NULL, false, constant);
        return (void*)let_node;
    }

    lexer_next(l);
    
    if (l->cur.kind != TOKEN_EQ) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);
    
    AST_Node *initializer = parse_expr(l, 0);
    if (initializer->kind == AST_ERROR) {
        ErrorKind err = ((AST_NodeError*)initializer)->err;
        ast_free(initializer);
        return (void*)alloc_error(err);
    }

    AST_NodeLet *let_node = alloc_let_stmt(name, initializer, true, constant);
    return (void*)let_node;
}

AST_Node *parse_const(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_FN: {
            node = parse_func_stmt(l, true);
        } break;

        case TOKEN_KW_LET: {
            node = parse_let_stmt(l, true);
        } break;
            
        default: {
            node = (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        } break;
    }

    return node;
}

AST_Node *parse_stmt(Lexer *l) {
    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_FN: {
            node = parse_func_stmt(l, false);
        } break;

        case TOKEN_KW_LET: {
            node = parse_let_stmt(l, false);
        } break;
            
        case TOKEN_KW_CONST: {
            node = parse_const(l);
        } break;
            
        default: {
            node = (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        } break;
    }
    
    return node;
}

AST_Node *parse(Lexer *l) {
    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_LET:
        case TOKEN_KW_CONST:
        case TOKEN_KW_FN: {
            node = parse_stmt(l);
        } break;
            
        default: {
            node = parse_expr(l, 0);
        } break;
    }
    
    return node;
}

AST_Node *parse_item(Lexer *l) {
    AST_Node *node = NULL;
    
    node = parse(l);
    skip(l, TOKEN_SEMICOLON);
    
    return node;
}

AST_NodeProgram *parse_program(Lexer *l) {
    Nodes nodes = {0};

    while (l->cur.kind != TOKEN_EOF) {
        da_append(&nodes, parse_item(l));
    }
    
    AST_NodeProgram *program = alloc_program(nodes);
    return program;
}