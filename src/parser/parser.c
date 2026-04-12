#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "parser.h"
#include "nothing/nothing.h"

void nodes_free(Nodes nodes) {
    for (size_t i = 0; i < nodes.count; ++i) {
        ast_free(nodes.items[i]);
    }

    da_free(nodes);
}

void pattern_free(Pattern pattern) {
    da_pfree(pattern.name);
    free(pattern.name);
}

void patterns_free(Patterns patterns) {
    for (size_t i = 0; i < patterns.count; ++i) {
        pattern_free(patterns.items[i]);
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

            if (let_node->initializer != NULL) {
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

AST_NodeError *alloc_error_node(ErrorKind err) {
    AST_NodeError *err_node = (AST_NodeError*)malloc(sizeof(AST_NodeError));
    assert(err_node != NULL && "Memory allocation failed");

    err_node->kind = AST_ERROR;
    err_node->err = err;
    
    return err_node;
}

AST_NodeLit *alloc_lit_node(Value val) {
    AST_NodeLit *lit_node = (AST_NodeLit*)malloc(sizeof(AST_NodeLit));
    assert(lit_node != NULL && "Memory allocation failed");
    
    lit_node->kind = AST_LIT;
    lit_node->val = val;
    
    return lit_node;
}

AST_NodeBinOp *alloc_binop_node(AST_Node *lhs, BinaryOp op, AST_Node *rhs) {
    AST_NodeBinOp *binop_node = (AST_NodeBinOp*)malloc(sizeof(AST_NodeBinOp));
    assert(binop_node != NULL && "Memory allocation failed");

    binop_node->kind = AST_BINOP;
    binop_node->lhs = lhs;
    binop_node->op = op;
    binop_node->rhs = rhs;
    
    return binop_node;
}

AST_NodeUnOp *alloc_unop_node(AST_Node *expr, UnaryOp op) {
    AST_NodeUnOp *unop_node = (AST_NodeUnOp*)malloc(sizeof(AST_NodeUnOp));
    assert(unop_node != NULL && "Memory allocation failed");

    unop_node->kind = AST_UNOP;
    unop_node->expr = expr;
    unop_node->op = op;
    
    return unop_node;
}

AST_NodeName *alloc_name_node(String_View sb) {
    AST_NodeName *name_node = (AST_NodeName*)malloc(sizeof(AST_NodeName));
    assert(name_node != NULL && "Memory allocation failed");

    name_node->kind = AST_NAME;
    name_node->name = sb;
    
    return name_node;
}

AST_NodeCall *alloc_call_node(String_View name, Args args) {
    AST_NodeCall *call_node = (AST_NodeCall*)malloc(sizeof(AST_NodeCall));
    assert(call_node != NULL && "Memory allocation failed");

    call_node->kind = AST_CALL;
    call_node->name = name;
    call_node->args = args;
    
    return call_node;
}

AST_NodeIndex *alloc_index_node(AST_Node *node, AST_Node *index) {
    AST_NodeIndex *index_node = (AST_NodeIndex*)malloc(sizeof(AST_NodeIndex));
    assert(index_node != NULL && "Memory allocation failed");

    index_node->kind = AST_INDEX;
    index_node->node = node;
    index_node->index = index;
    
    return index_node;
}

AST_NodeFuncDecl *alloc_func_node(String_View name, Patterns args, AST_Node *body, ValueType *ret_type, bool constant) {
    AST_NodeFuncDecl *func_node = (AST_NodeFuncDecl*)malloc(sizeof(AST_NodeFuncDecl));
    assert(func_node != NULL && "Memory allocation failed");

    func_node->kind = AST_FN;
    func_node->name = name;
    func_node->args = args;
    func_node->body = body;
    func_node->ret_type = ret_type;
    func_node->constant = constant;
    
    return func_node;
}

AST_NodeStructDecl *alloc_struct_node(String_View name, Patterns fields, Funcs funcs, bool constant) {
    AST_NodeStructDecl *struct_node = (AST_NodeStructDecl*)malloc(sizeof(AST_NodeStructDecl));
    assert(struct_node != NULL && "Memory allocation failed");

    struct_node->kind = AST_FN;
    struct_node->name = name;
    struct_node->fields = fields;
    struct_node->funcs = funcs;
    struct_node->constant = constant;
    
    return struct_node;
}

AST_NodeLetStmt *alloc_let_node(String_View name, ValueType *type, AST_Node *initializer, bool constant) {
    AST_NodeLetStmt *let_node = (AST_NodeLetStmt*)malloc(sizeof(AST_NodeLetStmt));
    assert(let_node != NULL && "Memory allocation failed");

    let_node->kind = AST_LET;
    let_node->name = name;
    let_node->type = type;
    let_node->initializer = initializer;
    let_node->constant = constant;
    
    return let_node;
}

AST_NodeForStmt *alloc_for_node(AST_Node *initializer, AST_Node *condition, AST_Node *next, AST_Node *body) {
    AST_NodeForStmt *for_node = (AST_NodeForStmt*)malloc(sizeof(AST_NodeForStmt));
    assert(for_node != NULL && "Memory allocation failed");

    for_node->kind = AST_FOR;
    for_node->initializer = initializer;
    for_node->condition = condition;
    for_node->next = next;
    for_node->body = body;
    
    return for_node;
}

AST_NodeArray *alloc_arr_node(Nodes nodes) {
    AST_NodeArray *arr_node = (AST_NodeArray*)malloc(sizeof(AST_NodeArray));
    assert(arr_node != NULL && "Memory allocation failed");

    arr_node->kind = AST_ARR;
    arr_node->nodes = nodes;
    
    return arr_node;
}

AST_NodeBlock *alloc_block_node(Nodes nodes, AST_Node *ret_expr) {
    AST_NodeBlock *block_node = (AST_NodeBlock*)malloc(sizeof(AST_NodeBlock));
    assert(block_node != NULL && "Memory allocation failed");

    block_node->kind = AST_BLOCK;
    block_node->nodes = nodes;
    block_node->ret_expr = ret_expr;
    
    return block_node;
}

AST_NodeBranch *alloc_branch_node(AST_Node *condition, AST_Node *body, Nodes elif_branches, AST_Node *else_branch) {
    AST_NodeBranch *branch_node = (AST_NodeBranch*)malloc(sizeof(AST_NodeBranch));
    assert(branch_node != NULL && "Memory allocation failed");

    branch_node->kind = AST_IF;
    branch_node->condition = condition;
    branch_node->body = body;
    branch_node->elif_branches = elif_branches;
    branch_node->else_branch = else_branch;
    
    return branch_node;
}

AST_NodeProgram *alloc_program_node(Nodes nodes) {
    AST_NodeProgram *program_node = (AST_NodeProgram*)malloc(sizeof(AST_NodeProgram));
    assert(program_node != NULL && "Memory allocation failed");

    program_node->kind = AST_PROGRAM;
    program_node->nodes = nodes;
    
    return program_node;
}

AST_NodeContinue *alloc_continue_signal() {
    AST_NodeContinue *sig = (AST_NodeContinue*)malloc(sizeof(AST_NodeContinue));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_CONTINUE;
    
    return sig;
}

AST_NodeBreak *alloc_break_signal() {
    AST_NodeBreak *sig = (AST_NodeBreak*)malloc(sizeof(AST_NodeBreak));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_BREAK;
    
    return sig;
}

AST_NodeReturn *alloc_return_signal(AST_Node *node) {
    AST_NodeReturn *sig = (AST_NodeReturn*)malloc(sizeof(AST_NodeReturn));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_RETURN;
    sig->node = node;
    
    return sig;
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
        case TOKEN_ASIGN: return BINOP_ASSIGN;
        case TOKEN_ANDAND: return BINOP_LOGIC_AND;
        case TOKEN_AND: return BINOP_BIT_AND;
        case TOKEN_OROR: return BINOP_LOGIC_OR;
        case TOKEN_OR: return BINOP_BIT_OR;
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
        case TOKEN_PLUSPLUS: return UNOP_INCREMENT;
        case TOKEN_MINUSMINUS: return UNOP_DECREMENT;
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

bool is_prim_expr_start(Token tok) {
    switch (tok.kind) {
        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE:
        case TOKEN_CHAR:
        case TOKEN_STR:
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_NAME:
        case TOKEN_LPAREN:
        case TOKEN_LBRACK:
        case TOKEN_LBRACE: 
        case TOKEN_KW_IF: return true;

        default: return false;
    }
}

bool is_unary_expr_start(Token tok) {
    return is_unop(tok);
}

Precedence get_precedence(Token tok) {
    Precedence prec = {0};

    if (is_binop(tok)) {
        BinaryOp binop = get_binop(tok);
        
        switch (binop) {
            case BINOP_ASSIGN: {
                prec.val = 0;
                prec.right_associative = true;
            } break;

            case BINOP_LOGIC_AND:
            case BINOP_LOGIC_OR:{
                prec.val = 1;
                prec.right_associative = false;
            } break;

            case BINOP_BIT_AND:
            case BINOP_BIT_OR:{
                prec.val = 2;
                prec.right_associative = false;
            } break;

            case BINOP_EQ:
            case BINOP_NEQ:
            case BINOP_GT:
            case BINOP_LT:
            case BINOP_GTEQ:
            case BINOP_LTEQ:{
                prec.val = 3;
                prec.right_associative = false;
            } break;
                
            case BINOP_PLUS:
            case BINOP_MINUS:{
                prec.val = 4;
                prec.right_associative = false;
            } break;

            case BINOP_MUL:
            case BINOP_DIV:
            case BINOP_MOD: {
                prec.val = 5;
                prec.right_associative = false;
            } break;
            
            case BINOP_POW: {
                prec.val = 7;
                prec.right_associative = true;
            } break;

            default: break;
        }
    }
    else if (is_unop(tok)) {
        UnaryOp unop = get_unop(tok);
        
        switch (unop) {
            case UNOP_DECREMENT:
            case UNOP_INCREMENT:
            case UNOP_NOT:
            case UNOP_PLUS:
            case UNOP_MINUS:{
                prec.val = 6;
                prec.right_associative = false;
            } break;
                
            default: break;
        }
    }

    return prec;
}

AST_Node *parse_unop(Lexer *l) {
    if (!is_unary_expr_start(l->cur)) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    UnaryOp op = get_unop(l->cur);
    Precedence prec = get_precedence(l->cur);
    
    lexer_next(l);
    
    AST_Node *expr = parse_expr(l, prec.val + prec.right_associative);
    return (void*)alloc_unop_node(expr, op);
}

AST_Node *parse_num(Lexer *l) {
    assert(l->cur.kind == TOKEN_INT || l->cur.kind == TOKEN_FLOAT);

    Value val;
    
    if (l->cur.kind == TOKEN_FLOAT) {
        FLOAT_CTYPE num_float;
        if (!parse_float_sv(l->cur.val, &num_float)) {
            lexer_next(l);
            return (void*)alloc_error_node(ERROR_WRONG_NUMBER_FORMAT);
        }

        val.as_float = num_float;
        val.type = FLOAT_TYPE;
    }
    
    if (l->cur.kind == TOKEN_INT) {
        INT_CTYPE num_int;
        if (!parse_int_sv(l->cur.val, &num_int)) {
            lexer_next(l);
            return (void*)alloc_error_node(ERROR_WRONG_NUMBER_FORMAT);
        }

        val.as_int = num_int;
        val.type = INT_TYPE;
    }
    
    lexer_next(l);
    return (void*)alloc_lit_node(val);
}

AST_Node *parse_bool(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_TRUE || l->cur.kind == TOKEN_KW_FALSE);

    Value val = create_value(BOOL_TYPE);
    val.as_int = l->cur.kind == TOKEN_KW_TRUE;
    
    lexer_next(l);

    return (void*)alloc_lit_node(val);
}

AST_Node *parse_str(Lexer *l) {
    assert(l->cur.kind == TOKEN_STR);

    Value val = create_value(STR_TYPE);
    
    String_Builder *sb = sb_alloc();
    sv_to_escaped_sb(sb, &l->cur.val);
    val.as_ptr = sb;
    
    lexer_next(l);

    return (void*)alloc_lit_node(val);
}

AST_Node *parse_char(Lexer *l) {
    assert(l->cur.kind == TOKEN_CHAR);

    if (l->cur.val.count == 0) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_EMPTY_CHAR_LIT);
    }

    int c = l->cur.val.items[0];
    if (l->cur.val.count == 2) {
        c = escape(l->cur.val.items[1]);
    }
    
    if (l->cur.val.count > 1 && c < 0) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_MULTI_CHARACTER_CHAR_LIT);
    }
    
    Value val = create_value(CHAR_TYPE);
    val.as_int = c;
    
    lexer_next(l);

    return (void*)alloc_lit_node(val);
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
    AST_Node *name = (void*)alloc_name_node(l->cur.val);
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
            return (void*)alloc_error_node(ERROR_UNEXPECTED_EOF);
        }

        AST_Node *el = parse_expr(l, 0);
        if (el->kind == AST_ERROR) {
            da_free(els);
            return el;
        }
        
        if (l->cur.kind != TOKEN_COMMA && l->cur.kind != TOKEN_RBRACK) {
            ast_free(el);
            da_free(els);
            lexer_next(l);
            return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
            return (void*)(ERROR_UNEXPECTED_EOF);
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
    return (void*)alloc_block_node(nodes, ret_expr);
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
    
    return (void*)alloc_branch_node(condition, body, elif_branches, NULL);
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
        
    return (void*)alloc_branch_node(condition, body, elif_branches, else_branch);
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
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
    
    return (void*)alloc_for_node(initializer, condition, next, body);
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
    
    return (void*)alloc_for_node(NULL, condition, NULL, body);
}

