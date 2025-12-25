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

/*!
 @header shell.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Shell command execution with I/O capture
*/

#ifndef HAL_SHELL_HEAD
#define HAL_SHELL_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_SHELL
#include "hal.h"
#include <stddef.h>

/*!
 @enum hal_shell_err
 @constant HAL_SHELL_OK Success
 @constant HAL_SHELL_ERR_GENERIC Generic error
 @constant HAL_SHELL_ERR_FORK Fork failed
 @constant HAL_SHELL_ERR_PIPE Pipe creation failed
 @constant HAL_SHELL_ERR_EXEC Exec failed
 @constant HAL_SHELL_ERR_TOKENIZE Tokenization failed
 @constant HAL_SHELL_ERR_EVAL Evaluation failed
 @discussion Shell error codes (negative values from hal_shell())
*/
typedef enum hal_shell_err {
    HAL_SHELL_OK = 0,
    HAL_SHELL_ERR_GENERIC = -1,
    HAL_SHELL_ERR_FORK = -2,
    HAL_SHELL_ERR_PIPE = -3,
    HAL_SHELL_ERR_EXEC = -4,
    HAL_SHELL_ERR_TOKENIZE = -5,
    HAL_SHELL_ERR_EVAL = -6
} hal_shell_err_t;

/*!
 @typedef hal_shell_stream_cb_t
 @param buffer Data buffer
 @param size Size of data in buffer
 @param userdata User-provided data pointer
 @brief Callback type for streaming stdout/stderr
 @discussion Receives buffer, size, and userdata. 
             Invoked as data arrives from child process.
*/
typedef void (*hal_shell_stream_cb_t)(const char *buffer, size_t size, void *userdata);

/*!
 @typedef hal_shell_io_t
 @field out Captured stdout buffer (NUL-terminated, allocated by hal_shell)
 @field out_len Length of captured stdout
 @field err Captured stderr buffer (NUL-terminated, allocated by hal_shell)
 @field err_len Length of captured stderr
 @field in Input buffer to write to child's stdin
 @field in_len Length of input buffer
 @field out_cb Callback for streaming stdout (if set, out buffer not populated)
 @field err_cb Callback for streaming stderr (if set, err buffer not populated)
 @field userdata User data passed to callbacks
 @discussion 
    hal_shell_io_t is used to control input/output capture and streaming for hal_shell.
    It allows capturing stdout and stderr into buffers or streaming them via callbacks.
    Input can be provided to the child process via a buffer.
    Callbacks receive chunks of data as they arrive, useful for large outputs.
    When callbacks are used, output buffers are not populated.
    The caller is responsible for freeing captured output buffers when done.
*/
typedef struct hal_shell_io {
    /* Captured output buffers (allocated by hal_shell when callbacks are NULL). */
    char *out;    /* captured stdout (NUL-terminated) */
    size_t out_len;
    char *err;    /* captured stderr (NUL-terminated) */
    size_t err_len;

    /* Input to be written to child's STDIN before closing it. Not modified by hal_shell. */
    const char *in;
    size_t in_len;

    /* Streaming callbacks. If non-NULL, hal_shell will call these with each
     * incoming chunk. Callbacks receive `userdata` as the last argument.
     * When callbacks are used, `out`/`err` buffers are not populated.
     */
    hal_shell_stream_cb_t out_cb;
    hal_shell_stream_cb_t err_cb;
    void *userdata;
} hal_shell_io_t;

/*!
 @function hal_shell
 @param command Command to execute
 @param io Capture structure, NULL to ignore (no capture output)
 @return Returns the exit code of the command, negative on internal error
 @brief Execute a shell command
 @discussion Execute a shell command (bourne shell syntax, |, >, <, &&, ;)
*/
int hal_shell(const char *command, hal_shell_io_t *io);

/*!
 @function hal_shell_fmt
 @param io Capture structure, NULL to ignore (no capture output)
 @param command Command to execute (formatted)
 @param ... printf style format arguments for command
 @return Returns the exit code of the command, negative on internal error
 @brief Execute a shell command with printf formatting
*/
int hal_shell_fmt(hal_shell_io_t *io, const char *command, ...);

#ifdef __cplusplus
}
#endif
#endif // HAL_SHELL_HEAD
