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
 @header filesystem.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Cross-platform file, directory, and path utilities
*/

#ifndef HAL_FILESYSTEM_HEAD
#define HAL_FILESYSTEM_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_FILESYSTEM
#include "hal.h"
#include <stddef.h>

#ifndef HAL_MAX_PATH
#if defined(__APPLE__)
#define HAL_MAX_PATH 255
#elif defined(_WIN32) || defined(_WIN64)
#define HAL_MAX_PATH 256
#elif defined(__linux__) || defined(__LINUX__)
#define HAL_MAX_PATH 4096
#else
#define HAL_MAX_PATH 1024
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define HAL_PATH_SEPARATOR '\\'
#define HAL_PATH_SEPARATOR_STR "\\"
#else
#define HAL_PATH_SEPARATOR '/'
#define HAL_PATH_SEPARATOR_STR "/"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define HAL_IO_IN STD_INPUT_HANDLE
#define HAL_IO_OUT STD_OUTPUT_HANDLE
#define HAL_IO_ERR STD_ERROR_HANDLE
#else
#define HAL_IO_IN STDIN_FILENO
#define HAL_IO_OUT STDOUT_FILENO
#define HAL_IO_ERR STDERR_FILENO
#endif

#if defined(_WIN32) || defined(_WIN64)
typedef void* hal_file_handle_t; /* HANDLE */
#define HAL_INVALID_FILE_HANDLE ((void*)-1)
#else
typedef int hal_file_handle_t;
#define HAL_INVALID_FILE_HANDLE -1
#endif

/*!
 @typedef hal_file_t
 @field fd Underlying file handler
 @discussion hal_file_t is a wrapper around OS specific file handlers
*/
typedef struct hal_file {
    hal_file_handle_t fd;
} hal_file_t;

/*!
 @const hal_io_invalid
 @discussion Invalid file handler (-1)
*/
extern const hal_file_t hal_io_invalid;
/*!
 @const hal_io_out
 @discussion Standard out
*/
extern const hal_file_t hal_io_out;
/*!
 @const hal_io_err
 @discussion Standard error
*/
extern const hal_file_t hal_io_err;
/*!
 @const hal_io_in
 @discussion Standard in
*/
extern const hal_file_t hal_io_in;

/*!
 @function hal_io_print
 @param IO File stream to write to
 @param MSG Message to print
 @brief Print a message to a file stream
*/
#define hal_io_print(IO, MSG)             \
    do                                    \
    {                                     \
        if (hal_io_valid(IO))             \
            hal_io_write_string(IO, MSG); \
    } while (0)
/*!
 @function hal_io_println
 @param IO File stream to write to
 @param MSG Message to print
 @brief Print a message to a file stream with a newline
*/
#define hal_io_println(IO, MSG)     \
    do                              \
    {                               \
        hal_io_print(IO, MSG);      \
        hal_io_print(IO, "\n");     \
    } while(0)
/*!
 @function hal_print
 @param MSG Message to print
 @brief Print a message to standard out
*/
#define hal_print(MSG)       hal_io_print(hal_io_out, MSG)
/*!
 @function hal_println
 @param MSG Message to print
 @brief Print a message to standard out with a newline
*/
#define hal_println(MSG)     hal_io_println(hal_io_out, MSG)
/*!
 @function hal_print_err
 @param MSG Message to print
 @brief Print a message to standard error
*/
#define hal_print_err(MSG)   hal_io_print(hal_io_err, MSG)
/*!
 @function hal_println_err
 @param MSG Message to print
 @brief Print a message to standard error with a newline
*/
#define hal_println_err(MSG) hal_io_println(hal_io_err, MSG)

/*!
 @enum hal_file_mode_t
 @constant HAL_FILE_READ File read flag (r)
 @constant HAL_FILE_WRITE File write flag (w)
 @constant HAL_FILE_APPEND File append flag (a)
 @discussion File permissions for hal_io_open
*/
typedef enum hal_file_mode {
    HAL_FILE_READ   = 0x00000001,
    HAL_FILE_WRITE  = 0x00000002,
    HAL_FILE_APPEND = 0x00000004
} hal_file_mode_t;

