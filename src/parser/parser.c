#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "parser.h"
#include "nothing/nothing.h"

void nodes_free(Nodes nodes) {
    for (size_t i = 0; i < nodes.count; ++i) {
        ast_free(nodes.items[i]);
    }

    da_free(nodes);
}

void patterns_free(Patterns patterns) {
    for (size_t i = 0; i < patterns.count; ++i) {
        free(patterns.items[i].name->items);
    }

    da_free(patterns);
}

void args_free(Args args) {
    for (size_t i = 0; i < args.count; ++i) {
        ast_free(args.items[i].node);
    }
    
    da_free(args);
}

void ast_free(AST_Node *root) {
    if (root == NULL) {
        return;
    }
    
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

        case AST_CALL: {
            AST_NodeCall *call = (void*)root;
            args_free(call->args);
        } break;

        case AST_LET: {
            AST_NodeLetStmt *let_node = (void*)root;

            if (let_node->has_initializer) {
                ast_free(let_node->initializer);
            }
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

Value create_value(ValueType *type) {
    return (Value){
        .type = type,
        .as_ptr = NULL,
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

AST_NodeName *alloc_name_expr(String_View sb) {
    AST_NodeName *tok = (AST_NodeName*)malloc(sizeof(AST_NodeName));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_NAME;
    tok->name = sb;
    
    return tok;
}

AST_NodeCall *alloc_call_expr(String_View name, Args args) {
    AST_NodeCall *tok = (AST_NodeCall*)malloc(sizeof(AST_NodeCall));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_CALL;
    tok->name = name;
    tok->args = args;
    
    return tok;
}

AST_NodeIndex *alloc_index_expr(AST_Node *node, AST_Node *index) {
    AST_NodeIndex *tok = (AST_NodeIndex*)malloc(sizeof(AST_NodeIndex));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_INDEX;
    tok->node = node;
    tok->index = index;
    
    return tok;
}

AST_NodeFnStmt *alloc_fn_stmt(String_View name, Patterns args, AST_Node *body, ValueType *ret_type, bool constant) {
    AST_NodeFnStmt *tok = (AST_NodeFnStmt*)malloc(sizeof(AST_NodeFnStmt));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_FN;
    tok->name = name;
    tok->args = args;
    tok->body = body;
    tok->ret_type = ret_type;
    tok->constant = constant;
    
    return tok;
}

AST_NodeLetStmt *alloc_let_stmt(String_View name, ValueType *type, AST_Node *initializer, bool has_initializer, bool constant) {
    AST_NodeLetStmt *tok = (AST_NodeLetStmt*)malloc(sizeof(AST_NodeLetStmt));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_LET;
    tok->name = name;
    tok->type = type;
    tok->initializer = initializer;
    tok->has_initializer = has_initializer;
    tok->constant = constant;
    
    return tok;
}

AST_NodeForStmt *alloc_for_stmt(AST_Node *initializer, AST_Node *condition, AST_Node *next, AST_Node *body) {
    AST_NodeForStmt *tok = (AST_NodeForStmt*)malloc(sizeof(AST_NodeForStmt));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_FOR;
    tok->initializer = initializer;
    tok->condition = condition;
    tok->next = next;
    tok->body = body;
    
    return tok;
}

AST_NodeArray *alloc_arr_node(Nodes nodes) {
    AST_NodeArray *tok = (AST_NodeArray*)malloc(sizeof(AST_NodeArray));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_ARR;
    tok->nodes = nodes;
    
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

AST_NodeBranch *alloc_branch_expr(AST_Node *condition, AST_Node *body, Nodes elif_branches, AST_Node *else_branch) {
    AST_NodeBranch *tok = (AST_NodeBranch*)malloc(sizeof(AST_NodeBranch));
    assert(tok != NULL && "Memory allocation failed");

    tok->kind = AST_IF;
    tok->condition = condition;
    tok->body = body;
    tok->elif_branches = elif_branches;
    tok->else_branch = else_branch;
    
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
    ValueTypeArray *type = (ValueTypeArray*)malloc(sizeof(ValueTypeArray));
    assert(type != NULL && "Memory allocation failed");

    type->tag = TYPE_ARRAY;
    type->el_type = el_type;
    
    return type;
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
        case TOKEN_ASIGN: return BINOP_ASIGN;
        case TOKEN_GT: return BINOP_GT;
        case TOKEN_LT: return BINOP_LT;
        case TOKEN_GTEQ: return BINOP_GTEQ;
        case TOKEN_LTEQ: return BINOP_LTEQ;
        case TOKEN_EQ: return BINOP_EQ;
        case TOKEN_NEQ: return BINOP_NEQ;
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

UnaryOp get_unop(Token tok) {
    assert(is_unop(tok) && "Expected unary operation");
    
    switch (tok.kind) {
        case TOKEN_NOT: return UNOP_NOT;
        case TOKEN_PLUS: return UNOP_PLUS;
        case TOKEN_MINUS: return UNOP_MINUS;
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

bool is_stmt(AST_Node *node) {
    return node->kind == AST_FN
        || node->kind == AST_LET
        || node->kind == AST_FOR;
}

Precedence get_precedence(Token tok) {
    Precedence prec = {0};

    if (is_binop(tok)) {
        BinaryOp binop = get_binop(tok);
        
        switch (binop) {
            case BINOP_ASIGN: {
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

    Value val = create_value(VOID_TYPE);
    
    errno = 0;
    
    char *end;
    char *cstr = sv_to_cstr(l->cur.val);
    double num = strtod(cstr, &end);

    free(cstr);
    
    if (end == l->cur.val.items) {
        return (void*)alloc_error(ERROR_WRONG_NUMBER_FORMAT);
    }
    if (errno == ERANGE) {
        return (void*)alloc_error(ERROR_VALUE_OUT_OF_RANGE);
    }

    if (l->cur.kind == TOKEN_INT) {
        val.as_int = (INT_CTYPE)num;
        val.type = INT_TYPE;
    }
    else {
        val.as_float = num;
        val.type = FLOAT_TYPE;
    }
    
    lexer_next(l);
    
    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_bool(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_TRUE || l->cur.kind == TOKEN_KW_FALSE);

    Value val = create_value(BOOL_TYPE);
    val.as_int = l->cur.kind == TOKEN_KW_TRUE;
    
    lexer_next(l);

    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_str(Lexer *l) {
    assert(l->cur.kind == TOKEN_STR);

    Value val = create_value(STR_TYPE);
    
    String_Builder *sb = sb_alloc();
    sv_to_escaped_sb(sb, &l->cur.val);
    val.as_ptr = sb;
    
    lexer_next(l);

    return (void*)alloc_lit_expr(val);
}

AST_Node *parse_char(Lexer *l) {
    assert(l->cur.kind == TOKEN_CHAR);

    if (l->cur.val.count == 0) {
        lexer_next(l);
        return (void*)alloc_error(ERROR_EMPTY_CHAR_LIT);
    }

    int c = l->cur.val.items[0];
    if (l->cur.val.count == 2) {
        c = escape(l->cur.val.items[1]);
    }
    
    if (l->cur.val.count > 1 && c < 0) {
        lexer_next(l);
        return (void*)alloc_error(ERROR_MULTI_CHARACTER_CHAR_LIT);
    }
    
    Value val = create_value(CHAR_TYPE);
    val.as_int = c;
    
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
    AST_Node *name = (void*)alloc_name_expr(l->cur.val);
    lexer_next(l);
    return name;
}

AST_Node *parse_arr_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_LBRACK);

    lexer_next(l);

    Nodes els = {0};
    
    while (l->cur.kind != TOKEN_RBRACK) {
        if (l->cur.kind == TOKEN_EOF) {
            da_free(els);
            return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        }

        AST_Node *el = parse_expr(l, 0);
        if (el->kind == AST_ERROR) {
            da_free(els);
            return el;
        }
        
        if (l->cur.kind != TOKEN_COMMA && l->cur.kind != TOKEN_RBRACK) {
            ast_free(el);
            da_free(els);
            return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        }

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
        
        da_append(&els, el);
    }

    lexer_next(l);

    return (void*)alloc_arr_node(els);
}

AST_Node *parse_block_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_LBRACE);
    lexer_next(l);

    Nodes nodes = {0};
    AST_Node *ret_expr = NULL;

    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            if (ret_expr != NULL) {
                ast_free(ret_expr);
            }
            
            nodes_free(nodes);
            return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        }

        AST_Node *node = parse(l);
        if (ret_expr != NULL) {
            da_append(&nodes, ret_expr);
            ret_expr = NULL;
        }
        
        if (!skip(l, TOKEN_SEMICOLON) && !is_stmt(node)) {
            ret_expr = node;
            continue;
        }
        
        da_append(&nodes, node);
    }

    lexer_next(l);
    return (void*)alloc_block_expr(nodes, ret_expr);
}

AST_Node *parse_elif_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_ELIF);

    lexer_next(l);

    AST_Node *condition = parse_expr(l, 0);
    if (condition->kind == AST_ERROR) {
        ast_free(condition);
        return condition;
    }
    
    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        ast_free(body);
        return body;
    }

    Nodes elif_branches = {0};
    
    return (void*)alloc_branch_expr(condition, body, elif_branches, NULL);
}

AST_Node *parse_else_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_ELSE);

    lexer_next(l);
    
    AST_Node *body = parse_block_expr(l);
    return body;
}

AST_Node *parse_if_expr(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_IF);

    lexer_next(l);

    AST_Node *condition = parse_expr(l, 0);
    if (condition->kind == AST_ERROR) {
        return condition;
    }
    
    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        ast_free(condition);
        return body;
    }
    
    Nodes elif_branches = {0};
    while (l->cur.kind == TOKEN_KW_ELIF) {
        AST_Node *elif = parse_elif_expr(l);
        if (elif->kind == AST_ERROR) {
            ast_free(condition);
            ast_free(body);
            nodes_free(elif_branches);
            return elif;
        }

        da_append(&elif_branches, elif);
    }

    AST_Node *else_branch = NULL;
    if (l->cur.kind == TOKEN_KW_ELSE) {
        else_branch = parse_else_expr(l);
        if (else_branch->kind == AST_ERROR) {
            ast_free(condition);
            ast_free(body);
            nodes_free(elif_branches);
            return else_branch;
        }
    }
        
    return (void*)alloc_branch_expr(condition, body, elif_branches, else_branch);
}

