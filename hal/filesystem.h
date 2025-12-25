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

#ifndef HAL_FILESYSTEM_HEAD 
#define HAL_FILESYSTEM_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_FILESYSTEM
#include "hal.h"

#if __has_include(<sys/param.h>)
#include <sys/param.h>
#endif

#ifdef PLATFORM_WEB
#define PAUL_OS_NO_NATIVE
#endif

#if defined(__clang__) || defined(__GNUC__)
#if __has_extension(blocks)
#define PLATFORM_HAS_BLOCKS
#if __has_include(<Block.h>)
#include <Block.h>
#endif
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <wchar.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#ifdef PLATFORM_WINDOWS 
#include <windows.h>
#include <io.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <combaseapi.h>
#include <KnownFolders.h>
#else
#ifdef PLATFORM_MACOS
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

#ifndef MAX_PATH
#if defined(PLATFORM_MACOS)
#define MAX_PATH 255
#elif defined(PLATFORM_WINDOWS)
#define MAX_PATH 256
#elif defined(PLATFORM_LINUX)
#define MAX_PATH 4096
#else
#define MAX_PATH 1024
#endif
#endif

#ifdef PLATFORM_WINDOWS
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#endif

#ifdef PLATFORM_WINDOWS
#define IO_IN STD_INPUT_HANDLE
#define IO_OUT STD_OUTPUT_HANDLE
#define IO_ERR STD_ERROR_HANDLE
#else
#define IO_IN STDIN_FILENO
#define IO_OUT STDOUT_FILENO
#define IO_ERR STDERR_FILENO
#endif

#ifdef PLATFORM_WINDOWS
typedef HANDLE file_handle_t;
#elif defined(PLATFORM_POSIX)
typedef int file_handle_t;
#else
typedef FILE* file_handle_t;
#endif

/*!
 @typedef file_t
 @field fd Underlying file handler
 @discussion file_t is a wrapper around OS specific file handlers
*/
typedef struct file {
    file_handle_t fd;
} file_t;

#ifdef PLATFORM_WINDOWS
#define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
#else
#define INVALID_FILE_HANDLE -1
#endif

/*!
 @const io_invalid
 @discussion Invalid file handler (-1)
*/
extern const file_t io_invalid;
/*!
 @const io_out
 @discussion Standard out
*/
extern const file_t io_out;
/*!
 @const io_err
 @discussion Standard error
*/
extern const file_t io_err;
/*!
 @const io_in
 @discussion Standard in
*/
extern const file_t io_in;

/*!
 @function io_print
 @param IO File stream to write to
 @param MSG Message to print
 @brief Print a message to a file stream
*/
#define io_print(IO, MSG)             \
    do                                \
    {                                 \
        if (io_valid(IO))             \
            io_write_string(IO, MSG); \
    } while (0)
/*!
 @function io_println
 @param IO File stream to write to
 @param MSG Message to print
 @brief Print a message to a file stream with a newline
*/
#define io_println(IO, MSG)     \
    do                          \
    {                           \
        io_print(IO, MSG);      \
        io_print(IO, "\n")      \
    } while(0)
/*!
 @function print
 @param MSG Message to print
 @brief Print a message to standard out
*/
#define print(MSG)       io_print(io_out, MSG)
/*!
 @function println
 @param MSG Message to print
 @brief Print a message to standard out with a newline
*/
#define println(MSG)     io_println(io_out, MSG)
/*!
 @function print_err
 @param MSG Message to print
 @brief Print a message to standard error
*/
#define print_err(MSG)   io_print(io_err, MSG)
/*!
 @function println_err
 @param MSG Message to print
 @brief Print a message to standard error with a newline
*/
#define println_err(MSG) io_println(io_err, MSG)

/*!
 @enum file_mode_t
 @constant FILE_READ File read flag (r)
 @constant FILE_WRITE File write flag (w)
 @constant FILE_APPEND File append flag (a)
 @discussion File permissions for io_open
*/
enum file_mode_t {
    FILE_READ   = 0x00000001,
    FILE_WRITE  = 0x00000002,
    FILE_APPEND = 0x00000004
};

