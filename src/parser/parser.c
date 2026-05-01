#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "parser.h"
#include "nothing/nothing.h"
#include "other/built_in.h"

#include "utf8/utf8.h"

void ast_nodes_free(AST_Nodes nodes) {
    for (size_t i = 0; i < nodes.count; ++i) {
        ast_free(nodes.items[i]);
    }

    da_free(nodes);
}

void ast_pattern_free(AST_Pattern pattern) {
    ast_free((void*)pattern.type);
    ast_free(pattern.initializer);
}

void ast_patterns_free(AST_Patterns patterns) {
    for (size_t i = 0; i < patterns.count; ++i) {
        ast_pattern_free(patterns.items[i]);
    }

    da_free(patterns);
}

void ast_enum_members_free(AST_EnumMembers members) {
    for (size_t i = 0; i < members.count; ++i) {
        ast_free(members.items[i].initializer);
    }

    da_free(members);
}

void ast_args_free(AST_Args args) {
    for (size_t i = 0; i < args.count; ++i) {
        ast_free(args.items[i].node);
    }
    
    da_free(args);
}

void initializers_free(AST_InitializerList *initializer) {
    for (size_t i = 0; i < initializer->count; ++i) {
        ast_free(initializer->items[i].initializer);
    }
    
    da_pfree(initializer);
}

void ast_free(AST_Node *node) {
    if (node == NULL) {
        return;
    }
    
    switch (node->kind) {
        case AST_BINOP: {
            AST_NodeBinOp *binop = (void*)node;
            ast_free(binop->lhs);
            ast_free(binop->rhs);
        } break;

        case AST_UNOP: {
            AST_NodeUnOp *unop = (void*)node;
            ast_free(unop->expr);
        } break;

        case AST_CALL: {
            AST_NodeCall *call = (void*)node;
            ast_args_free(call->args);
        } break;

        case AST_FUNC_DECL: {
            AST_NodeFuncDecl *func = (void*)node;
            ast_patterns_free(func->args);
            ast_free((void*)func->ret_type);
        } break;
            
        case AST_STRUCT_DECL: {
            AST_NodeStructDecl *struc = (void*)node;
            ast_patterns_free(struc->fields);
            ast_nodes_free(struc->methods);
        } break;

        case AST_LET: {
            AST_NodeLetStmt *let_node = (void*)node;

            if (let_node->initializer != NULL) {
                ast_free(let_node->initializer);
            }
        } break;

        case AST_PROGRAM: {
            AST_NodeProgram *program = (void*)node;

            for (size_t i = 0; i < program->nodes.count; ++i) {
                ast_free(program->nodes.items[i]);
            }
        } break;
            
        default: break;
    }

    free(node);
}

AST_Value create_ast_value(AST_LitType type, String_View view) {
    return (AST_Value){
        .type = type,
        .view = view,
    };
}

AST_NodeLit *alloc_lit_node(AST_Value val) {
    AST_NodeLit *lit_node = malloc(sizeof(AST_NodeLit));
    assert(lit_node != NULL && "Memory allocation failed");
    
    lit_node->kind = AST_LIT;
    lit_node->val = val;
    
    return lit_node;
}

AST_NodeBinOp *alloc_binop_node(AST_Node *lhs, BinaryOp op, AST_Node *rhs) {
    AST_NodeBinOp *binop_node = malloc(sizeof(AST_NodeBinOp));
    assert(binop_node != NULL && "Memory allocation failed");

    binop_node->kind = AST_BINOP;
    binop_node->lhs = lhs;
    binop_node->op = op;
    binop_node->rhs = rhs;
    
    return binop_node;
}

AST_NodeUnOp *alloc_unop_node(AST_Node *expr, UnaryOp op) {
    AST_NodeUnOp *unop_node = malloc(sizeof(AST_NodeUnOp));
    assert(unop_node != NULL && "Memory allocation failed");

    unop_node->kind = AST_UNOP;
    unop_node->expr = expr;
    unop_node->op = op;
    
    return unop_node;
}

AST_NodeName *alloc_name_node(String_View sv) {
    AST_NodeName *name_node = malloc(sizeof(AST_NodeName));
    assert(name_node != NULL && "Memory allocation failed");

    name_node->kind = AST_NAME;
    name_node->name = sv;
    
    return name_node;
}

AST_Type *alloc_basic_type_node(String_View sv) {
    AST_Type *type = malloc(sizeof(AST_Type));
    assert(type != NULL && "Memory allocation failed");

    type->kind = AST_TYPE;
    type->name = sv;
    type->el_type = NULL;
    type->provided_name = true;
    type->is_array = false;
    
    return type;
}

AST_NodeCall *alloc_call_node(AST_Node *node, AST_Args args) {
    AST_NodeCall *call_node = malloc(sizeof(AST_NodeCall));
    assert(call_node != NULL && "Memory allocation failed");

    call_node->kind = AST_CALL;
    call_node->to_call = node;
    call_node->args = args;
    
    return call_node;
}

AST_NodeIndex *alloc_index_node(AST_Node *node, AST_Node *index) {
    AST_NodeIndex *index_node = malloc(sizeof(AST_NodeIndex));
    assert(index_node != NULL && "Memory allocation failed");

    index_node->kind = AST_INDEX;
    index_node->node = node;
    index_node->index = index;
    
    return index_node;
}

AST_NodeInit *alloc_struct_expr_node(AST_Node *node, AST_InitializerList initializer) {
    AST_NodeInit *init_node = malloc(sizeof(AST_NodeInit));
    assert(init_node != NULL && "Memory allocation failed");

    init_node->kind = AST_STRUCT;
    init_node->node = node;
    init_node->initializers = initializer;
    
    return init_node;
}