/*!
 @enum hal_file_seek_t
 @constant HAL_FILE_START Beginning of file marker (SEEK_SET)
 @constant HAL_FILE_CURSOR Current file position marker (SEEK_CUR)
 @constant HAL_FILE_FINISH End of file marker (SEEK_END)
 @discussion File offset positions for hal_io_seek
*/
typedef enum hal_file_seek {
    HAL_FILE_START  = 0,
    HAL_FILE_CURSOR,
    HAL_FILE_FINISH
} hal_file_seek_t;

/*!
 @function hal_io_open
 @param dst File steam destination
 @param path Path to file
 @param mode File steam access mode(s)
 @return Returns true/false on success
 @brief Open file stream
*/
bool hal_io_open(hal_file_t *dst, const char* path, hal_file_mode_t mode);
/*!
 @function hal_io_close
 @param file Pointer to file steam to close
 @return Returns true/false on success
 @brief Close file stream
*/
bool hal_io_close(hal_file_t *file);
/*!
 @function hal_io_read
 @param file File stream to read from
 @param buffer Buffer to read to
 @param size Size of the destination buffer
 @return Returns number of bytes read, 0 on error
 @brief Read n bytes from file stream
 @discussion Please ensure that size >= destination buffer size
*/
size_t hal_io_read(hal_file_t file, void *buffer, size_t size);
/*!
 @function hal_io_write
 @param file File stream to write to
 @param buffer Source buffer to write
 @param size Number of bytes to write
 @return Returns number of bytes written, 0 on error
 @brief Write n bytes to file stream
 @discussion Please ensure that size is >= the source buffer size
*/
size_t hal_io_write(hal_file_t file, const void *buffer, size_t size);
/*!
 @function hal_io_seek
 @param file File stream to modify
 @param offset The number of bytes to seek
 @param whence The offset of the seek
 @return Returns true/false on success
 @brief Seek file stream position
*/
bool hal_io_seek(hal_file_t file, long offset, hal_file_seek_t whence);
/*!
 @function hal_io_advance
 @param file File stream to modify
 @param offset Number of bytes to seek
 @return Returns true/false on success
 @brief Advance the file stream cursor by n bytes
*/
bool hal_io_advance(hal_file_t file, long offset);
/*!
 @function hal_io_tell
 @param file File stream to check
 @return Returns the current file stream position, -1 on error
 @brief Get current position of file stream
*/
size_t hal_io_tell(hal_file_t file);
/*!
 @function hal_io_eof
 @param file File stream to check
 @return Returns true if file is at the end
 @brief Check if file is at the end
*/
bool hal_io_eof(hal_file_t file);
/*!
 @function hal_io_flush
 @param file File stream to flush
 @return Returns true/false on success
 @brief Flush file stream
*/
bool hal_io_flush(hal_file_t file);
/*!
 @function hal_io_valid
 @param file File stream to check
 @return Returns true if file stream is valid
 @brief Check if a file stream is valid
*/
bool hal_io_valid(hal_file_t file);
/*!
 @function hal_io_read_line
 @param file File stream to read from
 @param buffer Buffer to read to
 @param size Size of the destination buffer
 @return Returns true if a line was read
 @brief Read until newline or end of file
*/
bool hal_io_read_line(hal_file_t file, char *buffer, size_t size);
/*!
 @function hal_io_write_string
 @param file File stream to write to
 @param str String to write
 @return Returns true/false on success
 @brief Write a string to a file stream
*/
bool hal_io_write_string(hal_file_t file, const char *str);
/*!
 @function hal_io_truncate
 @param file File stream to truncate
 @param size New size for the file
 @return Returns true/false on success
 @brief Truncate a file stream to a specific length
*/
bool hal_io_truncate(hal_file_t file, long size);

/*!
 @function hal_file_exists
 @param path Path to file
 @return Returns true if file exists
 @brief Check if a file exists
 @discussion Checks if a path exists and is a file (directory will return false)
*/
bool hal_file_exists(const char *path);
/*!
 @function hal_file_delete
 @param path Path to file
 @return Returns true/false on success
 @brief Delete a file from file system
*/
bool hal_file_delete(const char *path);
/*!
 @function hal_file_rename
 @param old_path Path to file to rename
 @param new_path New path for file
 @param write_over Write over existing files when moving
 @return Returns true/false on success
 @brief Move a file
*/
bool hal_file_rename(const char *old_path, const char *new_path, bool write_over);
/*!
 @function hal_file_copy
 @param src_path Path to original file
 @param dst_path Path to copy
 @param write_over Write over existing files when copying
 @return Returns true/false on success
 @brief Copy a file
*/
bool hal_file_copy(const char *src_path, const char *dst_path, bool write_over);
/*!
 @function hal_file_size
 @param path Path to file
 @return Returns size of a file, -1 on error
 @brief Get file size
*/
int hal_file_size(const char *path);
/*!
 @function hal_file_read
 @param path Path to file
 @param size Pointer to receive file size
 @return Returns the contents of a file
 @brief Read a file from disk
 @discussion Return value will need to be freed on success
*/
const char* hal_file_read(const char *path, size_t *size);