AST_Node *parse_forever_stmt(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_FOREVER);

    lexer_next(l);
    
    AST_Node *body = parse_block_expr(l);
    if (body->kind == AST_ERROR) {
        return body;
    }
    
    return (void*)alloc_for_node(NULL, NULL, NULL, body);
}

AST_Node *parse_continue_signal(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_CONTINUE);

    lexer_next(l);
    
    return (void*)alloc_continue_signal();
}

AST_Node *parse_break_signal(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_BREAK);

    lexer_next(l);
    
    return (void*)alloc_break_signal();
}

AST_Node *parse_return_signal(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_RETURN);

    lexer_next(l);

    AST_Node *node = NULL;
    if (is_prim_expr_start(l->cur) || is_unary_expr_start(l->cur)) {
        node = parse_expr(l, 0);
    }
    
    return (void*)alloc_return_signal(node);
}

AST_Node *parse_prim_expr(Lexer *l) {
    if (!is_prim_expr_start(l->cur)) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }
    
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
                return (void*)alloc_error_node(ERROR_UNMATCHED_PAREN);
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
            return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
        return (void*)alloc_error_node(err);
    }
            
    AST_Node *call = (void*)alloc_call_node(name, args);
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
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);

    AST_Node *index_node = (void*)alloc_index_node(node, index);

    if (l->cur.kind == TOKEN_LBRACK) {
        index_node = parse_index_expr(l, index_node);
    }
    
    return index_node;
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
        lhs = (void*)alloc_binop_node(lhs, get_binop(binop_tok), rhs);
    }
    
    return lhs;
}