AST_NodeMemberAccess *alloc_member_access_node(AST_Node *node, AST_Node *member) {
    AST_NodeMemberAccess *init_node = malloc(sizeof(AST_NodeMemberAccess));
    assert(init_node != NULL && "Memory allocation failed");

    init_node->kind = AST_MEMBER_ACCESS;
    init_node->base = node;
    init_node->member = member;
    
    return init_node;
}

AST_NodeCast *alloc_cast_node(AST_Node *node, AST_Type *type) {
    AST_NodeCast *cast_node = malloc(sizeof(AST_NodeCast));
    assert(cast_node != NULL && "Memory allocation failed");

    cast_node->kind = AST_CAST;
    cast_node->base = node;
    cast_node->type = type;
    
    return cast_node;
}

AST_NodeFuncDecl *alloc_func_node(String_View name, AST_Patterns args, AST_Node *body, AST_Type *ret_type, bool is_static) {
    AST_NodeFuncDecl *func_node = malloc(sizeof(AST_NodeFuncDecl));
    assert(func_node != NULL && "Memory allocation failed");

    func_node->kind = AST_FUNC_DECL;
    func_node->name = name;
    func_node->args = args;
    func_node->body = body;
    func_node->ret_type = ret_type;
    func_node->is_static = is_static;
    
    return func_node;
}

AST_NodeStructDecl *alloc_struct_node(String_View name, AST_Patterns fields, AST_Nodes funcs) {
    AST_NodeStructDecl *struct_node = malloc(sizeof(AST_NodeStructDecl));
    assert(struct_node != NULL && "Memory allocation failed");

    struct_node->kind = AST_STRUCT_DECL;
    struct_node->name = name;
    struct_node->fields = fields;
    struct_node->methods = funcs;
    
    return struct_node;
}

AST_NodeEnumDecl *alloc_enum_node(String_View name, AST_EnumMembers members) {
    AST_NodeEnumDecl *enum_node = malloc(sizeof(AST_NodeEnumDecl));
    assert(enum_node != NULL && "Memory allocation failed");

    enum_node->kind = AST_ENUM_DECL;
    enum_node->name = name;
    enum_node->members = members;
    
    return enum_node;
}

AST_NodeLetStmt *alloc_let_node(String_View name, AST_Type *type, AST_Node *initializer, bool constant) {
    AST_NodeLetStmt *let_node = malloc(sizeof(AST_NodeLetStmt));
    assert(let_node != NULL && "Memory allocation failed");

    let_node->kind = AST_LET;
    let_node->name = name;
    let_node->type = type;
    let_node->initializer = initializer;
    let_node->constant = constant;
    
    return let_node;
}

AST_NodeForStmt *alloc_for_node(AST_Node *initializer, AST_Node *condition, AST_Node *next, AST_Node *body) {
    AST_NodeForStmt *for_node = malloc(sizeof(AST_NodeForStmt));
    assert(for_node != NULL && "Memory allocation failed");

    for_node->kind = AST_FOR;
    for_node->initializer = initializer;
    for_node->condition = condition;
    for_node->next = next;
    for_node->body = body;
    
    return for_node;
}

AST_NodeArray *alloc_arr_node(AST_Nodes nodes) {
    AST_NodeArray *arr_node = malloc(sizeof(AST_NodeArray));
    assert(arr_node != NULL && "Memory allocation failed");

    arr_node->kind = AST_ARR;
    arr_node->nodes = nodes;
    
    return arr_node;
}

AST_NodeBlock *alloc_block_node(AST_Nodes nodes, AST_Node *ret_expr) {
    AST_NodeBlock *block_node = malloc(sizeof(AST_NodeBlock));
    assert(block_node != NULL && "Memory allocation failed");

    block_node->kind = AST_BLOCK;
    block_node->nodes = nodes;
    block_node->ret_expr = ret_expr;
    
    return block_node;
}

AST_NodeBranch *alloc_branch_node(AST_Node *condition, AST_Node *body, AST_Nodes elif_branches, AST_Node *else_branch) {
    AST_NodeBranch *branch_node = malloc(sizeof(AST_NodeBranch));
    assert(branch_node != NULL && "Memory allocation failed");

    branch_node->kind = AST_IF;
    branch_node->condition = condition;
    branch_node->body = body;
    branch_node->elif_branches = elif_branches;
    branch_node->else_branch = else_branch;
    
    return branch_node;
}

AST_NodeProgram *alloc_program_node(AST_Nodes nodes) {
    AST_NodeProgram *program_node = malloc(sizeof(AST_NodeProgram));
    assert(program_node != NULL && "Memory allocation failed");

    program_node->kind = AST_PROGRAM;
    program_node->nodes = nodes;
    
    return program_node;
}

AST_NodeContinue *alloc_continue_signal() {
    AST_NodeContinue *sig = malloc(sizeof(AST_NodeContinue));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_CONTINUE;
    
    return sig;
}

AST_NodeBreak *alloc_break_signal() {
    AST_NodeBreak *sig = malloc(sizeof(AST_NodeBreak));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_BREAK;
    
    return sig;
}

AST_NodeReturn *alloc_return_signal(AST_Node *node) {
    AST_NodeReturn *sig = malloc(sizeof(AST_NodeReturn));
    assert(sig != NULL && "Memory allocation failed");

    sig->kind = AST_RETURN;
    sig->node = node;
    
    return sig;
}

