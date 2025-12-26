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

/* Windows shell implementation using CreateProcess */

#ifndef HAL_NO_SHELL
#include "../shell_common.c"
#include <windows.h>

/* Build command line from argv */
static char* build_command_line(int argc, char **argv) {
    if (argc == 0 || !argv || !argv[0])
        return NULL;
    
    /* Calculate total length needed */
    size_t total_len = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            total_len += strlen(argv[i]) + 3; /* space + potential quotes */
        }
    }
    
    char *cmdline = (char*)xmalloc(total_len + 1);
    cmdline[0] = '\0';
    
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            if (i > 0)
                strcat(cmdline, " ");
            /* Add quotes if arg contains spaces */
            if (strchr(argv[i], ' ')) {
                strcat(cmdline, "\"");
                strcat(cmdline, argv[i]);
                strcat(cmdline, "\"");
            } else {
                strcat(cmdline, argv[i]);
            }
        }
    }
    return cmdline;
}

/* Execute simple command using CreateProcess */
static int execute_simple_command(int argc, char **argv, HANDLE in_handle, HANDLE out_handle) {
    if (argc == 0 || !argv || !argv[0])
        return HAL_SHELL_ERR_GENERIC;
    
    char *cmdline = build_command_line(argc, argv);
    if (!cmdline)
        return HAL_SHELL_ERR_GENERIC;
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    
    if (in_handle != INVALID_HANDLE_VALUE || out_handle != INVALID_HANDLE_VALUE) {
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = in_handle != INVALID_HANDLE_VALUE ? in_handle : GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = out_handle != INVALID_HANDLE_VALUE ? out_handle : GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }
    
    BOOL success = CreateProcessA(
        NULL,           /* Application name (use cmdline) */
        cmdline,        /* Command line */
        NULL,           /* Process security attributes */
        NULL,           /* Thread security attributes */
        TRUE,           /* Inherit handles */
        0,              /* Creation flags */
        NULL,           /* Environment */
        NULL,           /* Current directory */
        &si,            /* Startup info */
        &pi             /* Process info */
    );
    
    free(cmdline);
    
    if (!success)
        return HAL_SHELL_ERR_EXEC;
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return (int)exit_code;
}

/* Shell execution state */
static HANDLE g_input_handle = INVALID_HANDLE_VALUE;
static HANDLE g_output_handle = INVALID_HANDLE_VALUE;

static int shell_exec(shell_ast_t *ast);

static void eval_commandtail(shell_ast_t *ast) {
    int argc;
    char **argv;
    command_argv_from_ast(&argc, &argv, ast);
    execute_simple_command(argc, argv, g_input_handle, g_output_handle);
    command_argv_free(argc, argv);
}

static void eval_sequence(shell_ast_t *ast) {
    shell_exec(ast->left);
    shell_exec(ast->right);
}

static void eval_redirection(shell_ast_t *ast) {
    HANDLE handle;
    unsigned char c = ast->token->begin[ast->token->length];
    ast->token->begin[ast->token->length] = '\0';
    
    if (ast->type == SHELL_AST_REDIR_IN) {
        handle = CreateFileA(
            (const char*)ast->token->begin,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (handle == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "shell: open: %s\n", (const char*)ast->token->begin);
            return;
        }
        g_input_handle = handle;
    } else {
        handle = CreateFileA(
            (const char*)ast->token->begin,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (handle == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "shell: open: %s\n", (const char*)ast->token->begin);
            return;
        }
        g_output_handle = handle;
    }
    ast->token->begin[ast->token->length] = c;

    shell_exec(ast->right);
    CloseHandle(handle);
    g_input_handle = INVALID_HANDLE_VALUE;
    g_output_handle = INVALID_HANDLE_VALUE;
}

static void eval_pipeline(shell_ast_t *ast) {
    HANDLE read_pipe, write_pipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        fprintf(stderr, "shell: pipe failed\n");
        return;
    }
    
    g_input_handle = INVALID_HANDLE_VALUE;
    g_output_handle = write_pipe;
    shell_exec(ast->left);
    CloseHandle(write_pipe);
    
    ast = ast->right;
    g_input_handle = read_pipe;
    
    while (ast->type == SHELL_AST_PIPE) {
        if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
            fprintf(stderr, "shell: pipe failed\n");
            return;
        }
        g_output_handle = write_pipe;
        shell_exec(ast->left);
        CloseHandle(write_pipe);
        CloseHandle(g_input_handle);
        g_input_handle = read_pipe;
        ast = ast->right;
    }
    
    g_output_handle = INVALID_HANDLE_VALUE;
    shell_exec(ast);
    CloseHandle(read_pipe);
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

