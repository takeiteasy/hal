/* https://github.com/takeiteasy/hal
   POSIX shell implementation */

#ifndef HAL_NO_SHELL
#include "hal/shell.h"
#include "hal/filesystem.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <signal.h>

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
static void _perror(const char *msg) {
    fprintf(stderr, "shell: %s: %s\n", msg, strerror(errno));
}

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
static void shell_lexer(shell_lexer_t *l, unsigned char* line) {
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

/* Command execution */
typedef struct shell_command {
    int argc;
    char **argv;
    int input_fd;
    int output_fd;
    int bg;
} shell_command_t;

static int input_fd = -1;
static int output_fd = -1;
static int bg = 0;

static void command_argv_from_ast(shell_command_t *cmd, shell_ast_t *ast) {
    cmd->argc = 0;
    cmd->argv = NULL;

    for (shell_ast_t *n = ast; n != NULL; n = n->right)
        cmd->argc++;

    if (cmd->argc == 0) {
        cmd->argv = (char **)xmalloc(sizeof(char *));
        cmd->argv[0] = NULL;
        return;
    }

    cmd->argv = (char **)xmalloc(sizeof(char *) * (cmd->argc + 1));

    for (int i = 0; i < cmd->argc; i++) {
        if (ast == NULL || ast->token == NULL) {
            cmd->argv[i] = (char *)xmalloc(1);
            cmd->argv[i][0] = '\0';
        } else {
            size_t len = (size_t)ast->token->length;
            size_t size = len + 1;
            cmd->argv[i] = (char *)xmalloc(size);
            memcpy(cmd->argv[i], ast->token->begin, len);
            cmd->argv[i][len] = '\0';
        }
        ast = ast ? ast->right : NULL;
    }
    cmd->argv[cmd->argc] = NULL;
}

static void command_execute(shell_command_t *cmd) {
    if (cmd->argc == 0)
        return;

    pid_t pid = fork();
    if (pid == -1) {
        _perror("fork");
        return;
    }

    if (pid == 0) {
        if (cmd->bg) {
            setpgid(0, 0);
            int fd = open("/dev/null", O_RDONLY, 0);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (cmd->input_fd != -1) {
            dup2(cmd->input_fd, STDIN_FILENO);
            close(cmd->input_fd);
        }
        if (cmd->output_fd != -1) {
            dup2(cmd->output_fd, STDOUT_FILENO);
            close(cmd->output_fd);
        }

        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            perror("execvp");
            _exit(EXIT_FAILURE);
        }
    }
    
    if (!cmd->bg) {
        int status;
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

static void command_destroy(shell_command_t *cmd) {
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++)
            free(cmd->argv[i]);
        free(cmd->argv);
    }
}

static int shell_exec(shell_ast_t *ast);

static void eval_commandtail(shell_ast_t *ast) {
    shell_command_t cmd;
    command_argv_from_ast(&cmd, ast);
    cmd.input_fd = input_fd;
    cmd.output_fd = output_fd;
    cmd.bg = bg;
    command_execute(&cmd);
    command_destroy(&cmd);
}

static void eval_sequence(shell_ast_t *ast) {
    if (ast->type == SHELL_AST_BACKGROUND)
        bg = 1;
    shell_exec(ast->left);
    shell_exec(ast->right);
    if (ast->type == SHELL_AST_BACKGROUND)
        bg = 0;
}

static void eval_redirection(shell_ast_t *ast) {
    int fd;
    unsigned char c = ast->token->begin[ast->token->length];
    ast->token->begin[ast->token->length] = '\0';
    if (ast->type == SHELL_AST_REDIR_IN) {
        fd = open((const char*)ast->token->begin, O_RDONLY);
        if (fd == -1) {
            _perror("open");
            return;
        }
        input_fd = fd;
    } else {
        fd = open((const char*)ast->token->begin,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) {
            _perror("open");
            return;
        }
        output_fd = fd;
    }
    ast->token->begin[ast->token->length] = c;

    shell_exec(ast->right);
    close(fd);
    input_fd = -1;
    output_fd = -1;
}

static void eval_pipeline(shell_ast_t *ast) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        _perror("pipe");
        return;
    }

    input_fd = -1;
    output_fd = pipefd[1];
    shell_exec(ast->left);
    close(pipefd[1]);

    ast = ast->right;

    input_fd = pipefd[0];

    while (ast->type == SHELL_AST_PIPE) {
        if (pipe(pipefd) == -1) {
            _perror("pipe");
            return;
        }
        output_fd = pipefd[1];
        shell_exec(ast->left);
        close(pipefd[1]);
        close(input_fd);
        input_fd = pipefd[0];
        ast = ast->right;
    }
    output_fd = -1;
    input_fd = pipefd[0];
    shell_exec(ast);
    close(pipefd[0]);
}