AST_Type *alloc_arr_type_node(AST_Type *el_type) {
    AST_Type *type = malloc(sizeof(AST_Type));
    assert(type != NULL && "Memory allocation failed");

    type->kind = AST_TYPE;
    type->name = (String_View){0};
    type->el_type = el_type;
    type->provided_name = false;
    type->is_array = true;
    
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
        case TOKEN_ASSIGN: return BINOP_ASSIGN;
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
        case TOKEN_AND: return UNOP_REF;
        case TOKEN_NOT: return UNOP_NOT;
        case TOKEN_PLUS: return UNOP_PLUS;
        case TOKEN_MINUS: return UNOP_MINUS;
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

bool is_stmt(AST_Node *node) {
    return node->kind == AST_FUNC_DECL
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
        case TOKEN_KW_IF:
        case TOKEN_AT: return true;

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

            default: assert(0 && "UNREACHABLE");
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

            case UNOP_REF: {
                prec.val = 8;
                prec.right_associative = false;
            }

            default: assert(0 && "UNREACHABLE");
        }
    }

    return prec;
}

ParserError parse_unary_expr(Lexer *l, AST_Node **ret) {
    if (!is_unary_expr_start(l->cur)) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }

    UnaryOp op = get_unop(l->cur);
    Precedence prec = get_precedence(l->cur);
    
    lexer_next(l);

    AST_Node *expr;
    ParserError err = parse_expr(l, prec.val + prec.right_associative, &expr);
    if (err != PERROR_NONE) {
        return err;
    }

    *ret = (void*)alloc_unop_node(expr, op);
    return PERROR_NONE;
}

ParserError parse_num(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_INT && l->cur.kind != TOKEN_FLOAT) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    AST_Value val = {
        .view = l->cur.val,
    };
    
    if (l->cur.kind == TOKEN_FLOAT) {
        FLOAT_CTYPE float_int;
        if (!parse_float_sv(l->cur.val, &float_int)) {
            return PERROR_WRONG_NUMBER_FORMAT;
        }
        
        val.type = AST_TYPE_FLOAT;
    }
    
    if (l->cur.kind == TOKEN_INT) {
        INT_CTYPE num_int;
        if (!parse_int_sv(l->cur.val, &num_int)) {
            return PERROR_WRONG_NUMBER_FORMAT;
        }
        
        val.type = AST_TYPE_INT;
    }
    
    lexer_next(l);
    *ret = (void*)alloc_lit_node(val);
    return PERROR_NONE;
}

ParserError parse_bool(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_KW_TRUE && l->cur.kind != TOKEN_KW_FALSE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    AST_Value val = create_ast_value(AST_TYPE_BOOL, l->cur.val);
    val.as_bool = l->cur.kind == TOKEN_KW_TRUE;
    
    lexer_next(l);

    *ret = (void*)alloc_lit_node(val);
    return PERROR_NONE;
}

ParserError parse_str(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_STR) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    AST_Value val = create_ast_value(AST_TYPE_STR, l->cur.val);
    
    lexer_next(l);

    *ret = (void*)alloc_lit_node(val);
    return PERROR_NONE;
}

ParserError parse_char(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_CHAR);

    if (l->cur.val.count == 0) {
        lexer_next(l);
        return PERROR_EMPTY_CHAR_LIT;
    }

    size_t len = utf8nlen(l->cur.val.items, l->cur.val.count);
    
    int c;
    utf8codepoint(l->cur.val.items, &c);
    if (len == 2) {
        c = escape(l->cur.val.items[1]);
    }
    
    if (len > 1 && c < 0) {
        return PERROR_MULTI_CHARACTER_CHAR_LIT;
    }
    
    AST_Value val = create_ast_value(AST_TYPE_CHAR, l->cur.val);
    val.as_char = c;
    lexer_next(l);

    *ret = (void*)alloc_lit_node(val);
    return PERROR_NONE;
}

ParserError parse_lit_expr(Lexer *l, AST_Node **ret) {
    switch (l->cur.kind) {
        case TOKEN_INT:
        case TOKEN_FLOAT: {
            return parse_num(l, ret);
        }

        case TOKEN_STR: {
            return parse_str(l, ret);
        }

        case TOKEN_CHAR: {
            return parse_char(l, ret);
        }

        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE: {
            return parse_bool(l, ret);
        }
            
        default: break;
    }

    assert(0 && "UNREACHABLE");
}

ParserError parse_name_expr(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_NAME) {
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    AST_NodeName *name = alloc_name_node(l->cur.val);
    lexer_next(l);
    *ret = (void*)name;
    return PERROR_NONE;
}

ParserError parse_arr_expr(Lexer *l, AST_Node **arr_expr) {
    assert(l->cur.kind == TOKEN_LBRACK);

    lexer_next(l);

    AST_Nodes els = {0};
    
    while (l->cur.kind != TOKEN_RBRACK) {
        if (l->cur.kind == TOKEN_EOF) {
            da_free(els);
            return PERROR_UNEXPECTED_EOF;
        }

        AST_Node *el;
        ParserError err = parse_expr(l, 0, &el);
        if (err != PERROR_NONE) {
            da_free(els);
            return err;
        }
        
        if (l->cur.kind != TOKEN_COMMA && l->cur.kind != TOKEN_RBRACK) {
            ast_free(el);
            da_free(els);
            lexer_next(l);
            return PERROR_UNEXPECTED_TOKEN;
        }
        
        da_append(&els, el);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    lexer_next(l);
    
    *arr_expr = (void*)alloc_arr_node(els);
    return PERROR_NONE;
}

ParserError parse_block_expr(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_LBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    lexer_next(l);

    AST_Nodes nodes = {0};
    AST_Node *ret_expr = NULL;

    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            if (ret_expr != NULL) {
                ast_free(ret_expr);
            }
            
            ast_nodes_free(nodes);
            return PERROR_UNEXPECTED_EOF;
        }

        AST_Node *node;
        ParserError err = parse(l, &node);
        if (err != PERROR_NONE) {
            ast_nodes_free(nodes);
            ast_free(ret_expr);
            return err;
        }
        
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
    *ret = (void*)alloc_block_node(nodes, ret_expr);
    return PERROR_NONE;
}