/*!
 @function hal_directory_exists
 @param path Path to directory
 @return Returns true if directory exists
 @brief Check if a directory exists
 @discussion Checks if a path exists and is a directory (file will return false)
*/
bool hal_directory_exists(const char *path);
/*!
 @function hal_directory_create
 @param path Path to directory
 @param recursive If true, will create any required parent directories
 @return Returns true/false on success
 @brief Create a directory
*/
bool hal_directory_create(const char *path, bool recursive);
/*!
 @function hal_directory_delete
 @param path Path to directory
 @param recursive If true, will delete child directories too
 @param and_files If true, will delete files inside the directory
 @return Returns true/false on success
 @brief Delete a directory
*/
bool hal_directory_delete(const char *path, bool recursive, bool and_files);
/*!
 @function hal_directory_rename
 @param old_path Path to directory
 @param new_path Path to directory to move to
 @param write_over If true, write over any existing files when copying
 @return Returns true/false on success
 @brief Move a directory
*/
bool hal_directory_rename(const char *old_path, const char *new_path, bool write_over);
/*!
 @function hal_directory_copy
 @param src_path Path to directory
 @param dst_path Path to copy directory to
 @param write_over If true, write over any existing files when copying
 @param delete_src If true, delete the original directory after copy succeeds
 @return Returns true/false on success
 @brief Copy a directory and contents
*/
bool hal_directory_copy(const char *src_path, const char *dst_path, bool write_over, bool delete_src);
/*!
 @function hal_directory_size
 @param path Path to directory
 @return Returns size of directory contents, -1 on error
 @brief Get the size of a directory
*/
int hal_directory_size(const char *path);
/*!
 @function hal_directory_item_count
 @param path Path to directory
 @param recursive If true, child directories will also be counted
 @return Returns number of items in directory, -1 on error
 @brief Get the number of files/directories inside a directory
*/
int hal_directory_item_count(const char *path, bool recursive);
/*!
 @function hal_directory_file_count
 @param path Path to directory
 @param recursive If true, child directories will also be counted
 @return Returns number of files in directory, -1 on error
 @brief Get the number of files inside a directory
*/
int hal_directory_file_count(const char *path, bool recursive);
/*!
 @function hal_directory_glob
 @param pattern Glob pattern to match (supports * and **)
 @param count Pointer to receive the number of matches
 @return Returns array of matching paths, NULL on error or no matches
 @brief Glob a directory for files matching a pattern
 @discussion Return value and array elements must be freed by caller
*/
const char** hal_directory_glob(const char *pattern, int *count);

/*!
 @typedef hal_dir_t
 @field path Path to the directory being iterated
 @discussion hal_dir_t is a wrapper around OS specific directory iterators
*/
typedef struct hal_dir {
    const char *path;
#if defined(_WIN32) || defined(_WIN64)
    char findData[592]; /* WIN32_FIND_DATA placeholder */
    void *hFind;
#else
    void *dir; /* DIR* */
#endif
} hal_dir_t;

/*!
 @function hal_directory_iter
 @param dir Directory iterator structure
 @param is_dir Pointer to receive whether the entry is a directory
 @return Returns the next entry name, NULL when done
 @brief Iterate through a directory
 @discussion Initialize hal_dir_t with .path = "path/to/dir" before first call
*/
const char* hal_directory_iter(hal_dir_t *dir, bool *is_dir);
/*!
 @function hal_directory_iter_end
 @param dir Directory iterator structure
 @brief End a directory iteration prematurely
 @discussion Call this if you break out of an iteration loop early
*/
void hal_directory_iter_end(hal_dir_t *dir);

