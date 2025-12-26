/* https://github.com/takeiteasy/hal

 hal Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef SHELL_COMMON_INCLUDED
#define SHELL_COMMON_INCLUDED

#include "hal/shell.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

/* Shell token types */
typedef enum shell_token_type {
    SHELL_TOKEN_ERROR,
    SHELL_TOKEN_EOL,
    SHELL_TOKEN_ATOM,
    SHELL_TOKEN_PIPE = '|',
    SHELL_TOKEN_AMPERSAND = '&',
    SHELL_TOKEN_GREATER = '>',
    SHELL_TOKEN_LESSER = '<',
    SHELL_TOKEN_SEMICOLON = ';'
} shell_token_type;

typedef struct shell_token {
    shell_token_type type;
    unsigned char *begin;
    int length;
} shell_token_t;

typedef struct shell_lexer {
    unsigned char *begin;
    struct {
        unsigned char *ptr;
        wchar_t ch;
        int ch_length;
    } cursor;
    char *error;
    unsigned char *input_begin;
    size_t error_pos;
} shell_lexer_t;

typedef enum shell_ast_type {
    SHELL_AST_CMD,
    SHELL_AST_BACKGROUND,
    SHELL_AST_SEQ,
    SHELL_AST_REDIR_IN,
    SHELL_AST_REDIR_OUT,
    SHELL_AST_PIPE
} shell_ast_type_t;

typedef struct shell_ast {
    shell_ast_type_t type;
    shell_token_t *token;
    struct shell_ast *left, *right;
} shell_ast_t;

typedef struct shell_token_array {
    shell_token_t *data;
    size_t count;
    size_t capacity;
} shell_token_array_t;

typedef struct shell_parser {
    shell_token_array_t tokens;
    shell_token_t *current;
    size_t cursor;
} shell_parser;

/* Helper functions */
static void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

static void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

/* UTF-8 reading */
static int utf8read(const unsigned char* c, wchar_t* out) {
    wchar_t u = *c, l = 1;
    if ((u & 0xC0) == 0xC0) {
        int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
        if (a < 6 || !(u & 0x02)) {
            u = ((u << (a + 1)) & 0xFF) >> (a + 1);
            for (int b = 1; b < a; ++b)
                u = (u << 6) | (c[l++] & 0x3F);
        }
    }
    if (out)
        *out = u;
    return l;
}

/* Lexer functions */
static void shell_lexer_init(shell_lexer_t *l, unsigned char* line) {
    l->begin = line;
    l->cursor.ptr = line;
    l->cursor.ch_length = utf8read(line, &l->cursor.ch);
    l->error = NULL;
    l->input_begin = line;
    l->error_pos = (size_t)-1;
}

static inline wchar_t peek(shell_lexer_t *l) {
    return l->cursor.ch;
}

static inline bool is_eof(shell_lexer_t *l) {
    return peek(l) == '\0';
}

static inline void update(shell_lexer_t *l) {
    l->begin = l->cursor.ptr;
    l->cursor.ch_length = utf8read(l->cursor.ptr, &l->cursor.ch);
    l->error = NULL;
}

static inline wchar_t advance(shell_lexer_t *l) {
    l->cursor.ptr += utf8read(l->cursor.ptr, NULL);
    l->cursor.ch_length = utf8read(l->cursor.ptr, &l->cursor.ch);
    return l->cursor.ch;
}

static void skip_whitespace(shell_lexer_t *l) {
    for (;;) {
        if (is_eof(l))
            return;
        switch (peek(l)) {
            case ' ':
            case '\t':
            case '\v':
            case '\r':
            case '\n':
            case '\f':
                advance(l);
                break;
            default:
                return;
        }
    }
}

static shell_token_t new_token(shell_lexer_t *l, shell_token_type type) {
    return (shell_token_t) {
        .type = type,
        .begin = l->begin,
        .length = (int)(l->cursor.ptr - l->begin)
    };
}

static shell_token_t read_atom(shell_lexer_t *l) {
    for (;;) {
        if (is_eof(l))
            goto BAIL;
        switch (peek(l)) {
            case ' ':
            case '\t':
            case '\v':
            case '\r':
            case '\n':
            case '\f':
            case '|':
            case '&':
            case '<':
            case '>':
            case ';':
                goto BAIL;
            default:
                advance(l);
                break;
        }
    }
BAIL:
    return new_token(l, SHELL_TOKEN_ATOM);
}