ParserError parse_elif_expr(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_ELIF);

    lexer_next(l);

    AST_Node *condition;
    ParserError err = parse_expr(l, 0, &condition);
    if (err != PERROR_NONE) {
        return err;
    }
    
    AST_Node *body;
    err = parse_block_expr(l, &body);
    if (err != PERROR_NONE) {
        ast_free(condition);
        return err;
    }

    AST_Nodes elif_branches = {0};

    *ret = (void*)alloc_branch_node(condition, body, elif_branches, NULL);
    return PERROR_NONE;
}

ParserError parse_else_expr(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_ELSE);

    lexer_next(l);
    
    ParserError err = parse_block_expr(l, ret);
    return err;
}

ParserError parse_if_expr(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_IF);

    lexer_next(l);

    AST_Node *condition;
    ParserError err = parse_expr(l, 0, &condition);
    if (err != PERROR_NONE) {
        return err;
    }
    
    AST_Node *body;
    err = parse_block_expr(l, &body);
    if (err != PERROR_NONE) {
        ast_free(condition);
        return err;
    }
    
    AST_Nodes elif_branches = {0};
    while (l->cur.kind == TOKEN_KW_ELIF) {
        AST_Node *elif;
        err = parse_elif_expr(l, &elif);
        if (err != PERROR_NONE) {
            ast_free(condition);
            ast_free(body);
            ast_nodes_free(elif_branches);
            return err;
        }

        da_append(&elif_branches, elif);
    }

    AST_Node *else_branch = NULL;
    if (l->cur.kind == TOKEN_KW_ELSE) {
        err = parse_else_expr(l, &else_branch);
        if (err != PERROR_NONE) {
            ast_free(condition);
            ast_free(body);
            ast_nodes_free(elif_branches);
            return err;
        }
    }

    *ret = (void*)alloc_branch_node(condition, body, elif_branches, else_branch);
    return PERROR_NONE;
}

ParserError parse_for_stmt(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_FOR);

    lexer_next(l);

    ParserError err = PERROR_NONE;
    
    AST_Node *initializer = NULL;
    if (l->cur.kind != TOKEN_SEMICOLON) {
        err = parse(l, &initializer);
    }

    if (err != PERROR_NONE) {
        return err;
    }
    
    if (l->cur.kind != TOKEN_SEMICOLON) {
        ast_free(initializer);
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    AST_Node *condition = NULL;
    if (l->cur.kind != TOKEN_SEMICOLON) {
        err = parse(l, &condition);
    }

    if (err != PERROR_NONE) {
        ast_free(initializer);
        return err;
    }
    
    if (l->cur.kind != TOKEN_SEMICOLON) {
        ast_free(initializer);
        ast_free(condition);
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    AST_Node *next = NULL;
    if (l->cur.kind != TOKEN_LBRACK) {
        err = parse(l, &next);
    }

    if (err != PERROR_NONE) {
        ast_free(initializer);
        ast_free(condition);
        return err;
    }

    AST_Node *body;
    err = parse_block_expr(l, &body);
    if (err != PERROR_NONE) {
        ast_free(initializer);
        ast_free(condition);
        ast_free(next);
        return err;
    }

    *ret = (void*)alloc_for_node(initializer, condition, next, body);
    return PERROR_NONE;
}

ParserError parse_while_stmt(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_KW_WHILE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    AST_Node *condition;
    ParserError err = parse(l, &condition);
    if (err != PERROR_NONE) {
        return err;
    }
    
    AST_Node *body;
    err = parse_block_expr(l, &body);
    if (err != PERROR_NONE) {
        ast_free(condition);
        return err;
    }

    *ret = (void*)alloc_for_node(NULL, condition, NULL, body);
    return PERROR_NONE;
}

ParserError parse_forever_stmt(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_FOREVER);

    lexer_next(l);
    
    AST_Node *body;
    ParserError err = parse_block_expr(l, &body);
    if (err != PERROR_NONE) {
        return err;
    }

    *ret = (void*)alloc_for_node(NULL, NULL, NULL, body);
    return PERROR_NONE;
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

ParserError parse_return_signal(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_RETURN);

    lexer_next(l);

    AST_Node *ret_val = NULL;
    if (is_prim_expr_start(l->cur) || is_unary_expr_start(l->cur)) {
        ParserError err = parse_expr(l, 0, &ret_val);
        if (err != PERROR_NONE) {
            return err;
        }
    }
    
    *ret = (void*)alloc_return_signal(ret_val);
    return PERROR_NONE;
}

ParserError parse_prim_expr(Lexer *l, AST_Node **ret) {
    if (!is_prim_expr_start(l->cur)) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    switch (l->cur.kind) {
        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE:
        case TOKEN_CHAR:
        case TOKEN_STR:
        case TOKEN_INT:
        case TOKEN_FLOAT:{
            return parse_lit_expr(l, ret);
        }

        case TOKEN_NAME: {
            ParserError err = parse_name_expr(l, ret);
            return err;
        }
            
        case TOKEN_LPAREN: {
            lexer_next(l);
            ParserError err = parse_expr(l, 0, ret);
            if (err != PERROR_NONE) {
                return err;
            }

            if (l->cur.kind != TOKEN_RPAREN) {
                return PERROR_UNMATCHED_PAREN;
            }

            lexer_next(l);
            return PERROR_NONE;
        }

        case TOKEN_LBRACK: {
            return parse_arr_expr(l, ret);
        }
            
        case TOKEN_LBRACE: {
            return parse_block_expr(l, ret);
        }

        case TOKEN_KW_IF: {
            return parse_if_expr(l, ret);
        }

        case TOKEN_AT: {
            return parse_struct_expr(l, ret);
        }
            
        default: return PERROR_UNEXPECTED_TOKEN;
    }
}