static int shell_exec(shell_ast_t *ast) {
    if (!ast)
        return 0;
    switch (ast->type) {
        case SHELL_AST_PIPE:
            eval_pipeline(ast);
            break;
        case SHELL_AST_REDIR_IN:
        case SHELL_AST_REDIR_OUT:
            eval_redirection(ast);
            break;
        case SHELL_AST_SEQ:
        case SHELL_AST_BACKGROUND:
            eval_sequence(ast);
            break;
        case SHELL_AST_CMD:
            eval_commandtail(ast);
            break;
        default:
            break;
    }
    return 0;
}

/* Helper functions for I/O */
static void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

static void close_pipe_pair(int *fds) {
    if (fds[0] != -1) { close(fds[0]); fds[0] = -1; }
    if (fds[1] != -1) { close(fds[1]); fds[1] = -1; }
}

static void write_input_to_child(int fd, const char *data, size_t len) {
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, data + written, len - written);
        if (n <= 0)
            break;
        written += n;
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

static int posix_shell_with_io(const char *cmd, hal_shell_io_t *io) {
    int pipes[6] = {-1, -1, -1, -1, -1, -1};
    int *inpipe = &pipes[0], *outpipe = &pipes[2], *errpipe = &pipes[4];
    
    if (pipe(inpipe) == -1 || pipe(outpipe) == -1 || pipe(errpipe) == -1) {
        for (int i = 0; i < 6; i += 2)
            close_pipe_pair(&pipes[i]);
        return HAL_SHELL_ERR_PIPE;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        for (int i = 0; i < 6; i += 2)
            close_pipe_pair(&pipes[i]);
        return HAL_SHELL_ERR_FORK;
    }
    
    if (pid == 0) {
        close(inpipe[1]);
        close(outpipe[0]);
        close(errpipe[0]);
        
        if (dup2(inpipe[0], STDIN_FILENO) == -1 ||
            dup2(outpipe[1], STDOUT_FILENO) == -1 ||
            dup2(errpipe[1], STDERR_FILENO) == -1) {
            _exit(HAL_SHELL_ERR_EXEC);
        }
        
        close(inpipe[0]);
        close(outpipe[1]);
        close(errpipe[1]);
        
        shell_lexer_t lexer;
        shell_lexer(&lexer, (unsigned char*)cmd);
        shell_token_array_t tokens = shell_parse(&lexer);
        
        if (!tokens.data || lexer.error) {
            if (tokens.data)
                free(tokens.data);
            _exit(HAL_SHELL_ERR_TOKENIZE);
        }
        
        shell_ast_t *ast = shell_eval(tokens);
        if (!ast) {
            free(tokens.data);
            _exit(HAL_SHELL_ERR_EVAL);
        }
        
        int result = shell_exec(ast);
        free(tokens.data);
        free_ast(ast);
        _exit(result == 0 ? HAL_SHELL_OK : result);
    }
    
    close(inpipe[0]);
    close(outpipe[1]);
    close(errpipe[1]);
    
    set_nonblocking(inpipe[1]);
    set_nonblocking(outpipe[0]);
    set_nonblocking(errpipe[0]);
    
    if (io->in && io->in_len > 0)
        write_input_to_child(inpipe[1], io->in, io->in_len);
    close(inpipe[1]);
    
    char *outbuf = NULL, *errbuf = NULL;
    size_t out_len = 0, out_cap = 0, err_len = 0, err_cap = 0;
    int use_out_cb = io->out_cb != NULL;
    int use_err_cb = io->err_cb != NULL;
    
    struct pollfd fds[2] = {
        {.fd = outpipe[0], .events = POLLIN | POLLHUP},
        {.fd = errpipe[0], .events = POLLIN | POLLHUP}
    };
    
    int active_fds = 2;
    while (active_fds > 0) {
        int poll_result = poll(fds, 2, -1);
        if (poll_result == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        
        for (int i = 0; i < 2; i++) {
            if (!(fds[i].revents & (POLLIN | POLLHUP)) || fds[i].fd == -1)
                continue;

            char buffer[4096];
            ssize_t bytes_read;
            
            while ((bytes_read = read(fds[i].fd, buffer, sizeof(buffer))) > 0) {
                if (i == 0) {
                    if (use_out_cb)
                        io->out_cb(buffer, bytes_read, io->userdata);
                    else {
                        ensure_buffer_capacity(&outbuf, &out_cap, out_len, bytes_read);
                        memcpy(outbuf + out_len, buffer, bytes_read);
                        out_len += bytes_read;
                    }
                } else {
                    if (use_err_cb)
                        io->err_cb(buffer, bytes_read, io->userdata);
                    else {
                        ensure_buffer_capacity(&errbuf, &err_cap, err_len, bytes_read);
                        memcpy(errbuf + err_len, buffer, bytes_read);
                        err_len += bytes_read;
                    }
                }
            }
            
            if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN && errno != EINTR)) {
                fds[i].fd = -1;
                active_fds--;
            }
        }
    }
    
    if (!use_out_cb) {
        io->out = outbuf ? (outbuf[out_len] = '\0', outbuf) : (outbuf = (char *)xmalloc(1), outbuf[0] = '\0', outbuf);
        io->out_len = out_len;
    } else {
        io->out = NULL;
        io->out_len = 0;
    }
    
    if (!use_err_cb) {
        io->err = errbuf ? (errbuf[err_len] = '\0', errbuf) : (errbuf = (char *)xmalloc(1), errbuf[0] = '\0', errbuf);
        io->err_len = err_len;
    } else {
        io->err = NULL;
        io->err_len = 0;
    }
    
    close(outpipe[0]);
    close(errpipe[0]);
    
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int posix_shell_inline(const char *cmd) {
    shell_lexer_t lexer;
    shell_lexer(&lexer, (unsigned char*)cmd);
    
    shell_token_array_t tokens = shell_parse(&lexer);
    if (!tokens.data || lexer.error) {
        if (tokens.data)
            free(tokens.data);
        return HAL_SHELL_ERR_TOKENIZE;
    }
    
    shell_ast_t *ast = shell_eval(tokens);
    if (!ast) {
        free(tokens.data);
        return HAL_SHELL_ERR_EVAL;
    }
    
    int result = shell_exec(ast);
    free(tokens.data);
    free_ast(ast);
    return result;
}

/* Public API */
int hal_shell(const char *cmd, hal_shell_io_t *io) {
    if (!cmd)
        return HAL_SHELL_ERR_GENERIC;
    return io ? posix_shell_with_io(cmd, io) : posix_shell_inline(cmd);
}

int hal_shell_fmt(hal_shell_io_t *io, const char *fmt, ...) {
    if (!fmt)
        return HAL_SHELL_ERR_GENERIC;

    va_list args;
    va_start(args, fmt);
    char *cmd = NULL;
    if (vasprintf(&cmd, fmt, args) == -1) {
        va_end(args);
        return HAL_SHELL_ERR_GENERIC;
    }
    va_end(args);

    int result = hal_shell(cmd, io);
    free(cmd);
    return result;
}

#endif /* HAL_NO_SHELL */
