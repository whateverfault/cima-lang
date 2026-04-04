#include "ctype.h"
#include "lexer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CURRENT(l) (l)->source[(l)->pos]
#define INBOUNDS(l) ((l)->pos < (l)->source_len && CURRENT(l) != '\0')

void token_free(Token tok) {
    switch (tok.kind) {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_STR:
        case TOKEN_CHAR:
        case TOKEN_NAME:
        case TOKEN_KW_FN:
        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE:{
            free(tok.val);
        } break;

        default: break;
    }
}

bool is_binop(Token tok) {
    switch (tok.kind) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
        case TOKEN_CARET:
        case TOKEN_EQ:
        case TOKEN_GT:
        case TOKEN_LT:
        case TOKEN_GTEQ:
        case TOKEN_LTEQ: {
            return true;
        }

        default: return false;
    }
}

bool is_unop(Token tok) {
    switch (tok.kind) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:{
            return true;
        }

        default: return false;
    }
}

bool is_special(Token tok) {
    switch (tok.kind) {
        case TOKEN_LPAREN:
        case TOKEN_RPAREN:
        case TOKEN_LBRACK:
        case TOKEN_RBRACK:
        case TOKEN_LBRACE:
        case TOKEN_RBRACE:
        case TOKEN_DOT:
        case TOKEN_COMMA:
        case TOKEN_COLON:{
            return true;
        }

        default: return false;
    }
}

bool is_binop_symb(char symb) {
    switch (symb) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case '=':
        case '>':
        case '<':
        case TOKEN_GTEQ:
        case TOKEN_LTEQ: {
            return true;
        }

        default: return false;
    }
}

bool is_unop_symb(char symb) {
    switch (symb) {
        case '+':
        case '-':{
            return true;
        }

        default: return false;
    }
}

bool is_special_symb(char symb) {
    switch (symb) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ',':
        case ':':
        case ';':
        case '\"':
        case '\'':{
            return true;
        }

        default: return false;
    }
}

char escape(char c) {
    switch (c) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'b': return '\b';
        case 'a': return '\a';
        case '\'': return '\'';
        case '\"': return '\"';

        // TODO: Throw an error
        default: return c;
    }
}

char *fill_val(Lexer *l, size_t len, size_t start) {
    size_t val_len = l->pos - start;
    
    if (val_len == 0) {
        return NULL;
    }
    
    char *val = (char*)malloc(sizeof(char) * (len + 1));
    assert(val != NULL && "Memory allocation failed");

    size_t decrease = 0;
    for (size_t i = 0; i < val_len; ++i) {
        char c = l->source[start + i];
        if (c == '\\' && i + 1 < val_len) {
            c = escape(l->source[start + ++i]);
            ++decrease;
        }
        
        val[i - decrease] = c;
    }

    val[len] = '\0';
    return val;
}

void skip_whitespaces(Lexer *l) {
    if (l->skipped != NULL && l->skipped != "") {
        free(l->skipped);
    }
    
    size_t start = l->pos;
    
    while (INBOUNDS(l) && (isblank(CURRENT(l)) || iscntrl(CURRENT(l)))) {
        ++l->pos;
    }

    l->skipped = fill_val(l, l->pos - start, start);
    
    if (l->skipped == NULL) {
        l->skipped = "";
    }
}

Token lexer_get_number(Lexer *l) {
    bool is_integer = true;
    size_t start = l->pos;
    
    while (INBOUNDS(l) && (isdigit(CURRENT(l)) || CURRENT(l) == '.')) {
        if (CURRENT(l) == '.') {
            is_integer = false;
        }
        
        ++l->pos;
    }
    
    return (Token){
        .val = fill_val(l, l->pos - start, start),
        .kind = is_integer? TOKEN_INT : TOKEN_FLOAT,
    };
}

Token lexer_get_str(Lexer *l) {
    size_t start = ++l->pos;

    size_t len = 0;
    while (INBOUNDS(l)
        && CURRENT(l) != '\"') {
        if (CURRENT(l) == '\\') {
            ++l->pos;
        }

        if (CURRENT(l) == '{') {
            while (INBOUNDS(l) && CURRENT(l) != '}'){
                ++len;
                ++l->pos;
            }
        }
        
        ++len;
        ++l->pos;
    }

    char *val = fill_val(l, len, start);
    ++l->pos;
    
    return (Token){
        .val = val,
        .kind = TOKEN_STR,
    };
}

Token lexer_get_char(Lexer *l) {
    size_t start = ++l->pos;
    
    size_t len = 0;
    while (INBOUNDS(l)
        && CURRENT(l) != '\'') {
        if (CURRENT(l) == '\\') {
            ++l->pos;
        }
        
        ++len;
        ++l->pos;
    }

    char *val = fill_val(l, len, start);
    ++l->pos;

    return (Token){
        .val = val,
        .kind = TOKEN_CHAR,
    };
}

Token lexer_get_name(Lexer *l) {
    size_t start = l->pos;
    
    while (INBOUNDS(l)
        && !isblank(CURRENT(l))
        && !is_special_symb(CURRENT(l))
        && !is_binop_symb(CURRENT(l))) {
        ++l->pos;
    }

    return (Token){
        .val = fill_val(l, l->pos - start, start),
        .kind = TOKEN_NAME,
    };
}