AST_Node *parse_for_stmt(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_FOR);

    lexer_next(l);

    AST_Node *initializer = NULL;
    if (l->cur.kind != TOKEN_SEMICOLON) {
        initializer = parse(l);
    }

    if (initializer != NULL && initializer->kind == AST_ERROR) {
        return initializer;
    }
    
    if (l->cur.kind != TOKEN_SEMICOLON) {
        ast_free(initializer);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);
    
    AST_Node *condition = NULL;
    if (l->cur.kind != TOKEN_SEMICOLON) {
        condition = parse(l);
    }

    if (condition != NULL && condition->kind == AST_ERROR) {
        ast_free(initializer);
        return condition;
    }
    
    if (l->cur.kind != TOKEN_SEMICOLON) {
        ast_free(initializer);
        ast_free(condition);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);

    AST_Node *next = NULL;
    if (l->cur.kind != TOKEN_LBRACK) {
        next = parse(l);
    }

    if (next != NULL && next->kind == AST_ERROR) {
        ast_free(initializer);
        ast_free(condition);
        return next;
    }

    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        ast_free(initializer);
        ast_free(condition);
        ast_free(next);
        return body;
    }
    
    return (void*)alloc_for_stmt(initializer, condition, next, body);
}

AST_Node *parse_while_stmt(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_WHILE);

    lexer_next(l);
    
    AST_Node *condition = parse(l);
    if (condition->kind == AST_ERROR) {
        return condition;
    }

    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        ast_free(condition);
        return body;
    }
    
    return (void*)alloc_for_stmt(NULL, condition, NULL, body);
}