/*!
 @typedef hal_glob_callback
 @param pattern The directory path
 @param filename The matched filename
 @param userdata User-provided data pointer
 @return Return 0 to continue, non-zero to stop iteration
 @discussion Callback type for glob matching
*/
typedef int(*hal_glob_callback)(const char* pattern, const char* filename, void* userdata);
/*!
 @typedef hal_walk_callback
 @param path The directory path
 @param filename The filename
 @param userdata User-provided data pointer
 @return Return 0 to continue, non-zero to stop iteration
 @discussion Callback type for directory walking
*/
typedef int(*hal_walk_callback)(const char* path, const char* filename, void* userdata);
/*!
 @function hal_path_glob
 @param pattern Glob pattern to match
 @param callback Callback function for each match
 @param userdata User data to pass to callback
 @return Returns true on success
 @brief Glob a pattern and call a callback for each match
*/
bool hal_path_glob(const char *pattern, hal_glob_callback callback, void *userdata);
/*!
 @function hal_path_walk
 @param path Directory path to walk
 @param recursive If true, recurse into subdirectories
 @param callback Callback function for each entry
 @param userdata User data to pass to callback
 @return Returns true on success
 @brief Walk a directory and call a callback for each file/directory
*/
bool hal_path_walk(const char *path, bool recursive, hal_walk_callback callback, void *userdata);

/*!
 @function hal_path_exists
 @param path Path to check
 @return Returns true if path exists (file or directory)
 @brief Check if a path exists
*/
bool hal_path_exists(const char *path);
/*!
 @function hal_path_cwd
 @return Returns the current working directory
 @brief Get the current working directory
 @discussion Returns a pointer to a static buffer, do not free
*/
const char* hal_path_cwd(void);
/*!
 @function hal_path_set_cwd
 @param path Path to set as working directory
 @return Returns true/false on success
 @brief Set the current working directory
*/
bool hal_path_set_cwd(const char *path);
/*!
 @function hal_path_root
 @return Returns the root directory ("/" on POSIX, "C:\" on Windows)
 @brief Get the root directory
*/
const char* hal_path_root(void);

/*!
 @function hal_path_extension
 @param path The path to get the file extension from
 @return Returns the file extension or NULL if there is no extension
 @brief Get the file extension from a path
 @discussion Returns pointer into the original string, do not free
*/
const char* hal_path_extension(const char *path);
/*!
 @function hal_path_filename
 @param path The path to get the file name from
 @return Returns the file name or NULL if there is no file name
 @brief Get the file name from a path
 @discussion Returns pointer into the original string, do not free
*/
const char* hal_path_filename(const char *path);
/*!
 @function hal_path_filename_no_ext
 @param path The path to get the file name without extension from
 @return Returns the file name without extension or NULL if there is no file name
 @brief Get the file name without extension from a path
 @discussion The result must be freed by the caller
*/
const char* hal_path_filename_no_ext(const char *path);
/*!
 @function hal_path_dirname
 @param path The path to get the directory from
 @return Returns the directory part or NULL if there is none
 @brief Get the directory name from a path
 @discussion The result must be freed by the caller
*/
const char* hal_path_dirname(const char *path);
/*!
 @function hal_path_parent
 @param path The path to get the parent directory from
 @return Returns the parent directory or root if there is no parent
 @brief Get the parent directory from a path
 @discussion The result must be freed by the caller
*/
const char* hal_path_parent(const char *path);

/*!
 @function hal_path_resolve
 @param path The path to resolve
 @return Returns the resolved absolute path or NULL on error
 @brief Resolve a path to an absolute path
 @discussion Resolves ~ to home directory and normalizes . and .. components.
             The result must be freed by the caller.
*/
const char* hal_path_resolve(const char *path);
/*!
 @function hal_path_join
 @param a The first path
 @param b The second path
 @return Returns the joined path or NULL on error
 @brief Join two paths together
 @discussion The result must be freed by the caller
*/
const char* hal_path_join(const char *a, const char *b);
/*!
 @function hal_path_join_va
 @param n The number of paths to join
 @param ... The paths to join
 @return Returns the joined path or NULL on error
 @brief Join multiple paths together
 @discussion The result must be freed by the caller
*/
const char* hal_path_join_va(int n, ...);
/*!
 @function hal_path_split
 @param path The path to split
 @param count Pointer to receive the number of parts
 @return Returns an array of path components, or NULL on error
 @brief Split a path into its components
 @discussion The result array and each element must be freed by the caller.
             The count will be set to the number of parts.
*/
const char** hal_path_split(const char *path, size_t *count);

#ifdef __cplusplus
}
#endif
#endif // HAL_FILESYSTEM_HEAD
