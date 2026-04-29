#include "ctype.h"
#include "lexer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CURRENT(l) (l)->source.items[(l)->pos]
#define INBOUNDS(l) ((l)->pos < (l)->source.count)

bool is_binop(Token tok) {
    switch (tok.kind) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
        case TOKEN_CARET:
        case TOKEN_ASSIGN:
        case TOKEN_ANDAND:
        case TOKEN_AND:
        case TOKEN_OROR:
        case TOKEN_OR:
        case TOKEN_GT:
        case TOKEN_LT:
        case TOKEN_GTEQ:
        case TOKEN_LTEQ:
        case TOKEN_EQ:
        case TOKEN_NEQ:{
            return true;
        }

        default: return false;
    }
}

bool is_unop(Token tok) {
    switch (tok.kind) {
        case TOKEN_AND:
        case TOKEN_PLUSPLUS:
        case TOKEN_MINUSMINUS:
        case TOKEN_NOT:
        case TOKEN_PLUS:
        case TOKEN_MINUS:{
            return true;
        }

        default: return false;
    }
}

bool is_special(Token tok) {
    switch (tok.kind) {
        case TOKEN_AT:
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
        case '.':
        case ':':
        case ';':
        case '\"':
        case '\'':{
            return true;
        }

        default: return false;
    }
}

int escape(int c) {
    switch (c) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'b': return '\b';
        case 'a': return '\a';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        
        default: return -1;
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
        char c = l->source.items[start + i];
        if (c == '\\' && i + 1 < val_len) {
            c = escape(l->source.items[start + ++i]);
            ++decrease;
        }
        
        val[i - decrease] = c;
    }

    val[len] = '\0';
    return val;
}

void skip_whitespaces(Lexer *l) {
    l->skipped.count = 0;
    l->skipped.items = l->source.items + l->pos;
    
    while (INBOUNDS(l) && (isblank(CURRENT(l)) || iscntrl(CURRENT(l)))) {
        ++l->skipped.count;
        ++l->pos;
    }
}

Token lexer_get_number(Lexer *l) {
    Token tok = {
        .kind = TOKEN_INT,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 0,
        },
    };

    char last = '\0';
    while (INBOUNDS(l) && (isdigit(CURRENT(l)) || CURRENT(l) == '.' || CURRENT(l) == 'e' || CURRENT(l) == 'E' || CURRENT(l) == '-' || CURRENT(l) == '+')) {
        if ((CURRENT(l) == '-' || CURRENT(l) == '+') && (last != 'e' && last != 'E')) {
            break;
        }

        last = CURRENT(l);
        
        if (CURRENT(l) == '.' || CURRENT(l) == 'e' || CURRENT(l) == '-' || CURRENT(l) == '+') {
            tok.kind = TOKEN_FLOAT;
        }
        
        ++tok.val.count;
        ++l->pos;
    }

    while (tok.val.count > 0 && (tok.val.items[tok.val.count - 1] == '-' || tok.val.items[tok.val.count - 1] == '+')) {
        --tok.val.count;
        --l->pos;
    }
    
    return tok;
}

Token lexer_get_str(Lexer *l) {
    ++l->pos;
    
    Token tok = {
        .kind = TOKEN_STR,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 0,
        },
    };
    
    while (INBOUNDS(l)
        && CURRENT(l) != '\"') {
        if (CURRENT(l) == '\\') {
            ++tok.val.count;
            ++l->pos;
        }

        if (CURRENT(l) == '{') {
            size_t to_skip = 0;
            while (INBOUNDS(l) && (CURRENT(l) != '}' || to_skip != 0)){
                to_skip -= CURRENT(l) == '}';
                
                ++tok.val.count;
                ++l->pos;

                to_skip += CURRENT(l) == '{';
            }
        }
        
        ++tok.val.count;
        ++l->pos;
    }

    ++l->pos;
    return tok;
}

Token lexer_get_char(Lexer *l) {
    ++l->pos;
    
    Token tok = {
        .kind = TOKEN_CHAR,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 0,
        },
    };
    
    while (INBOUNDS(l)
        && CURRENT(l) != '\'') {
        if (CURRENT(l) == '\\') {
            ++tok.val.count;
            ++l->pos;
        }
        
        ++tok.val.count;
        ++l->pos;
    }

    ++l->pos;
    return tok;
}