ErrorKind parse_basic_type(Lexer *l, ValueType **type) {
    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
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
        lexer_next(l);
        return ERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    ValueType *el_type = ANY_TYPE; 
    ErrorKind err = parse_type(l, &el_type);
    if (err != ERROR_NONE) {
        return err;
    }

    if (l->cur.kind != TOKEN_RBRACK) {
        lexer_next(l);
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

ErrorKind parse_pattern(Lexer *l, Pattern *pattern, bool constant) {
    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return ERROR_UNEXPECTED_TOKEN;
    }

    String_View name_sv = l->cur.val;

    lexer_next(l);

    if (l->cur.kind != TOKEN_COLON) {
        lexer_next(l);
        return ERROR_UNEXPECTED_TOKEN;
    }
    
    lexer_next(l);

    if (l->cur.kind == TOKEN_ELLIPSIS) {
        String_Builder *sb = sb_alloc();
        sv_to_sb(&name_sv, sb);
        
        *pattern = (Pattern){
            .name = sb,
            .type = VARIADIC_TYPE,
            .constant = constant,
        };
        
        return ERROR_NONE;
    }

    ValueType *type;
    ErrorKind err = parse_type(l, &type);
    if (err != ERROR_NONE) {
        return err;
    }

    String_Builder *sb = sb_alloc();
    sv_to_sb(&name_sv, sb);
        
    *pattern = (Pattern){
        .name = sb,
        .type = type,
        .constant = constant,
    };

    return ERROR_NONE;
}

ErrorKind parse_patterns(Lexer *l, Patterns *patterns) {
    assert(l->cur.kind == TOKEN_LPAREN);

    lexer_next(l);
    
    Patterns parsed = {0};
    bool has_variadic = false;
    
    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            patterns_free(parsed);
            return ERROR_UNEXPECTED_EOF;
        }

        bool constant = false;
        if (l->cur.kind == TOKEN_KW_CONST) {
            constant = true;
            lexer_next(l);
        }
        
        Pattern pattern;
        ErrorKind err = parse_pattern(l, &pattern, constant);
        if (err != ERROR_NONE) {
            pattern_free(pattern);
            patterns_free(parsed);
            return err;
        }

        if (has_variadic) {
            patterns_free(parsed);
            return ERROR_ARGS_AFTER_VA_ARG;
        }

        if (pattern.type->tag == TYPE_VARIADIC) {
            has_variadic = true;
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

    *patterns = parsed;
    return ERROR_NONE;
}

AST_Node *parse_func_item(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_FUNC);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }
    
    String_View name = l->cur.val;

    lexer_next(l);

    if (l->cur.kind != TOKEN_LPAREN) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }
    
    Patterns args;
    ErrorKind err = parse_patterns(l, &args);
    if (err != ERROR_NONE) {
        return (void*)alloc_error_node(err);
    }

    ValueType *ret_type = ANY_TYPE;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        err = parse_type(l, &ret_type);
        if (err != ERROR_NONE) {
            return (void*)alloc_error_node(err);
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
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    if (body->kind == AST_ERROR){
        return body;
    }

    AST_Node *func = (void*)alloc_func_node(name, args, body, ret_type, constant);
    return func;
}