ParserError parse_prefix_expr(Lexer *l, AST_Node **ret) {
    if (is_unop(l->cur)) {
        return parse_unary_expr(l, ret);
    }

    return parse_prim_expr(l, ret);
}

ParserError parse_args(Lexer *l, AST_Args *args) {
    AST_Args parsed = {0};
    
    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            ast_args_free(parsed);
            return PERROR_UNMATCHED_PAREN;
        }
        
        AST_Node *expr;
        ParserError err = parse_expr(l, 0, &expr);
        if (err != PERROR_NONE) {
            ast_args_free(parsed);
            return err;
        }

        AST_Arg arg = {
            .node = expr,
            .has_name = false,
        };
        
        if (expr->kind == AST_NAME && l->cur.kind == TOKEN_COLON) {
            lexer_next(l);
            
            err = parse_expr(l, 0, &arg.node);
            if (err != PERROR_NONE) {
                ast_free(arg.node);
                ast_args_free(parsed);
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

    if (l->cur.kind != TOKEN_RPAREN) {
        ast_args_free(parsed);
        return PERROR_UNMATCHED_PAREN;
    }
    
    lexer_next(l);

    for (size_t i = 0; i < parsed.count; ++i) {
        AST_Arg arg_1 = parsed.items[i];
        if (arg_1.name.count <= 0) {
            continue;
        }
        
        for (size_t j = i + 1; j < parsed.count; ++j) {
            AST_Arg arg_2 = parsed.items[j];
            if (arg_2.name.count <= 0) {
                continue;
            }
            
            if (sv_cmp(&arg_1.name, &arg_2.name)) {
                ast_args_free(parsed);
                return PERROR_MULTIPLE_ARGS;
            }
        }
    }
    
    *args = parsed;
    return PERROR_NONE;
}

ParserError parse_call_expr(Lexer *l, AST_Node *node, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_LPAREN);
    lexer_next(l);
    
    AST_Args args;
    ParserError err = parse_args(l, &args);
    if (err != PERROR_NONE) {
        return err;
    }
    
    *ret = (void*)alloc_call_node(node, args);
    return PERROR_NONE;
}

ParserError parse_index_expr(Lexer *l, AST_Node *node, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_LBRACK);

    lexer_next(l);

    AST_Node *index;
    ParserError err = parse_expr(l, 0, &index);
    if (err != PERROR_NONE) {
        return err;
    }

    if (l->cur.kind != TOKEN_RBRACK) {
        ast_free(index);
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    AST_Node *index_node = (void*)alloc_index_node(node, index);

    *ret = index_node;
    return PERROR_NONE;
}

ParserError parse_initializers(Lexer *l, AST_InitializerList *initializers) {
    if (l->cur.kind != TOKEN_LBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    *initializers = (AST_InitializerList){0};
    
    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            initializers_free(initializers);
            return PERROR_UNEXPECTED_EOF;
        }
        
        bool has_name = false;
        String_View name = {0};
        AST_Node *expr;
        ParserError err = parse_expr(l, 0, &expr);
        if (err != PERROR_NONE) {
            initializers_free(initializers);
            return err;
        }

        if (l->cur.kind == TOKEN_COLON) {
            lexer_next(l);

            has_name = true;

            if (expr->kind != AST_NAME) {
                ast_free(expr);
                initializers_free(initializers);
                return PERROR_WRONG_FIELD_NAME_FORMAT;
            }

            AST_NodeName *name_node = (void*)expr;
            
            name = name_node->name;
            err = parse_expr(l, 0, &expr);
            if (err != PERROR_NONE) {
                ast_free((void*)name_node);
                initializers_free(initializers);
                return err;
            }
        }

        AST_Initializer initializer = {
            .name = name,
            .has_name = has_name,
            .initializer = expr,
        };

        da_append(initializers, initializer);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    if (l->cur.kind != TOKEN_RBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    lexer_next(l);

    for (size_t i = 0; i < initializers->count; ++i) {
        AST_Initializer init_1 = initializers->items[i];
        if (init_1.name.count <= 0) {
            continue;
        }
        
        for (size_t j = i + 1; j < initializers->count; ++j) {
            AST_Initializer init_2 = initializers->items[j];
            if (init_2.name.count <= 0) {
                continue;
            }
            
            if (sv_cmp(&init_1.name, &init_2.name)) {
                initializers_free(initializers);
                return PERROR_MULTIPLE_INITIALIZERS;
            }
        }
    }

    return PERROR_NONE;
}

ParserError parse_struct_expr(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_AT) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    AST_Node *type;
    ParserError err = parse_type(l, &type);
    if (err != PERROR_NONE) {
        return err;
    }

    AST_InitializerList initializers;
    err = parse_initializers(l, &initializers);
    if (err != PERROR_NONE) {
        return err;
    }
    
    *ret = (void*)alloc_struct_expr_node(type, initializers);
    return PERROR_NONE;
}