static shell_token_t read_token(shell_lexer_t *l) {
    if (is_eof(l))
        return new_token(l, SHELL_TOKEN_EOL);
    update(l);
    wchar_t wc = peek(l);
    switch (wc) {
        case ' ':
        case '\t':
        case '\v':
        case '\r':
        case '\n':
        case '\f':
            skip_whitespace(l);
            update(l);
            return read_token(l);
        case '"':
        case '\'': {
            wchar_t quote = wc;
            unsigned char *start = l->cursor.ptr + l->cursor.ch_length;
            unsigned char *p = start;
            for (;;) {
                wchar_t ch;
                int len = utf8read(p, &ch);
                if (ch == '\0') {
                    l->error = "unterminated quote";
                    l->begin = start;
                    l->cursor.ptr = p;
                    if (l->input_begin)
                        l->error_pos = (size_t)(p - l->input_begin);
                    return new_token(l, SHELL_TOKEN_ERROR);
                }
                if (ch == quote) {
                    l->begin = start;
                    l->cursor.ptr = p;
                    shell_token_t tok = new_token(l, SHELL_TOKEN_ATOM);
                    advance(l);
                    return tok;
                }
                p += len;
            }
        }
        case '|':
        case '&':
        case '<':
        case '>':
        case ';':
            advance(l);
            return new_token(l, (shell_token_type)wc);
        default:
            return read_atom(l);
    }
    return new_token(l, SHELL_TOKEN_ERROR);
}

/* Token array functions */
static void shell_token_array_init(shell_token_array_t *arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static bool shell_token_array_append(shell_token_array_t *arr, shell_token_t token) {
    if (arr->count + 1 > arr->capacity) {
        size_t new_capacity = arr->capacity == 0 ? 8 : arr->capacity * 2;
        shell_token_t *new_data = (shell_token_t*)xrealloc(arr->data, sizeof(shell_token_t) * new_capacity);
        if (!new_data)
            return false;
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
    arr->data[arr->count++] = token;
    return true;
}

static shell_token_array_t shell_parse(shell_lexer_t *l) {
    shell_token_array_t tokens;
    shell_token_array_init(&tokens);
    for (;;) {
        shell_token_t token = read_token(l);
        switch (token.type) {
            default:
                l->error = "unknown token";
            case SHELL_TOKEN_ERROR:
            case SHELL_TOKEN_EOL:
                goto BAIL;
            case SHELL_TOKEN_ATOM:
            case SHELL_TOKEN_PIPE:
            case SHELL_TOKEN_AMPERSAND:
            case SHELL_TOKEN_GREATER:
            case SHELL_TOKEN_LESSER:
            case SHELL_TOKEN_SEMICOLON:
                if (!shell_token_array_append(&tokens, token))
                    l->error = "out of memory";
                break;
        }
    }
BAIL:
    return tokens;
}

/* AST functions */
static inline shell_ast_t* new_ast(void) {
    shell_ast_t *result = (shell_ast_t*)malloc(sizeof(shell_ast_t));
    memset(result, 0, sizeof(shell_ast_t));
    return result;
}

static void free_ast(shell_ast_t *node) {
    if (!node)
        return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Parser functions */
static shell_token_t* parser_peek(shell_parser *p) {
    return p->cursor < p->tokens.count ? &p->tokens.data[p->cursor] : NULL;
}

static shell_token_t* parser_next(shell_parser *p) {
    if (p->cursor + 1 >= p->tokens.count)
        return NULL;
    ++p->cursor;
    return &p->tokens.data[p->cursor];
}

static int match_token(shell_parser *p, shell_token_type type) {
    shell_token_t *t = parser_peek(p);
    return t != NULL && t->type == type;
}

static int expect_token(shell_parser *p, shell_token_type type) {
    shell_token_t *t = parser_peek(p);
    return t != NULL && t->type == type;
}

static shell_ast_t *simple_command(shell_parser *p) {
    shell_token_t *t = parser_peek(p);
    if (!t || t->type != SHELL_TOKEN_ATOM)
        return NULL;
    shell_ast_t *ast = new_ast();
    ast->type = SHELL_AST_CMD;
    ast->token = t;
    if (parser_next(p) != NULL)
        ast->right = simple_command(p);
    return ast;
}

static inline shell_ast_t *handle_redirection(shell_parser *p, shell_ast_type_t type, shell_ast_t *simple) {
    if (!expect_token(p, SHELL_TOKEN_ATOM))
        return NULL;
    shell_ast_t *ast = new_ast();
    ast->type = type;
    ast->right = simple;
    ast->token = parser_next(p);
    parser_next(p);
    return ast;
}

static shell_ast_t *command(shell_parser *p) {
    shell_ast_t *simple = simple_command(p);
    if (!simple)
        return NULL;
    if (match_token(p, SHELL_TOKEN_GREATER)) {
        shell_ast_t *ast = handle_redirection(p, SHELL_AST_REDIR_OUT, simple);
        if (ast == NULL)
            free_ast(simple);
        return ast;
    }
    if (match_token(p, SHELL_TOKEN_LESSER)) {
        shell_ast_t *ast = handle_redirection(p, SHELL_AST_REDIR_IN, simple);
        if (!ast)
            free_ast(simple);
        return ast;
    }
    return simple;
}

static shell_ast_t *_pipe(shell_parser *p);

static shell_ast_t *_pipe(shell_parser *p) {
    shell_ast_t *left = command(p);
    if (left == NULL)
        return NULL;
    if (match_token(p, SHELL_TOKEN_PIPE)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM)) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_PIPE;
        ast->left = left;
        parser_next(p);
        ast->right = _pipe(p);
        return ast;
    }
    return left;
}

static shell_ast_t *full_command(shell_parser *p);

static shell_ast_t *full_command(shell_parser *p) {
    shell_ast_t *left = _pipe(p);
    if (left == NULL)
        return NULL;
    if (match_token(p, SHELL_TOKEN_AMPERSAND)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM) && parser_peek(p) != NULL) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_BACKGROUND;
        ast->left = left;
        parser_next(p);
        ast->right = full_command(p);
        return ast;
    }
    if (match_token(p, SHELL_TOKEN_SEMICOLON)) {
        if (!expect_token(p, SHELL_TOKEN_ATOM) && parser_peek(p) != NULL) {
            free_ast(left);
            return NULL;
        }
        shell_ast_t *ast = new_ast();
        ast->type = SHELL_AST_SEQ;
        ast->left = left;
        parser_next(p);
        ast->right = full_command(p);
        return ast;
    }
    return left;
}