AST_Node *parse_forever_stmt(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_FOREVER);

    lexer_next(l);
    
    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        return body;
    }
    
    return (void*)alloc_for_stmt(NULL, NULL, NULL, body);
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

        case TOKEN_LBRACK: {
            return parse_arr_expr(l);
        }
            
        case TOKEN_LBRACE: {
            return parse_block_expr(l);
        }

        case TOKEN_KW_IF: {
            return parse_if_expr(l);
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
        if (expr->kind == AST_ERROR) {
            ErrorKind err = ((AST_NodeError*)expr)->err;
            ast_free(expr);
            args_free(parsed);
            return err;
        }

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
    assert(node->kind == AST_NAME);
    assert(l->cur.kind == TOKEN_LPAREN);
    lexer_next(l);

    String_View name = ((AST_NodeName*)node)->name;

    Args args;
    ErrorKind err = parse_args(l, &args);
    if (err != ERROR_NONE) {
        ast_free(node);
        return (void*)alloc_error(err);
    }
            
    AST_Node *call = (void*)alloc_call_expr(name, args);
    ast_free(node);
    return call;
}

AST_Node *parse_index_expr(Lexer *l, AST_Node *node) {
    assert(l->cur.kind == TOKEN_LBRACK);

    lexer_next(l);

    AST_Node *index = parse_expr(l, 0);

    if (l->cur.kind != TOKEN_RBRACK) {
        ast_free(node);
        ast_free(index);
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);

    return (void*)alloc_index_expr(node, index);
}