ParserError parse_member_access_expr(Lexer *l, AST_Node *node, AST_Node **ret) {
    if (l->cur.kind != TOKEN_DOT) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    while (l->cur.kind == TOKEN_DOT) {
        lexer_next(l);
        
        AST_Node *member_node;
        ParserError err = parse_name_expr(l, &member_node);
        if (err != PERROR_NONE) {
            return err;
        }

        node = (void*)alloc_member_access_node(node, member_node);
    }

    *ret = node;
    return PERROR_NONE;
}

ParserError parse_cast_expr(Lexer *l, AST_Node *node, AST_Node **ret) {
    if (l->cur.kind != TOKEN_AS) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
        
    AST_Node *type_node;
    ParserError err = parse_type(l, &type_node);
    if (err != PERROR_NONE) {
        return err;
    }

    node = (void*)alloc_cast_node(node, (void*)type_node);

    *ret = node;
    return PERROR_NONE;
}

ParserError parse_postfix_expr(Lexer *l, AST_Node *node, AST_Node **ret) {
    ParserError err = PERROR_NONE;
    bool proceed = true;
    
    *ret = node;
    
    do {
        switch (l->cur.kind) {
            case TOKEN_LPAREN: {
                err = parse_call_expr(l, *ret, ret);
            } break;

            case TOKEN_LBRACK: {
                err = parse_index_expr(l, *ret, ret);
            } break;

            case TOKEN_DOT: {
                err = parse_member_access_expr(l, *ret, ret);
            } break;

            case TOKEN_AS: {
                err = parse_cast_expr(l, *ret, ret);
            } break;
            
            default: {
                proceed = false;
            } break;
        }

        if (err != PERROR_NONE) {
            ast_free(*ret);
            proceed = false;
        }
    } while (proceed);

    return err;
}

ParserError parse_expr(Lexer *l, size_t min_prec, AST_Node **ret) {
    AST_Node *lhs;
    ParserError err = parse_prefix_expr(l, &lhs);
    if (err != PERROR_NONE) {
        return err;
    }
    
    err = parse_postfix_expr(l, lhs, &lhs);
    if (err != PERROR_NONE) {
        return err;
    }

    while (is_binop(l->cur)) {
        Precedence prec = get_precedence(l->cur);
        if (prec.val < min_prec) break;

        Token binop_tok = l->cur;
        lexer_next(l);

        AST_Node *rhs;
        err = parse_expr(l, prec.right_associative? prec.val : prec.val + 1, &rhs);
        if (err != PERROR_NONE) {
            ast_free(lhs);
            return err;
        }
        
        lhs = (void*)alloc_binop_node(lhs, get_binop(binop_tok), rhs);
    }

    *ret = lhs;
    return PERROR_NONE;
}

ParserError parse_primitive_type(Lexer *l, AST_Node **ret) {
    String_View name_sv = l->cur.val;
    lexer_next(l);
    *ret = (void*)alloc_basic_type_node(name_sv);
    return PERROR_NONE;
}

ParserError parse_array_type(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_LBRACK) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    AST_Node *el_type = NULL;
    if (l->cur.kind != TOKEN_RBRACK) {
        ParserError err = parse_type(l, &el_type);
        if (err != PERROR_NONE) {
            return err;
        }
    }

    if (l->cur.kind != TOKEN_RBRACK) {
        ast_free(el_type);
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    *ret = (void*)alloc_arr_type_node((void*)el_type);
    return PERROR_NONE;
}

ParserError parse_ref_type(Lexer *l, AST_Node **ret) {
    if (l->cur.kind != TOKEN_AND) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    return parse_type(l, ret);
}

ParserError parse_type(Lexer *l, AST_Node **ret) {
    switch (l->cur.kind) {
        case TOKEN_ELLIPSIS:
        case TOKEN_NAME: {
            return parse_primitive_type(l, ret);
        }
            
        case TOKEN_LBRACK: {
            return parse_array_type(l, ret);
        }

        case TOKEN_AND: {
            return parse_ref_type(l, ret);
        }

        default: {
            return PERROR_UNEXPECTED_TOKEN;
        }
    }
}

ParserError parse_pattern(Lexer *l, AST_Pattern *pattern, bool is_const, bool is_static) {
    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }

    String_View name_sv = l->cur.val;

    lexer_next(l);

    if (l->cur.kind != TOKEN_COLON) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    lexer_next(l);
    
    AST_Node *type_node;
    ParserError err = parse_type(l, &type_node);
    if (err != PERROR_NONE) {
        return err;
    }

    AST_Node *default_initializer = NULL;
    if (l->cur.kind == TOKEN_ASSIGN) {
        lexer_next(l);

        err = parse_expr(l, 0, &default_initializer);
        if (err != PERROR_NONE) {
            ast_free(type_node);
            return err;
        }
    }
        
    *pattern = (AST_Pattern){
        .name = name_sv,
        .type = (void*)type_node,
        .is_const = is_const,
        .is_static = is_static,
        .initializer = default_initializer,
    };

    return PERROR_NONE;
}

