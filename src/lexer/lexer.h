#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TOKEN_NONE,
    TOKEN_EOF,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GTEQ,
    TOKEN_LTEQ,
    TOKEN_EQ,
    TOKEN_ARROW,
    
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STR,
    TOKEN_CHAR,
    TOKEN_NAME,
    
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
    
    TOKEN_KW_FN,
    TOKEN_KW_CONST,
    TOKEN_KW_LET,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
} TokenKind;

typedef struct {
    char *val;
    TokenKind kind;
} Token;

typedef struct {
    char *source;
    char *skipped;
    size_t source_len;
    size_t pos;
    Token cur;
} Lexer;

bool is_binop(Token tok);
bool is_unop(Token tok);

Token lexer_next(Lexer *l);
void lexer_init(Lexer *l);

#endif //LEXER_H
