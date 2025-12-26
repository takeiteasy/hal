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

#ifndef HAL_NO_SHELL
#include "shell_common.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

/* Helper functions */
static void _perror(const char *msg) {
    fprintf(stderr, "shell: %s: %s\n", msg, strerror(errno));
}

/* Command execution */
typedef struct shell_command {
    int argc;
    char **argv;
    int input_fd;
    int output_fd;
    int bg;
} shell_command_t;

static int g_input_fd = -1;
static int g_output_fd = -1;
static int g_bg = 0;

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

static int shell_exec(shell_ast_t *ast);

static void eval_commandtail(shell_ast_t *ast) {
    shell_command_t cmd;
    command_argv_from_ast(&cmd.argc, &cmd.argv, ast);
    cmd.input_fd = g_input_fd;
    cmd.output_fd = g_output_fd;
    cmd.bg = g_bg;
    command_execute(&cmd);
    command_argv_free(cmd.argc, cmd.argv);
}

static void eval_sequence(shell_ast_t *ast) {
    if (ast->type == SHELL_AST_BACKGROUND)
        g_bg = 1;
    shell_exec(ast->left);
    shell_exec(ast->right);
    if (ast->type == SHELL_AST_BACKGROUND)
        g_bg = 0;
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
        g_input_fd = fd;
    } else {
        fd = open((const char*)ast->token->begin,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) {
            _perror("open");
            return;
        }
        g_output_fd = fd;
    }
    ast->token->begin[ast->token->length] = c;

    shell_exec(ast->right);
    close(fd);
    g_input_fd = -1;
    g_output_fd = -1;
}

static void eval_pipeline(shell_ast_t *ast) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        _perror("pipe");
        return;
    }

    g_input_fd = -1;
    g_output_fd = pipefd[1];
    shell_exec(ast->left);
    close(pipefd[1]);

    ast = ast->right;

    g_input_fd = pipefd[0];

    while (ast->type == SHELL_AST_PIPE) {
        if (pipe(pipefd) == -1) {
            _perror("pipe");
            return;
        }
        g_output_fd = pipefd[1];
        shell_exec(ast->left);
        close(pipefd[1]);
        close(g_input_fd);
        g_input_fd = pipefd[0];
        ast = ast->right;
    }
    g_output_fd = -1;
    g_input_fd = pipefd[0];
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
        shell_lexer_init(&lexer, (unsigned char*)cmd);
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
    shell_lexer_init(&lexer, (unsigned char*)cmd);
    
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