Token lexer_get_name(Lexer *l) {
    Token tok = {
        .kind = TOKEN_NAME,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 0,
        },
    };
    
    while (INBOUNDS(l)
        && !isblank(CURRENT(l))
        && !iscntrl(CURRENT(l))
        && !isspace(CURRENT(l))
        && !is_special_symb(CURRENT(l))
        && !is_binop_symb(CURRENT(l))) {
        ++tok.val.count;
        ++l->pos;
    }
    
    return tok;
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
    
        if (sv_cmp_cstr(&tok.val, "func")) {
            tok.kind = TOKEN_KW_FUNC;
        }
        else if (sv_cmp_cstr(&tok.val, "let")) {
            tok.kind = TOKEN_KW_LET;
        }
        else if (sv_cmp_cstr(&tok.val, "struct")) {
            tok.kind = TOKEN_KW_STRUCT;
        }
        else if (sv_cmp_cstr(&tok.val, "enum")) {
            tok.kind = TOKEN_KW_ENUM;
        }
        else if (sv_cmp_cstr(&tok.val, "const")) {
            tok.kind = TOKEN_KW_CONST;
        }
        else if (sv_cmp_cstr(&tok.val, "static")) {
            tok.kind = TOKEN_KW_STATIC;
        }
        else if (sv_cmp_cstr(&tok.val, "for")) {
            tok.kind = TOKEN_KW_FOR;
        }
        else if (sv_cmp_cstr(&tok.val, "while")) {
            tok.kind = TOKEN_KW_WHILE;
        }
        else if (sv_cmp_cstr(&tok.val, "forever")) {
            tok.kind = TOKEN_KW_FOREVER;
        }
        else if (sv_cmp_cstr(&tok.val, "if")) {
            tok.kind = TOKEN_KW_IF;
        }
        else if (sv_cmp_cstr(&tok.val, "elif")) {
            tok.kind = TOKEN_KW_ELIF;
        }
        else if (sv_cmp_cstr(&tok.val, "else")) {
            tok.kind = TOKEN_KW_ELSE;
        }
        else if (sv_cmp_cstr(&tok.val, "continue")) {
            tok.kind = TOKEN_KW_CONTINUE;
        }
        else if (sv_cmp_cstr(&tok.val, "break")) {
            tok.kind = TOKEN_KW_BREAK;
        }
        else if (sv_cmp_cstr(&tok.val, "return")) {
            tok.kind = TOKEN_KW_RETURN;
        }
        else if (sv_cmp_cstr(&tok.val, "true")) {
            tok.kind = TOKEN_KW_TRUE;
        }
        else if (sv_cmp_cstr(&tok.val, "false")) {
            tok.kind = TOKEN_KW_FALSE;
        }
        else if (sv_cmp_cstr(&tok.val, "and")) {
            tok.kind = TOKEN_ANDAND;
        }
        else if (sv_cmp_cstr(&tok.val, "or")) {
            tok.kind = TOKEN_OROR;
        }
        else if (sv_cmp_cstr(&tok.val, "as")) {
            tok.kind = TOKEN_AS;
        }
    }
    
    return tok;
}

Token get_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_NONE,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
switch (CURRENT(l)) {
        case '*': {
            tok.kind = TOKEN_STAR;
        } break;

        case '/': {
            tok.kind = TOKEN_SLASH;
        } break;

        case '%': {
            tok.kind = TOKEN_PERCENT;
        } break;

        case '^': {
            tok.kind = TOKEN_CARET;
        } break;

        case '(': {
            tok.kind = TOKEN_LPAREN;
        } break;

        case ')': {
            tok.kind = TOKEN_RPAREN;
        } break;

        case '[': {
            tok.kind = TOKEN_LBRACK;
        } break;

        case ']': {
            tok.kind = TOKEN_RBRACK;
        } break;

        case '{': {
            tok.kind = TOKEN_LBRACE;
        } break;

        case '}': {
            tok.kind = TOKEN_RBRACE;
        } break;
        
        case ',': {
            tok.kind = TOKEN_COMMA;
        } break;

        case ':': {
            tok.kind = TOKEN_COLON;
        } break;

        case ';': {
            tok.kind = TOKEN_SEMICOLON;
        } break;

        case '@': {
            tok.kind = TOKEN_AT;
        } break;
            
        default: {
            tok.val.count = 0;
        } break;
    }

    if (tok.val.count != 0) {
        ++l->pos;
    }
    
    return tok;
}