AST_Node *parse_struct_item(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_STRUCT);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    String_View name_sv = l->cur.val;

    lexer_next(l);
    
    if (l->cur.kind != TOKEN_LBRACE) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);
    
    Patterns fields = {0};
    Funcs funcs = {0};
    
    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            return (void*)alloc_error_node(ERROR_UNEXPECTED_EOF);
        }

        bool constant_pattern = l->cur.kind == TOKEN_KW_CONST;
        if (constant_pattern) {
            lexer_next(l);
        }
        
        switch (l->cur.kind) {
            case TOKEN_NAME: {
                Pattern field;
                ErrorKind err = parse_pattern(l, &field, constant_pattern);
                if (err != ERROR_NONE) {
                    return (void*)alloc_error_node(err);
                }

                if (field.type->tag == TYPE_VARIADIC) {
                    pattern_free(field);
                    return (void*)alloc_error_node(UNEXPECTED_TYPE);
                }

                da_append(&fields, field);
            } break;

            case TOKEN_KW_FUNC: {
                AST_Node *func = parse_func_item(l, constant_pattern);
                if (func->kind == AST_ERROR) {
                    return func;
                }

                da_append(&funcs, (void*)func);
            } break;

            default: {
                lexer_next(l);
                return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
            }
        }
    }
    
    if (l->cur.kind != TOKEN_RBRACE) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    lexer_next(l);
    
    return (void*)alloc_struct_node(name_sv, fields, funcs, constant);
}