/*!
 @enum file_seek_t
 @constant FILE_START Beginning of file marker (SEEK_SET)
 @constant FILE_CURSOR Current file position marker (SEEK_CUR)
 @constant FILE_FINISH End of file marker (SEEK_END)
 @discussion File offset positions for io_seek
*/
enum file_seek_t {
    FILE_START  = 0,
    FILE_CURSOR,
    FILE_FINISH
};

/*!
 @function io_open
 @param dst File steam destination
 @param path Path to file
 @param mode File steam access mode(s)
 @brief Open file stream
*/
bool io_open(file_t *dst, const char* path, enum file_mode_t mode);
/*!
 @function io_close
 @param file Pointer to file steam to close
 @return Returns true/false on success
 @brief Close file stream
*/
bool io_close(file_t *file);
/*!
 @function io_read
 @param file File stream to read from
 @param buffer Buffer to read to
 @param size Size of the destination buffer
 @return Returns true/false on success
 @brief Read n bytes from file stream
 @discussion Please ensure that size >= destination buffer size
*/
size_t io_read(file_t file, void *buffer, size_t size);
/*!
 @function io_write
 @param file File stream to write to
 @param buffer Source buffer to write
 @param size Number of bytes to write
 @return Returns number of bytes read, -1 on error
 @brief Write n bytes to file stream
 @discussion Please ensure that size is >= the source buffer size
*/
size_t io_write(file_t file, const void *buffer, size_t size);
/*!
 @function io_seek
 @param file File stream to modify
 @param offset The number of bytes to seek
 @param whence The offset of the seek
 @return Returns true/false on success
 @brief Seek file stream position
*/
bool io_seek(file_t file, long offset, enum file_seek_t whence);
/*!
 @function io_advance
 @param file File stream to modify
 @param offset Number of bytes to seek
 @return Returns true/false on success
 @brief Advance the file stream cursor by n bytes
*/
bool io_advance(file_t file, long offset);
/*!
 @function io_tell
 @param file File stream to check
 @return Returns the current file stream position, -1 on error
 @brief Get current position of file stream
*/
size_t io_tell(file_t file);
/*!
 @function io_eof
 @param file File stream to check
 @return Returns true is file is at the end
 @brief Check if file is at the end
*/
bool io_eof(file_t file);
/*!
 @function io_flush
 @param file File stream to flush
 @return Returns true/false on success
 @brief Flush file stream
*/
bool io_flush(file_t file);
/*!
 @function io_valid
 @param file File stream to check
 @return Check if a file stream is valid
 @brief Check if a file stream is valid
*/
bool io_valid(file_t file);
/*!
 @function io_read_line
 @param file File stream to read
 @param buffer Destination buffer
 @param size Size of destination buffer
 @return Returns true/false on success
 @brief Read until newline or end of file
*/
bool io_read_line(file_t file, char *buffer, size_t size);
/*!
 @function io_write_string
 @param file File stream to write to
 @param str String to write
 @return Returns true/false on success
 @brief Write a string to a file stream
*/
bool io_write_string(file_t file, const char *str);
/*!
 @function io_truncate
 @param file File stream to modify
 @param size Offset bytes
 @return Returns true/false on success
 @brief Truncate a file stream to a specific length
*/
bool io_truncate(file_t file, long size);

/*!
 @function environment_variable
 @param name Variable name
 @return Returns enviroment variable string or NULL on failure
 @brief Get an enviroment variable
 @discussion Return value will need to be freed on success
*/
char* environment_variable(const char *name); // !

/*!
 @enum shell_error_t
 @constant SHELL_OK No error
 @constant SHELL_ERR_GENERIC Generic error
 @constant SHELL_ERR_TOKENIZE Error tokenizing command
 @constant SHELL_ERR_EVAL Error evaluating command
 @constant SHELL_ERR_PIPE Error creating pipe
 @constant SHELL_ERR_FORK Error forking process
 @constant SHELL_ERR_READ Error reading from pipe
 @discussion Error codes for shell() function
*/
typedef enum shell_error {
    SHELL_OK = 0,
    SHELL_ERR_GENERIC = -1,
    SHELL_ERR_TOKENIZE = -2,
    SHELL_ERR_EVAL = -3,
    SHELL_ERR_PIPE = -4,
    SHELL_ERR_FORK = -5,
    SHELL_ERR_READ = -6
} shell_error_t;