AST_Node *parse_postfix_expr(Lexer *l, AST_Node *node) {
    switch (l->cur.kind) {
        case TOKEN_LPAREN: {
            node = parse_call_expr(l, node);
        } break;

        case TOKEN_LBRACK: {
            node = parse_index_expr(l, node);
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

    bool parsed = false;
    
    if (strncmp("int", l->cur.val.items, l->cur.val.count) == 0) {
        *type = INT_TYPE;
        parsed = true;
    }

    if (strncmp("float", l->cur.val.items, l->cur.val.count) == 0) {
        *type = FLOAT_TYPE;
        parsed = true;
    }

    if (strncmp("str", l->cur.val.items, l->cur.val.count) == 0) {
        *type = STR_TYPE;
        parsed = true;
    }

    if (strncmp("any", l->cur.val.items, l->cur.val.count) == 0) {
        *type = ANY_TYPE;
        parsed = true;
    }

    if (strncmp("void", l->cur.val.items, l->cur.val.count) == 0) {
        *type = VOID_TYPE;
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

        case TOKEN_RBRACK: {
            *type = ANY_TYPE;
            return ERROR_NONE;
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
    bool has_variadic = false;
    
    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            patterns_free(parsed);
            return ERROR_UNMATCHED_PAREN;
        }
        
        if (l->cur.kind != TOKEN_NAME) {
            patterns_free(parsed);
            return ERROR_UNEXPECTED_TOKEN;
        }
        
        String_View name_sv = l->cur.val;

        lexer_next(l);
        
        ValueType *type = ANY_TYPE;
        if (l->cur.kind == TOKEN_COLON) {
            lexer_next(l);

            if (l->cur.kind == TOKEN_ELLIPSIS) {
                String_Builder *sb = sb_alloc();
                sv_to_sb(&name_sv, sb);
            
                Pattern pattern = (Pattern){
                    .name = sb,
                    .type = VARIADIC_TYPE,
                };
        
                da_append(&parsed, pattern);
            
                lexer_next(l);
                if (l->cur.kind == TOKEN_COMMA) {
                    lexer_next(l);
                }

                has_variadic = true;
                continue;
            }
            
            ErrorKind err = parse_type(l, &type);
            if (err != ERROR_NONE) {
                patterns_free(parsed);
                return err;
            }
        }

        String_Builder *sb = sb_alloc();
        sv_to_sb(&name_sv, sb);
        
        Pattern pattern = (Pattern){
            .name = sb,
            .type = type,
        };

        if (has_variadic) {
            patterns_free(parsed);
            return ERROR_ARGS_AFTER_VA_ARG;
        }
        
        da_append(&parsed, pattern);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    if (l->cur.kind != TOKEN_RPAREN) {
        patterns_free(parsed);
        return ERROR_ARGS_AFTER_VA_ARG;
    }
    
    lexer_next(l);

    *args = parsed;
    return ERROR_NONE;
}

AST_Node *parse_fn_item(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_FN);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }
    
    String_View name = l->cur.val;

    lexer_next(l);

    if (l->cur.kind != TOKEN_LPAREN) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }
    
    Patterns args;
    ErrorKind err = parse_patterns(l, &args);
    if (err != ERROR_NONE) {
        return (void*)alloc_error(err);
    }

    ValueType *ret_type = ANY_TYPE;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        err = parse_type(l, &ret_type);
        if (err != ERROR_NONE) {
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
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    if (body->kind == AST_ERROR){
        return body;
    }

    AST_Node *func = (void*)alloc_fn_stmt(name, args, body, ret_type, constant);
    return func;
}

AST_Node *parse_let_stmt(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_LET || l->cur.kind == TOKEN_NAME);

    if (l->cur.kind == TOKEN_KW_LET) {
        lexer_next(l);
    }

    if (l->cur.kind != TOKEN_NAME) {
        return (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
    }

    String_View name = l->cur.val;

    lexer_next(l);

    ValueType *type = ANY_TYPE;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        ErrorKind err = parse_type(l, &type);
        if (err != ERROR_NONE) {
            return (void*)alloc_error(err);
        }
    }
    
    if (l->cur.kind != TOKEN_ASIGN) {
        AST_NodeLetStmt *let_node = alloc_let_stmt(name, type, NULL, false, constant);
        return (void*)let_node;
    }
    
    lexer_next(l);
    
    AST_Node *initializer = parse_expr(l, 0);
    if (initializer->kind == AST_ERROR) {
        ErrorKind err = ((AST_NodeError*)initializer)->err;
        ast_free(initializer);
        return (void*)alloc_error(err);
    }

    AST_NodeLetStmt *let_node = alloc_let_stmt(name, type, initializer, true, constant);
    return (void*)let_node;
}

AST_Node *parse_const_item(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_FN: {
            node = parse_fn_item(l, true);
        } break;

        case TOKEN_NAME: {
            node = parse_let_stmt(l, true);
        } break;
            
        default: {
            node = (void*)alloc_error(ERROR_UNEXPECTED_TOKEN);
        } break;
    }

    return node;
}

AST_Node *parse_const_stmt(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_NAME: {
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
        case TOKEN_KW_LET: {
            node = parse_let_stmt(l, false);
        } break;

        case TOKEN_KW_FOR: {
            node = parse_for_stmt(l);
        } break;

        case TOKEN_KW_WHILE: {
            node = parse_while_stmt(l);
        } break;

        case TOKEN_KW_FOREVER: {
            node = parse_forever_stmt(l);
        } break;
            
        case TOKEN_KW_CONST: {
            node = parse_const_stmt(l);
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
        case TOKEN_KW_FOR:
        case TOKEN_KW_WHILE:
        case TOKEN_KW_FOREVER:{
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

    switch (l->cur.kind) {
        case TOKEN_KW_FN: {
            node = parse_fn_item(l, false);
        } break;

        case TOKEN_KW_CONST: {
            node = parse_const_item(l);
        } break;

        default: {
            node = parse(l);
        }
    }
    
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