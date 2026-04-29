#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#include "nothing/nothing.h"

typedef enum {
    TOKEN_NONE,
    TOKEN_EOF,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,

    TOKEN_AND,
    TOKEN_ANDAND,
    TOKEN_OR,
    TOKEN_OROR,

    TOKEN_PLUSPLUS,
    TOKEN_MINUSMINUS,
    
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GTEQ,
    TOKEN_LTEQ,
    TOKEN_NOT,
    TOKEN_EQ,
    TOKEN_NEQ,
    
    TOKEN_ARROW,
    TOKEN_ASSIGN,

    TOKEN_AS,
    
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STR,
    TOKEN_CHAR,
    TOKEN_NAME,

    TOKEN_AT,
    
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    
    TOKEN_LBRACK,
    TOKEN_RBRACK,

    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_RANGE,
    TOKEN_ELLIPSIS,
    
    TOKEN_KW_FUNC,
    TOKEN_KW_IF,
    TOKEN_KW_ELIF,
    TOKEN_KW_ELSE,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_BREAK,
    TOKEN_KW_RETURN,
    TOKEN_KW_FOR,
    TOKEN_KW_WHILE,
    TOKEN_KW_FOREVER,
    TOKEN_KW_CONST,
    TOKEN_KW_STATIC,
    TOKEN_KW_LET,
    TOKEN_KW_STRUCT,
    TOKEN_KW_ENUM,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
} TokenKind;

typedef struct {
    String_View val;
    TokenKind kind;
} Token;

typedef struct {
    String_View source;
    String_View skipped;
    size_t pos;
    Token cur;
} Lexer;

int escape(int c);

bool is_binop(Token tok);
bool is_unop(Token tok);

Token lexer_next(Lexer *l);
void lexer_init(Lexer *l);

#endif //LEXER_H