/*!
 @typedef shell_stream_cb_t
 @brief Callback type for streaming output
*/
typedef void (*shell_stream_cb_t)(const char *data, size_t len, void *userdata);

/*!
 @struct shell_io
 @field out Captured stdout buffer (NUL-terminated)
 @field out_len Length of captured stdout buffer
 @field err Captured stderr buffer (NUL-terminated)
 @field err_len Length of captured stderr buffer
 @field in Input buffer to write to child's stdin
 @field in_len Length of input buffer
 @field out_cb Callback for streaming stdout chunks
 @field err_cb Callback for streaming stderr chunks
 @field userdata User data pointer passed to callbacks
 @brief I/O capture and streaming control structure
 @discussion 
    shell_io is used to control input/output capture and streaming for the shell function.
    It allows capturing stdout and stderr into buffers or streaming them via callbacks.
    Input can be provided to the child process via a buffer.
    Callbacks receive chunks of data as they arrive, useful for large outputs.
    When callbacks are used, output buffers are not populated.
    The caller is responsible for freeing captured output buffers when done.
*/
typedef struct shell_io {
    /* I/O capture and streaming control structure
     *
     * - Pass a non-NULL `shell_io *` to `shell()` to enable capture/streaming.
     * - If `out_cb` and/or `err_cb` are set, `shell()` invokes the callback
     *   as chunks of data arrive on the child's STDOUT/STDERR. When callbacks
     *   are provided, `shell()` does NOT fill the `out`/`err` buffers (they
     *   will remain NULL). Callbacks are useful for streaming large outputs
     *   without buffering everything in memory.
     * - If callbacks are NULL, `shell()` accumulates the full contents of
     *   STDOUT and STDERR into freshly allocated, NUL-terminated buffers
     *   stored in `out` and `err` respectively. The sizes are in `out_len`
     *   and `err_len`. The caller is responsible for calling `free()` on
     *   `out` and `err` when done.
     * - To provide input to the child, set `in` to a buffer of length
     *   `in_len`. `shell()` will write the contents of `in` to the child's
     *   STDIN and then close it. Ownership of `in` stays with the caller.
     * - Return values: non-negative return values are the child's exit code.
     *   Negative return values are library-level errors (see SHELL_ERR_*).
     */
    /* Captured output buffers (allocated by shell() when callbacks are NULL). */
    char *out;    /* captured stdout (NUL-terminated) */
    size_t out_len;
    char *err;    /* captured stderr (NUL-terminated) */
    size_t err_len;

    /* Input to be written to child's STDIN before closing it. Not modified by shell(). */
    const char *in;
    size_t in_len;

    /* Streaming callbacks. If non-NULL, shell() will call these with each
     * incoming chunk. Callbacks receive `userdata` as the last argument.
     * When callbacks are used, `out`/`err` buffers are not populated.
     */
    shell_stream_cb_t out_cb;
    shell_stream_cb_t err_cb;
    void *userdata;
} shell_io;

/*!
 @function shell
 @param command Command to execute
 @param io Capture structure, NULL to ignore (no capture output)
 @return Returns the exit code of the command, negative on internal error
 @brief Execute a shell command
 @discussion Execute a shell command (bourne shell syntax, |, >, <, &&, ;)
*/
int shell(const char *command, shell_io *io);

/*!
 @function shell_fmt
 @param io Capture structure, NULL to ignore (no capture output)
 @param command Command to execute (formatted)
 @param ... printf style format arguments for command
 @return Returns the exit code of the command, negative on internal error
 @discussion shell with printf formatting
*/
int shell_fmt(shell_io *io, const char *command, ...);