AST_Node *parse_let_stmt(Lexer *l, bool constant) {
    assert(l->cur.kind == TOKEN_KW_LET || l->cur.kind == TOKEN_NAME);

    if (l->cur.kind == TOKEN_KW_LET) {
        lexer_next(l);
    }

    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
    }

    String_View name = l->cur.val;

    lexer_next(l);

    ValueType *type = ANY_TYPE;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        ErrorKind err = parse_type(l, &type);
        if (err != ERROR_NONE) {
            return (void*)alloc_error_node(err);
        }
    }
    
    if (l->cur.kind != TOKEN_ASIGN) {
        AST_NodeLetStmt *let_node = alloc_let_node(name, type, NULL, constant);
        return (void*)let_node;
    }
    
    lexer_next(l);
    
    AST_Node *initializer = parse_expr(l, 0);
    if (initializer->kind == AST_ERROR) {
        return initializer;
    }

    AST_NodeLetStmt *let_node = alloc_let_node(name, type, initializer, constant);
    return (void*)let_node;
}

AST_Node *parse_const_item(Lexer *l) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_FUNC: {
            node = parse_func_item(l, true);
        } break;

        case TOKEN_KW_STRUCT: {
            node = parse_struct_item(l, true);
        } break;
            
        case TOKEN_NAME: {
            node = parse_let_stmt(l, true);
        } break;
            
        default: {
            lexer_next(l);
            node = (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
            lexer_next(l);
            node = (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
        } break;
    }

    return node;
}

AST_Node *parse_stmt(Lexer *l) {
    AST_Node *node = NULL;
    
    switch (l->cur.kind) {
        case TOKEN_KW_CONST: {
            node = parse_const_stmt(l);
        } break;
        
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

        case TOKEN_KW_CONTINUE: {
            node = parse_continue_signal(l);
        } break;
            
        case TOKEN_KW_BREAK: {
            node = parse_break_signal(l);
        } break;
            
        case TOKEN_KW_RETURN: {
            node = parse_return_signal(l);
        } break;
            
        default: {
            lexer_next(l);
            node = (void*)alloc_error_node(ERROR_UNEXPECTED_TOKEN);
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
        case TOKEN_KW_FOREVER:
        case TOKEN_KW_CONTINUE:
        case TOKEN_KW_BREAK:
        case TOKEN_KW_RETURN:{
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
        case TOKEN_KW_FUNC: {
            node = parse_func_item(l, false);
        } break;

        case TOKEN_KW_STRUCT: {
            node = parse_struct_item(l, false);
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
    
    AST_NodeProgram *program = alloc_program_node(nodes);
    return program;
}