/* Execute with I/O capture */
static int windows_shell_with_io(const char *cmd, hal_shell_io_t *io) {
    HANDLE in_read = INVALID_HANDLE_VALUE, in_write = INVALID_HANDLE_VALUE;
    HANDLE out_read = INVALID_HANDLE_VALUE, out_write = INVALID_HANDLE_VALUE;
    HANDLE err_read = INVALID_HANDLE_VALUE, err_write = INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    
    /* Create pipes */
    if (!CreatePipe(&in_read, &in_write, &sa, 0) ||
        !CreatePipe(&out_read, &out_write, &sa, 0) ||
        !CreatePipe(&err_read, &err_write, &sa, 0)) {
        if (in_read != INVALID_HANDLE_VALUE) CloseHandle(in_read);
        if (in_write != INVALID_HANDLE_VALUE) CloseHandle(in_write);
        if (out_read != INVALID_HANDLE_VALUE) CloseHandle(out_read);
        if (out_write != INVALID_HANDLE_VALUE) CloseHandle(out_write);
        if (err_read != INVALID_HANDLE_VALUE) CloseHandle(err_read);
        if (err_write != INVALID_HANDLE_VALUE) CloseHandle(err_write);
        return HAL_SHELL_ERR_PIPE;
    }
    
    /* Make read ends non-inheritable */
    SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(err_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0);
    
    /* Build command line for cmd.exe */
    size_t cmd_len = strlen(cmd);
    char *full_cmd = (char*)xmalloc(cmd_len + 16);
    sprintf(full_cmd, "cmd.exe /c %s", cmd);
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = in_read;
    si.hStdOutput = out_write;
    si.hStdError = err_write;
    
    BOOL success = CreateProcessA(
        NULL, full_cmd, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    );
    
    free(full_cmd);
    
    /* Close child-side handles */
    CloseHandle(in_read);
    CloseHandle(out_write);
    CloseHandle(err_write);
    
    if (!success) {
        CloseHandle(in_write);
        CloseHandle(out_read);
        CloseHandle(err_read);
        return HAL_SHELL_ERR_EXEC;
    }
    
    /* Write stdin if provided */
    if (io->in && io->in_len > 0) {
        DWORD written;
        WriteFile(in_write, io->in, (DWORD)io->in_len, &written, NULL);
    }
    CloseHandle(in_write);
    
    /* Read stdout and stderr */
    char *outbuf = NULL, *errbuf = NULL;
    size_t out_len = 0, out_cap = 0, err_len = 0, err_cap = 0;
    int use_out_cb = io->out_cb != NULL;
    int use_err_cb = io->err_cb != NULL;
    
    char buffer[4096];
    DWORD bytes_read;
    
    /* Read stdout */
    while (ReadFile(out_read, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0) {
        if (use_out_cb)
            io->out_cb(buffer, bytes_read, io->userdata);
        else {
            ensure_buffer_capacity(&outbuf, &out_cap, out_len, bytes_read);
            memcpy(outbuf + out_len, buffer, bytes_read);
            out_len += bytes_read;
        }
    }
    
    /* Read stderr */
    while (ReadFile(err_read, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0) {
        if (use_err_cb)
            io->err_cb(buffer, bytes_read, io->userdata);
        else {
            ensure_buffer_capacity(&errbuf, &err_cap, err_len, bytes_read);
            memcpy(errbuf + err_len, buffer, bytes_read);
            err_len += bytes_read;
        }
    }
    
    CloseHandle(out_read);
    CloseHandle(err_read);
    
    /* Set output buffers */
    if (!use_out_cb) {
        io->out = outbuf ? (outbuf[out_len] = '\0', outbuf) : (outbuf = (char*)xmalloc(1), outbuf[0] = '\0', outbuf);
        io->out_len = out_len;
    } else {
        io->out = NULL;
        io->out_len = 0;
    }
    
    if (!use_err_cb) {
        io->err = errbuf ? (errbuf[err_len] = '\0', errbuf) : (errbuf = (char*)xmalloc(1), errbuf[0] = '\0', errbuf);
        io->err_len = err_len;
    } else {
        io->err = NULL;
        io->err_len = 0;
    }
    
    /* Wait for process and get exit code */
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return (int)exit_code;
}

static int windows_shell_inline(const char *cmd) {
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
    return io ? windows_shell_with_io(cmd, io) : windows_shell_inline(cmd);
}

int hal_shell_fmt(hal_shell_io_t *io, const char *fmt, ...) {
    if (!fmt)
        return HAL_SHELL_ERR_GENERIC;

    va_list args;
    va_start(args, fmt);
    
    /* Windows doesn't have vasprintf, use vsnprintf */
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (len < 0) {
        va_end(args);
        return HAL_SHELL_ERR_GENERIC;
    }
    
    char *cmd = (char*)xmalloc(len + 1);
    vsnprintf(cmd, len + 1, fmt, args);
    va_end(args);

    int result = hal_shell(cmd, io);
    free(cmd);
    return result;
}

#endif /* HAL_NO_SHELL */