/*!
 @function file_exists
 @param path Path to file
 @return Returns true if file exists
 @brief Check if a file exists
 @discussion Checks if a path exists and is a file (directory will return false)
*/
bool file_exists(const char *path);
/*!
 @function file_delete
 @param path Path to file
 @return Returns true/false on success
 @brief Delete a file from file system
*/
bool file_delete(const char *path);
/*!
 @function file_rename
 @param old_path Path to file to rename
 @param new_path New path for file
 @param write_over Write over existing files when moving
 @return Returns true/false on success
 @brief Move a file
 @discussion WIP
*/
bool file_rename(const char *old_path, const char *new_path, bool write_over);
/*!
 @function file_copy
 @param src_path Path to original file
 @param dst_path Path to copy
 @param write_over Write over existing files when moving
 @return Returns true/false on success
 @brief Copy a file
 @discussion WIP
*/
bool file_copy(const char *src_path, const char *dst_path, bool write_over);
/*!
 @function file_size
 @param path Path to file
 @return Returns size of a file, -1 on error
 @brief Get file size
*/
int file_size(const char *path);
/*!
 @function file_read
 @param path Path to file
 @param size Pointer to recieve file size
 @return Returns the contents of a file
 @brief Read a file from disk
 @discussion Return value will need to be freed on success
*/
const char* file_read(const char *path, size_t *size);  // !

/*!
 @function directory_exists
 @param path Path to directory
 @return Returns true if file exists
 @brief Check if a directory exists
 @discussion Checks if a path exists and is a directory (file will return false)
*/
bool directory_exists(const char *path);
/*!
 @function directory_create
 @param path Path to directory
 @param recursive If this is true, will create any required parent directories
 @return Returns true/false on success
 @brief Create a directory
 @discussion WIP
*/
bool directory_create(const char *path, bool recursive);
/*!
 @function directory_delete
 @param path Path to directory
 @param recursive If this is true, it will delete and child directories too
 @param and_files If this is ture, it will delete and files inside the directory
 @return Returns true/false on success
 @brief Delete a directory
 @discussion WIP
*/
bool directory_delete(const char *path, bool recursive, bool and_files);
/*!
 @function directory_rename
 @param old_path Path to directory
 @param new_path Path to directory to move to
 @param write_over If this is true, write over any existing files when copying
 @return Returns true/false on success
 @brief Move a directory
 @discussion WIP
*/
bool directory_rename(const char *old_path, const char *new_path, bool write_over);
/*!
 @function directory_copy
 @param src_path Path to directory
 @param dst_path Path copy directory to
 @param write_over If this is true, write over any existing files when copying
 @param delete_src If this is true, delete the original directory and files after copy succeeds
 @return Returns true/false on success
 @brief Copy a directory and contents
*/
bool directory_copy(const char *src_path, const char *dst_path, bool write_over, bool delete_src);
/*!
 @function directory_size
 @param path Path to directory
 @return Returns size of directory contents, -1 on error
 @brief Get the size of a directory
*/
int directory_size(const char *path);
/*!
 @function directory_item_count
 @param path Path to directory
 @param recursive If this is true, child directories will also be counted
 @return Returns number of items in directory, -1 on error
 @brief Get the number of files/directories inside a directory
*/
int directory_item_count(const char *path, bool recursive);
/*!
 @function directory_file_count
 @param path Path to directory
 @param recursive If this is true, child directories will also be counted
 @return Returns number of files in directory, -1 on error
 @brief Get the number of files inside a directory
 @discussion This doesn't count directories
*/
int directory_file_count(const char *path, bool recursive);
/*!
 @function directory_glob
 @param pattern Glob pattern to match
 @param count Pointer to an integer to store the number of matches
 @return Returns an array of strings containing the matched file paths, or NULL on error
 @brief Glob a directory for files matching a pattern
 @discussion The result must be freed by the caller.
             The count will be set to the number of matches.
             The array and its elements must be freed by the caller.
*/
const char** directory_glob(const char *pattern, int *count); // !!

/*!
 @typedef dir_t
 @field path Path to directory
 @discussion dir_t is a wrapper around OS specific directory iterators
*/
typedef struct dir {
    const char *path;
#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATA findData;
    HANDLE hFind;
#elif defined(PLATFORM_POSIX)
    DIR *dir;
#endif
} dir_t;