Token get_lit_token(Lexer *l) {
    Token tok;
    
    if (isdigit(CURRENT(l)) || CURRENT(l) == '.'){
        tok = lexer_get_number(l);
    }
    else if (CURRENT(l) == '\"'){
        tok = lexer_get_str(l);
    }
    else if (CURRENT(l) == '\''){
        tok = lexer_get_char(l);
    }
    
    else {
        tok = lexer_get_name(l);
        
        if (strncmp(tok.val, "fn", strlen(tok.val)) == 0) {
            tok.kind = TOKEN_KW_FN;
        }

        if (strncmp(tok.val, "const", strlen(tok.val)) == 0) {
            tok.kind = TOKEN_KW_CONST;
        }

        if (strncmp(tok.val, "let", strlen(tok.val)) == 0) {
            tok.kind = TOKEN_KW_LET;
        }
        
        if (strncmp(tok.val, "true", strlen(tok.val)) == 0) {
            tok.kind = TOKEN_KW_TRUE;
        }

        if (strncmp(tok.val, "false", strlen(tok.val)) == 0) {
            tok.kind = TOKEN_KW_FALSE;
        }
    }
    
    return tok;
}

Token get_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_NONE,
    };
    
    switch (CURRENT(l)) {
        case '+': {
            tok = (Token){
                .val = "+",
                .kind = TOKEN_PLUS,
            };
        } break;

        case '-': {
            tok = (Token){
                .val = "-",
                .kind = TOKEN_MINUS,
            };
        } break;

        case '*': {
            tok = (Token){
                .val = "*",
                .kind = TOKEN_STAR,
            };
        } break;

        case '/': {
            tok = (Token){
                .val = "/",
                .kind = TOKEN_SLASH,
            };
        } break;

        case '%': {
            tok = (Token){
                .val = "%",
                .kind = TOKEN_PERCENT,
            };
        } break;

        case '^': {
            tok = (Token){
                .val = "^",
                .kind = TOKEN_CARET,
            };
        } break;

        case '(': {
            tok = (Token){
                .val = "(",
                .kind = TOKEN_LPAREN,
            };
        } break;

        case ')': {
            tok = (Token){
                .val = ")",
                .kind = TOKEN_RPAREN,
            };
        } break;

        case '[': {
            tok = (Token){
                .val = "[",
                .kind = TOKEN_LBRACK,
            };
        } break;

        case ']': {
            tok = (Token){
                .val = "]",
                .kind = TOKEN_RBRACK,
            };
        } break;

        case '{': {
            tok = (Token){
                .val = "{",
                .kind = TOKEN_LBRACE,
            };
        } break;

        case '}': {
            tok = (Token){
                .val = "}",
                .kind = TOKEN_RBRACE,
            };
        } break;

        case '.': {
            tok = (Token){
                .val = ".",
                .kind = TOKEN_DOT,
            };
        } break;
            
        case ',': {
            tok = (Token){
                .val = ",",
                .kind = TOKEN_COMMA,
            };
        } break;

        case ':': {
            tok = (Token){
                .val = ":",
                .kind = TOKEN_COLON,
            };
        } break;

        case ';': {
            tok = (Token){
                .val = ";",
                .kind = TOKEN_SEMICOLON,
            };
        } break;
            
        default: break;
    }

    if (tok.kind != TOKEN_NONE) {
        ++l->pos;
    }
    
    return tok;
}

Token get_eq_token(Lexer *l) {
    TokenKind kind = TOKEN_EQ;

    ++l->pos;
    if (CURRENT(l) == '>') {
        kind = TOKEN_ARROW;
        ++l->pos;
    }

    Token tok = {
        .kind = kind,
    };
    
    if (kind == TOKEN_EQ) {
        tok.val = "=";
    }
    else {
        tok.val = "=>";
    }

    return tok;
}

Token get_gt_token(Lexer *l) {
    TokenKind kind = TOKEN_GT;

    ++l->pos;
    if (CURRENT(l) == '=') {
        kind = TOKEN_GTEQ;
        ++l->pos;
    }

    Token tok = {
        .kind = kind,
    };
    
    if (kind == TOKEN_GT) {
        tok.val = ">";
    }
    else {
        tok.val = ">=";
    }

    return tok;
}

Token get_lt_token(Lexer *l) {
    TokenKind kind = TOKEN_LT;

    ++l->pos;
    if (CURRENT(l) == '=') {
        kind = TOKEN_LTEQ;
        ++l->pos;
    }

    Token tok = {
        .kind = kind,
    };
    
    if (kind == TOKEN_LT) {
        tok.val = "<";
    }
    else {
        tok.val = "<=";
    }

    return tok;
}

Token get_dot_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_DOT,
        .val = ".",
    };

    ++l->pos;
    if (CURRENT(l) == '.') {
        tok.kind = TOKEN_RANGE;
        tok.val = "..";
        ++l->pos;
        
        if (CURRENT(l) == '.') {
            tok.kind = TOKEN_ELLIPSIS;
            tok.val = "...";
            ++l->pos;
        }
    }
    
    return tok;
}

Token lexer_next(Lexer *l) {
    token_free(l->cur);
    skip_whitespaces(l);
    
    if (!INBOUNDS(l)) {
        l->cur = (Token){
            .val = "",
            .kind = TOKEN_EOF,
        };
        return l->cur;
    }
    
    Token tok;
    
    switch (CURRENT(l)) {
        case '=': {
            tok = get_eq_token(l);
        } break;

        case '>': {
            tok = get_gt_token(l);
        } break;

        case '<': {
            tok = get_lt_token(l);
        } break;

        case '.': {
            tok = get_dot_token(l);
        }
            
        default: {
            tok = get_token(l);
        } break;
    }

    if (tok.kind == TOKEN_NONE) {
        tok = get_lit_token(l);
    }
    
    l->cur = tok;
    return tok;
}

void lexer_init(Lexer *l) {
    lexer_next(l);
}