static shell_ast_t* shell_eval(shell_token_array_t tokens) {
    shell_parser parser = {
        .tokens = tokens,
        .cursor = 0
    };
    shell_ast_t *result = full_command(&parser);
    return tokens.count == 0 ? NULL : parser.cursor == tokens.count - 1 ? result : NULL;
}

/* Common helper for building argv from AST */
static void command_argv_from_ast(int *argc, char ***argv, shell_ast_t *ast) {
    *argc = 0;
    *argv = NULL;

    for (shell_ast_t *n = ast; n != NULL; n = n->right)
        (*argc)++;

    if (*argc == 0) {
        *argv = (char **)xmalloc(sizeof(char *));
        (*argv)[0] = NULL;
        return;
    }

    *argv = (char **)xmalloc(sizeof(char *) * (*argc + 1));

    for (int i = 0; i < *argc; i++) {
        if (ast == NULL || ast->token == NULL) {
            (*argv)[i] = (char *)xmalloc(1);
            (*argv)[i][0] = '\0';
        } else {
            size_t len = (size_t)ast->token->length;
            size_t size = len + 1;
            (*argv)[i] = (char *)xmalloc(size);
            memcpy((*argv)[i], ast->token->begin, len);
            (*argv)[i][len] = '\0';
        }
        ast = ast ? ast->right : NULL;
    }
    (*argv)[*argc] = NULL;
}

static void command_argv_free(int argc, char **argv) {
    if (argv) {
        for (int i = 0; i < argc; i++)
            free(argv[i]);
        free(argv);
    }
}

static int ensure_buffer_capacity(char **buffer, size_t *capacity, size_t current_len, size_t needed) {
    if (current_len + needed <= *capacity)
        return 0;
    size_t new_capacity = (*capacity == 0) ? 4096 : *capacity * 2;
    while (new_capacity - current_len < needed)
        new_capacity *= 2;
    
    *buffer = (char*)xrealloc(*buffer, new_capacity + 1);
    *capacity = new_capacity;
    return 0;
}

#endif /* SHELL_COMMON_INCLUDED */