/*!
 @function directory_iter
 @param dir Pointer to dir_t structure
 @param is_dir Pointer to bool to store if the current item is a directory
 @return Returns the name of the next item in the directory, or NULL if there are no more items
 @brief Iterate through a directory
 @discussion If you need to exit an iteration early, call directory_iter_end(dir)
*/
const char* directory_iter(dir_t *dir, bool *is_dir);
/*!
 @function directory_iter_end
 @param dir Pointer to dir_t structure
 @brief End a directory iteration prematurely
 @discussion This will close the directory iterator and free any resources used.
             Normally this will be done at the end of the iteration.
*/
void directory_iter_end(dir_t *dir);

/*!
 @typedef glob_callback
 @param pattern The glob pattern
 @param filename The filename that matches the pattern
 @param userdata User data pointer
 @return Returns 0 to continue iteration, non-zero to stop
*/
typedef int(*glob_callback)(const char* pattern, const char* filename, void* userdata);
/*!
 @typedef walk_callback
 @param path The current path being walked
 @param filename The filename in the current path
 @param userdata User data pointer
 @return Returns 0 to continue walking, non-zero to stop
*/
typedef int(*walk_callback)(const char* path, const char* filename, void* userdata);
/*!
 @function path_glob
 @param pattern The glob pattern to match
 @param callback Callback function to call for each match
 @param userdata User data pointer to pass to the callback
 @return Returns true/false on success
 @brief Glob a pattern and call a callback for each match
*/
bool path_glob(const char *pattern, glob_callback callback, void *userdata);
/*!
 @function path_walk
 @param path The path to walk
 @param recursive If this is true, it will walk recursively
 @param callback Callback function to call for each file/directory
 @param userdata User data pointer to pass to the callback
 @return Returns true/false on success
 @brief Walk a directory and call a callback for each file/directory
*/
bool path_walk(const char *path, bool recursive, walk_callback callback, void *userdata);

#ifdef PLATFORM_HAS_BLOCKS
/*!
 @typedef glob_block
 @param pattern The glob pattern
 @param filename The filename that matches the pattern
 @param userdata User data pointer
 @return Returns 0 to continue iteration, non-zero to stop
 @discussion This is a block version of glob_callback. Blocks are a clang/gcc extension. -fblocks must be enabled to use this.
*/
typedef int(^glob_block)(const char* pattern, const char* filename, void* userdata);
/*!
 @typedef walk_block
 @param path The current path being walked
 @param filename The filename in the current path
 @param userdata User data pointer
 @return Returns 0 to continue walking, non-zero to stop
 @discussion This is a block version of walk_callback. Blocks are a clang/gcc extension. -fblocks must be enabled to use this.
*/
typedef int(^walk_block)(const char* path, const char* filename, void* userdata);
/*!
 @function path_glob_block
 @param pattern The glob pattern to match
 @param callback Block to call for each match
 @param userdata User data pointer to pass to the block
 @return Returns true/false on success
 @brief Glob a pattern and call a block for each match
 @discussion Blocks are a clang/gcc extension. -fblocks must be enabled to use this.
*/
bool path_glob_block(const char *pattern, glob_block callback, void *userdata);
/*!
 @function path_walk_block
 @param path The path to walk
 @param recursive If this is true, it will walk recursively
 @param callback Block to call for each file/directory
 @param userdata User data pointer to pass to the block
 @return Returns true/false on success
 @brief Walk a directory and call a block for each file/directory
 @discussion Blocks are a clang/gcc extension. -fblocks must be enabled to use this.
*/
bool path_walk_block(const char *path, bool recursive, walk_block callback, void *userdata);
#endif

/*!
 @function path_exists
 @param path The path to check
 @return Returns true/false on success
 @brief Check if a path exists. This will return true for both files and directories.
*/
bool path_exists(const char *path);
/*!
 @function path_get_working_directory
 @return Returns the current working directory
 @brief Get the current working directory
*/
const char* path_get_working_directory(void);
/*!
 @function path_set_working_directory
 @param path The path to set as the working directory
 @return Returns true/false on success
 @brief Set the current working directory
*/
bool path_set_working_directory(const char *path);