ParserError parse_patterns(Lexer *l, AST_Patterns *patterns) {
    assert(l->cur.kind == TOKEN_LPAREN);

    lexer_next(l);
    
    AST_Patterns parsed = {0};
    bool has_va_args = false;

    while (l->cur.kind != TOKEN_RPAREN) {
        if (l->cur.kind == TOKEN_EOF) {
            ast_patterns_free(parsed);
            return PERROR_UNEXPECTED_EOF;
        }

        if (has_va_args) {
            ast_patterns_free(parsed);
            return PERROR_ARGS_AFTER_VA_ARG;
        }
        
        bool constant = false;
        if (l->cur.kind == TOKEN_KW_CONST) {
            constant = true;
            lexer_next(l);
        }
        
        AST_Pattern pattern = {0};
        ParserError err = parse_pattern(l, &pattern, constant, false);
        if (err != PERROR_NONE) {
            ast_pattern_free(pattern);
            ast_patterns_free(parsed);
            return err;
        }

        if (sv_cmp_cstr(&pattern.type->name, "...")) {
            has_va_args = true;
        }
        
        da_append(&parsed, pattern);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }

    if (l->cur.kind != TOKEN_RPAREN) {
        ast_patterns_free(parsed);
        return PERROR_UNMATCHED_PAREN;
    }
    
    lexer_next(l);

    *patterns = parsed;
    return PERROR_NONE;
}

ParserError parse_func_item(Lexer *l, bool is_static, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_FUNC);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    String_View name = l->cur.val;

    lexer_next(l);

    if (l->cur.kind != TOKEN_LPAREN) {
        return PERROR_UNEXPECTED_TOKEN;
    }
    
    AST_Patterns args;
    ParserError err = parse_patterns(l, &args);
    if (err != PERROR_NONE) {
        return err;
    }

    AST_Type *ret_type = NULL;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        
        AST_Node *type_node;
        err = parse_type(l, &type_node);
        if (err != PERROR_NONE) {
            ast_patterns_free(args);
            return err;
        }

        ret_type = (void*)type_node;
    }

    AST_Node *body = NULL;
    if (l->cur.kind == TOKEN_ARROW) {
        lexer_next(l);
        err = parse(l, &body);
    }
    else if (l->cur.kind == TOKEN_LBRACE) {
        err = parse_block_expr(l, &body);
    }
    else {
        err = PERROR_UNEXPECTED_TOKEN;
    }

    if (err != PERROR_NONE) {
        ast_patterns_free(args);
        ast_free((void*)ret_type);
        return err;
    }

    *ret = (void*)alloc_func_node(name, args, body, ret_type, is_static);
    return PERROR_NONE;
}

ParserError parse_struct_item(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_STRUCT);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }

    String_View name_sv = l->cur.val;

    lexer_next(l);
    
    if (l->cur.kind != TOKEN_LBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    AST_Patterns fields = {0};
    AST_Nodes funcs = {0};
    
    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            ast_patterns_free(fields);
            ast_nodes_free(funcs);
            return PERROR_UNEXPECTED_EOF;
        }

        bool is_static = l->cur.kind == TOKEN_KW_STATIC;
        if (is_static) {
            lexer_next(l);
        }
        
        bool is_const = l->cur.kind == TOKEN_KW_CONST;
        if (is_const) {
            lexer_next(l);
        }

        if (l->cur.kind == TOKEN_KW_STATIC) {
            ast_patterns_free(fields);
            ast_nodes_free(funcs);
            return PERROR_UNEXPECTED_TOKEN;
        }
        
        switch (l->cur.kind) {
            case TOKEN_NAME: {
                AST_Pattern field;
                ParserError err = parse_pattern(l, &field, is_const, is_static);
                if (err != PERROR_NONE) {
                    ast_patterns_free(fields);
                    ast_nodes_free(funcs);
                    return err;
                }

                da_append(&fields, field);
            } break;

            case TOKEN_KW_FUNC: {
                AST_Node *func;
                ParserError err = parse_func_item(l, is_static, &func);
                if (err != PERROR_NONE) {
                    ast_patterns_free(fields);
                    ast_nodes_free(funcs);
                    return err;
                }

                da_append(&funcs, (void*)func);
            } break;

            default: {
                ast_patterns_free(fields);
                ast_nodes_free(funcs);
                return PERROR_UNEXPECTED_TOKEN;
            }
        }
    }
    
    if (l->cur.kind != TOKEN_RBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    *ret = (void*)alloc_struct_node(name_sv, fields, funcs);
    return PERROR_NONE; 
}

ParserError parse_enum_item(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_ENUM);

    lexer_next(l);

    if (l->cur.kind != TOKEN_NAME) {
        lexer_next(l);
        return PERROR_UNEXPECTED_TOKEN;
    }

    String_View name_sv = l->cur.val;

    lexer_next(l);
    
    if (l->cur.kind != TOKEN_LBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);
    
    AST_EnumMembers members = {0};
    
    while (l->cur.kind != TOKEN_RBRACE) {
        if (l->cur.kind == TOKEN_EOF) {
            ast_enum_members_free(members);
            return PERROR_UNEXPECTED_EOF;
        }

        if (l->cur.kind != TOKEN_NAME) {
            ast_enum_members_free(members);
            return PERROR_UNEXPECTED_TOKEN;
        }

        String_View member_name_sv = l->cur.val;

        lexer_next(l);
        
        if (l->cur.kind != TOKEN_COLON) {
            AST_EnumMember member = {
                .name = member_name_sv,
                .initializer = NULL,
            };
        
            da_append(&members, member);
            continue;
        }

        lexer_next(l);
        
        AST_Node *initializer = NULL;
        ParserError err = parse_expr(l, 0, &initializer);

        if (err != PERROR_NONE) {
            ast_enum_members_free(members);
            return err;
        }

        AST_EnumMember member = {
            .name = member_name_sv,
            .initializer = initializer,
        };
        
        da_append(&members, member);

        if (l->cur.kind == TOKEN_COMMA) {
            lexer_next(l);
        }
    }
    
    if (l->cur.kind != TOKEN_RBRACE) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    lexer_next(l);

    if (members.count <= 0) {
        return PERROR_TOO_FEW_ENUM_MEMBERS;
    }
    
    *ret = (void*)alloc_enum_node(name_sv, members);
    return PERROR_NONE; 
}

