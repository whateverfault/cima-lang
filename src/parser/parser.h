#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "parser_error.h"
#include "ast.h"

typedef struct {
    bool right_associative;
    size_t val;
} Precedence;

void ast_nodes_free(AST_Nodes funcs);
void ast_patterns_free(AST_Patterns patterns);
void ast_funcs_free(AST_Patterns patterns);
void ast_free(AST_Node *node);

AST_Type *alloc_arr_type_node(AST_Type *el_type);

AST_NodeBinOp *alloc_binop_node(AST_Node *lhs, BinaryOp op, AST_Node *rhs);
AST_NodeUnOp *alloc_unop_node(AST_Node *expr, UnaryOp op);
AST_NodeLit *alloc_lit_node(AST_Value val);
AST_NodeName *alloc_name_node(String_View sv);
AST_NodeArray *alloc_arr_node(AST_Nodes nodes);

ParserError parse_type(Lexer *l, AST_Node **ret);

ParserError parse_name_expr(Lexer *l, AST_Node **ret);

ParserError parse_struct_expr(Lexer *l, AST_Node **ret);
ParserError parse_prefix_expr(Lexer *l, AST_Node **ret);
ParserError parse_expr(Lexer *l, size_t min_prec, AST_Node **ret);
ParserError parse(Lexer *l, AST_Node **ret);
ParserError parse_item(Lexer *l, AST_Node **ret);
ParserError parse_nodes(Lexer *l, AST_Nodes *nodes);
ParserError parse_program(Lexer *l, AST_NodeProgram **ret);

#endif //PARSER_H