Token get_eq_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_ASSIGN,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };

    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '=') {
        tok.kind = TOKEN_EQ;
        ++tok.val.count;
        ++l->pos;
    } else if (INBOUNDS(l) && CURRENT(l) == '>') {
        tok.kind = TOKEN_ARROW;
        ++tok.val.count;
        ++l->pos;
    }

    return tok;
}

Token get_not_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_NOT,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };

    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '=') {
        tok.kind = TOKEN_NEQ;
        ++tok.val.count;
        ++l->pos;
    }

    return tok;
}

Token get_gt_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_GT,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };

    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '=') {
        tok.kind = TOKEN_GTEQ;
        ++l->pos;
    }

    return tok;
}

Token get_lt_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_LT,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };

    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '=') {
        tok.kind = TOKEN_LTEQ;
        ++tok.val.count;
        ++l->pos;
    }

    return tok;
}

Token get_dot_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_DOT,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '.') {
        tok.kind = TOKEN_RANGE;
        ++tok.val.count;
        ++l->pos;
        
        if (INBOUNDS(l) && CURRENT(l) == '.') {
            tok.kind = TOKEN_ELLIPSIS;
            ++tok.val.count;
            ++l->pos;
        }
    }
    
    return tok;
}

Token get_plus_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_PLUS,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '+') {
        tok.kind = TOKEN_PLUSPLUS;
        ++tok.val.count;
        ++l->pos;
    }
    
    return tok;
}

Token get_minus_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_MINUS,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '-') {
        tok.kind = TOKEN_MINUSMINUS;
        ++tok.val.count;
        ++l->pos;
    }
    
    return tok;
}

Token get_and_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_AND,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '&') {
        tok.kind = TOKEN_ANDAND;
        ++tok.val.count;
        ++l->pos;
    }
    
    return tok;
}

Token get_or_token(Lexer *l) {
    Token tok = {
        .kind = TOKEN_OR,
        .val = (String_View){
            .items = l->source.items + l->pos,
            .count = 1,
        },
    };
    
    ++l->pos;
    if (INBOUNDS(l) && CURRENT(l) == '|') {
        tok.kind = TOKEN_OROR;
        ++tok.val.count;
        ++l->pos;
    }
    
    return tok;
}

void skip_multi_line_comment(Lexer *l) {
    ++l->pos;

    while (INBOUNDS(l)) {
        if (CURRENT(l) == '~') {
            ++l->pos;
            if (CURRENT(l) == '#') {
                ++l->pos;
            }

            break;
        }
        
        ++l->pos;
    }
}

void skip_comment(Lexer *l) {
    ++l->pos;

    if (INBOUNDS(l) && CURRENT(l) == '~') {
        skip_multi_line_comment(l);
        return;
    }

    while (INBOUNDS(l) && CURRENT(l) != '\n') {
        ++l->pos;
    }
}

Token lexer_next(Lexer *l) {
    skip_whitespaces(l);
    
    if (!INBOUNDS(l)) {
        l->cur = (Token){
            .val = {
                .items = l->source.items + l->pos,
                .count = 0,
            },
            .kind = TOKEN_EOF,
        };
        return l->cur;
    }
    
    Token tok;
    
    switch (CURRENT(l)) {
        case '+': {
            tok = get_plus_token(l);
        } break;

        case '-': {
            tok = get_minus_token(l);
        } break;
        
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
        } break;

        case '!': {
            tok = get_not_token(l);
        } break;

        case '&': {
            tok = get_and_token(l);
        } break;

        case '|': {
            tok = get_or_token(l);
        } break;

        case '#': {
            while (INBOUNDS(l) && CURRENT(l) == '#') {
                skip_comment(l);
            }
            tok = lexer_next(l);
        } break;
            
        default: {
            tok = get_token(l);
        } break;
    }

    if (tok.val.count == 0 && tok.kind != TOKEN_EOF) {
        tok = get_lit_token(l);
    }
    
    l->cur = tok;
    return tok;
}

void lexer_init(Lexer *l) {
    l->skipped.items = l->source.items;
    l->skipped.count = 0;
    
    lexer_next(l);
}