ParserError parse_let_stmt(Lexer *l, bool constant, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_LET || l->cur.kind == TOKEN_NAME);

    if (l->cur.kind == TOKEN_KW_LET) {
        lexer_next(l);
    }

    if (l->cur.kind != TOKEN_NAME) {
        return PERROR_UNEXPECTED_TOKEN;
    }

    String_View name = l->cur.val;

    lexer_next(l);

    AST_Type *type_node = NULL;
    if (l->cur.kind == TOKEN_COLON) {
        lexer_next(l);
        AST_Node *node;
        ParserError err = parse_type(l, &node);
        if (err != PERROR_NONE) {
            return err;
        }

        type_node = (void*)node;
    }
    
    if (l->cur.kind != TOKEN_ASSIGN) {
        *ret = (void*)alloc_let_node(name, type_node, NULL, constant);
        return PERROR_NONE;
    }
    
    lexer_next(l);
    
    AST_Node *initializer;
    ParserError err = parse_expr(l, 0, &initializer);
    if (err != PERROR_NONE) {
        return err;
    }

    *ret = (void*)alloc_let_node(name, type_node, initializer, constant);
    return PERROR_NONE;
}

ParserError parse_const_item(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    ParserError err = PERROR_NONE;
    
    switch (l->cur.kind) {
        case TOKEN_NAME: {
            err = parse_let_stmt(l, true, &node);
        } break;
            
        default: {
            err = PERROR_UNEXPECTED_TOKEN;
        } break;
    }

    *ret = node;
    return err;
}

ParserError parse_const_stmt(Lexer *l, AST_Node **ret) {
    assert(l->cur.kind == TOKEN_KW_CONST);
    lexer_next(l);

    AST_Node *node = NULL;
    ParserError err = PERROR_NONE;
    
    switch (l->cur.kind) {
        case TOKEN_NAME: {
            err = parse_let_stmt(l, true, &node);
        } break;
            
        default: {
            err = PERROR_UNEXPECTED_TOKEN;
        } break;
    }

    *ret = node;
    return err;
}

ParserError parse_stmt(Lexer *l, AST_Node **ret) {
    AST_Node *node = NULL;
    ParserError err = PERROR_NONE;
    
    switch (l->cur.kind) {
        case TOKEN_KW_CONST: {
            err = parse_const_stmt(l, &node);
        } break;
        
        case TOKEN_KW_LET: {
            err = parse_let_stmt(l, false, &node);
        } break;

        case TOKEN_KW_FOR: {
            err = parse_for_stmt(l, &node);
        } break;

        case TOKEN_KW_WHILE: {
            err = parse_while_stmt(l, &node);
        } break;

        case TOKEN_KW_FOREVER: {
            err = parse_forever_stmt(l, &node);
        } break;

        case TOKEN_KW_CONTINUE: {
            node = parse_continue_signal(l);
        } break;
            
        case TOKEN_KW_BREAK: {
            node = parse_break_signal(l);
        } break;
            
        case TOKEN_KW_RETURN: {
            err = parse_return_signal(l, &node);
        } break;
            
        default: {
            err = PERROR_UNEXPECTED_TOKEN;
        } break;
    }

    *ret = node;
    return err;
}

ParserError parse(Lexer *l, AST_Node **ret) {
    AST_Node *node = NULL;
    ParserError err = PERROR_NONE;
    
    switch (l->cur.kind) {
        case TOKEN_KW_LET:
        case TOKEN_KW_CONST:
        case TOKEN_KW_FOR:
        case TOKEN_KW_WHILE:
        case TOKEN_KW_FOREVER:
        case TOKEN_KW_CONTINUE:
        case TOKEN_KW_BREAK:
        case TOKEN_KW_RETURN:{
            err = parse_stmt(l, &node);
        } break;
            
        default: {
            err = parse_expr(l, 0, &node);
        } break;
    }

    *ret = node;
    return err;
}

ParserError parse_item(Lexer *l, AST_Node **ret) {
    AST_Node *node = NULL;
    ParserError err = PERROR_NONE;

    bool is_static = l->cur.kind == TOKEN_KW_STATIC;
    
    switch (l->cur.kind) {
        case TOKEN_KW_FUNC: {
            err = parse_func_item(l, is_static, &node);
        } break;

        case TOKEN_KW_STRUCT: {
            if (is_static) {
                return PERROR_UNEXPECTED_TOKEN;
            }
            
            err = parse_struct_item(l, &node);
        } break;

        case TOKEN_KW_ENUM: {
            if (is_static) {
                return PERROR_UNEXPECTED_TOKEN;
            }
            
            err = parse_enum_item(l, &node);
        } break;

        case TOKEN_KW_CONST: {
            if (is_static) {
                return PERROR_UNEXPECTED_TOKEN;
            }
            
            err = parse_const_item(l, &node);
        } break;

        default: {
            if (is_static) {
                return PERROR_UNEXPECTED_TOKEN;
            }
            
            err = parse(l, &node);
        }
    }
    
    skip(l, TOKEN_SEMICOLON);

    *ret = node;
    return err;
}

ParserError parse_program(Lexer *l, AST_NodeProgram **ret) {
    AST_Nodes nodes = {0};

    while (l->cur.kind != TOKEN_EOF) {
        AST_Node *node = NULL;
        ParserError err = parse_item(l, &node);
        if (err != PERROR_NONE) {
            ast_nodes_free(nodes);
            return err;
        }
        
        da_append(&nodes, node);
    }
    
    *ret = alloc_program_node(nodes);
    return PERROR_NONE;
}