/*!
 @function path_get_root_dir
 @return Returns the root directory
 @brief Get the root directory
*/
const char* path_get_root_dir(void);
/*!
 @function path_get_home_dir
 @return Returns the home directory
 @brief Get the home directory
*/
const char* path_get_home_dir(void);
/*!
 @function path_get_documents_dir
 @return Returns the documents directory
 @brief Get the documents directory
*/
const char* path_get_documents_dir(void);
/*!
 @function path_get_downloads_dir
 @return Returns the downloads directory
 @brief Get the downloads directory
*/
const char* path_get_downloads_dir(void);
/*!
 @function path_get_video_dir
 @return Returns the video directory
 @brief Get the video directory
*/
const char* path_get_video_dir(void);
/*!
 @function path_get_music_dir
 @return Returns the music directory
 @brief Get the music directory
*/
const char* path_get_music_dir(void);
/*!
 @function path_get_picture_dir
 @return Returns the picture directory
 @brief Get the picture directory
*/
const char* path_get_picture_dir(void);
/*!
 @function path_get_application_dir
 @return Returns the application directory
 @brief Get the application directory
*/
const char* path_get_application_dir(void);
/*!
 @function path_get_desktop_dir
 @return Returns the desktop directory
 @brief Get the desktop directory
*/
const char* path_get_desktop_dir(void);

/*!
 @function path_get_file_extension
 @param path The path to get the file extension from
 @return Returns the file extension or NULL if there is no extension
 @brief Get the file extension from a path
*/
const char* path_get_file_extension(const char *path);
/*!
 @function path_get_file_name
 @param path The path to get the file name from
 @return Returns the file name or NULL if there is no file name
 @brief Get the file name from a path
*/
const char* path_get_file_name(const char *path);
/*!
 @function path_get_file_name_no_extension
 @param path The path to get the file name without extension from
 @return Returns the file name without extension or NULL if there is no file name
 @brief Get the file name without extension from a path
 @discussion The result must be freed by the caller.
*/
const char* path_get_file_name_no_extension(const char *path);  // !
/*!
 @function path_without_file_name
 @param path The path to get the directory name from
 @return Returns the directory name or NULL if there is no directory name
 @brief Get the directory name from a path
 @discussion The result must be freed by the caller.
*/
const char* path_without_file_name(const char *path);  // !
/*!
 @function path_get_directory_name
 @param path The path to get the directory name from
 @return Returns the directory name or NULL if there is no directory name
 @brief Get the directory name from a path
 @discussion The result must be freed by the caller.
*/
const char* path_get_directory_name(const char *path); // !
/*!
 @function path_get_parent_directory
 @param path The path to get the parent directory from
 @return Returns the parent directory or NULL if there is no parent directory
 @brief Get the parent directory from a path
 @discussion The result must be freed by the caller.
*/
const char* path_get_parent_directory(const char *path); // !

/*!
 @function path_resolve
 @param path The path to resolve
 @return Returns the resolved path or NULL on error
 @brief Resolve a path to an absolute path
 @discussion This will attempt to resolve ~ to the user's home directory and ".." to the parent directory. The result must be freed by the caller. WIP.
*/
const char* path_resolve(const char *path); // !
/*!
 @function path_join
 @param a The first path
 @param b The second path
 @return Returns the joined path or NULL on error
 @brief Join two paths together
 @discussion The result must be freed by the caller.
*/
const char* path_join(const char *a, const char *b); // !
/*!
 @function path_join_va
 @param n The number of paths to join
 @param ... The paths to join
 @return Returns the joined path or NULL on error
 @brief Join multiple paths together
 @discussion The result must be freed by the caller.
*/
const char* path_join_va(int n, ...); // !
/*!
 @function path_split
 @param path The path to split
 @param count Pointer to an integer to store the number of parts
 @return Returns an array of strings containing the parts of the path, or NULL on error
 @brief Split a path into its components
 @discussion The result must be freed by the caller.
             The count will be set to the number of parts.
             The array and its elements must be freed by the caller.
*/
const char** path_split(const char *path, size_t *count); // !!

#ifdef __cplusplus
}
#endif
#endif // HAL_FILESYSTEM_HEAD