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

#ifndef HAL_NO_FILESYSTEM
#include "hal/filesystem.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Constants */
const hal_file_t hal_io_invalid = {.fd = HAL_INVALID_FILE_HANDLE};
const hal_file_t hal_io_out = {.fd = (hal_file_handle_t)STD_OUTPUT_HANDLE};
const hal_file_t hal_io_err = {.fd = (hal_file_handle_t)STD_ERROR_HANDLE};
const hal_file_t hal_io_in  = {.fd = (hal_file_handle_t)STD_INPUT_HANDLE};

/* hal_io_* functions */
bool hal_io_open(hal_file_t *dst, const char* path, hal_file_mode_t mode) {
    if (!dst || !path)
        return false;
    
    DWORD access = 0;
    DWORD creation = OPEN_EXISTING;
    
    if (mode & HAL_FILE_READ)
        access |= GENERIC_READ;
    if (mode & HAL_FILE_WRITE)
        access |= GENERIC_WRITE;
    if (mode & HAL_FILE_APPEND)
        access |= FILE_APPEND_DATA;
    
    if (mode & (HAL_FILE_WRITE | HAL_FILE_APPEND)) {
        if (mode & HAL_FILE_WRITE && !(mode & HAL_FILE_APPEND))
            creation = CREATE_ALWAYS;
        else
            creation = OPEN_ALWAYS;
    }
    
    HANDLE h = CreateFileA(path, access, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        dst->fd = HAL_INVALID_FILE_HANDLE;
        return false;
    }
    
    if (mode & HAL_FILE_APPEND)
        SetFilePointer(h, 0, NULL, FILE_END);
    
    dst->fd = (hal_file_handle_t)h;
    return true;
}

bool hal_io_close(hal_file_t *file) {
    if (!file || !hal_io_valid(*file))
        return false;
    BOOL result = CloseHandle((HANDLE)file->fd);
    file->fd = HAL_INVALID_FILE_HANDLE;
    return result != 0;
}

size_t hal_io_read(hal_file_t file, void *buffer, size_t size) {
    if (!hal_io_valid(file) || !buffer || size <= 0)
        return 0;
    DWORD bytesRead;
    if (!ReadFile((HANDLE)file.fd, buffer, (DWORD)size, &bytesRead, NULL))
        return 0;
    return (size_t)bytesRead;
}

size_t hal_io_write(hal_file_t file, const void *buffer, size_t size) {
    if (!hal_io_valid(file) || !buffer || size <= 0)
        return 0;
    DWORD bytesWritten;
    if (!WriteFile((HANDLE)file.fd, buffer, (DWORD)size, &bytesWritten, NULL))
        return 0;
    return (size_t)bytesWritten;
}

bool hal_io_seek(hal_file_t file, long offset, hal_file_seek_t whence) {
    if (!hal_io_valid(file))
        return false;
    DWORD method = (whence == HAL_FILE_START) ? FILE_BEGIN :
                   (whence == HAL_FILE_CURSOR) ? FILE_CURRENT : FILE_END;
    return SetFilePointer((HANDLE)file.fd, offset, NULL, method) != INVALID_SET_FILE_POINTER;
}

bool hal_io_advance(hal_file_t file, long offset) {
    return hal_io_seek(file, offset, HAL_FILE_CURSOR);
}

size_t hal_io_tell(hal_file_t file) {
    if (!hal_io_valid(file))
        return (size_t)-1;
    DWORD pos = SetFilePointer((HANDLE)file.fd, 0, NULL, FILE_CURRENT);
    if (pos == INVALID_SET_FILE_POINTER)
        return (size_t)-1;
    return (size_t)pos;
}

bool hal_io_eof(hal_file_t file) {
    if (!hal_io_valid(file))
        return true;
    DWORD pos = SetFilePointer((HANDLE)file.fd, 0, NULL, FILE_CURRENT);
    DWORD size = GetFileSize((HANDLE)file.fd, NULL);
    return pos >= size;
}

bool hal_io_flush(hal_file_t file) {
    if (!hal_io_valid(file))
        return false;
    return FlushFileBuffers((HANDLE)file.fd) != 0;
}

bool hal_io_valid(hal_file_t file) {
    return file.fd != HAL_INVALID_FILE_HANDLE && file.fd != NULL;
}

bool hal_io_read_line(hal_file_t file, char *buffer, size_t buffer_size) {
    if (!hal_io_valid(file) || !buffer || buffer_size <= 0)
        return false;
    size_t pos = 0;
    char ch;
    while (pos < buffer_size - 1) {
        if (hal_io_read(file, &ch, 1) != 1)
            break;
        if (ch == '\n')
            break;
        if (ch != '\r')
            buffer[pos++] = ch;
    }
    buffer[pos] = '\0';
    return pos > 0 || !hal_io_eof(file);
}

bool hal_io_write_string(hal_file_t file, const char *str) {
    if (!hal_io_valid(file) || !str)
        return false;
    size_t len = strlen(str);
    if (!len)
        return true;
    return hal_io_write(file, str, len) == len;
}

bool hal_io_truncate(hal_file_t file, long size) {
    if (!hal_io_valid(file) || size < 0)
        return false;
    DWORD pos = SetFilePointer((HANDLE)file.fd, size, NULL, FILE_BEGIN);
    if (pos == INVALID_SET_FILE_POINTER)
        return false;
    return SetEndOfFile((HANDLE)file.fd) != 0;
}

/* hal_file_* functions */
bool hal_file_exists(const char *path) {
    if (!path)
        return false;
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool hal_file_delete(const char *path) {
    if (!path || !hal_file_exists(path))
        return false;
    return DeleteFileA(path) != 0;
}

bool hal_file_rename(const char *old_path, const char *new_path, bool write_over) {
    if (!old_path || !hal_file_exists(old_path) || !new_path)
        return false;
    if (!write_over && hal_file_exists(new_path))
        return false;
    if (write_over && hal_file_exists(new_path))
        DeleteFileA(new_path);
    return MoveFileA(old_path, new_path) != 0;
}

bool hal_file_copy(const char *src_path, const char *dst_path, bool write_over) {
    if (!src_path || !dst_path || !hal_file_exists(src_path))
        return false;
    if (!write_over && hal_path_exists(dst_path))
        return false;
    return CopyFileA(src_path, dst_path, !write_over) != 0;
}

int hal_file_size(const char *path) {
    if (!path)
        return -1;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad))
        return -1;
    return (int)fad.nFileSizeLow;
}

const char* hal_file_read(const char *path, size_t *size) {
    char *result = NULL;
    int _size = hal_file_size(path);
    hal_file_t file = hal_io_invalid;
    if (_size < 0 || !hal_io_open(&file, path, HAL_FILE_READ))
        goto BAIL;
    if (!(result = (char*)malloc(_size + 1)))
        goto BAIL;
    if (hal_io_read(file, result, _size) != (size_t)_size) {
        free(result);
        result = NULL;
        goto BAIL;
    }
    result[_size] = '\0';
BAIL:
    if (hal_io_valid(file))
        hal_io_close(&file);
    if (size)
        *size = result ? _size : 0;
    return result;
}

/* hal_directory_* functions */
bool hal_directory_exists(const char *path) {
    if (!path)
        return false;
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool hal_directory_create(const char *path, bool recursive) {
    if (!path)
        return false;
    if (hal_directory_exists(path))
        return true;
    if (!recursive)
        return CreateDirectoryA(path, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
    
    char tmp[HAL_MAX_PATH];
    size_t len = strlen(path);
    if (len >= HAL_MAX_PATH)
        return false;
    strcpy(tmp, path);
    for (size_t i = 1; i < len; i++) {
        if (tmp[i] == '\\' || tmp[i] == '/') {
            char c = tmp[i];
            tmp[i] = '\0';
            CreateDirectoryA(tmp, NULL);
            tmp[i] = c;
        }
    }
    return CreateDirectoryA(tmp, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

static bool rmdir_recursive_win(const char *path) {
    char pattern[HAL_MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    
    bool success = true;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        char fullpath[HAL_MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", path, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!rmdir_recursive_win(fullpath))
                success = false;
        } else {
            if (!DeleteFileA(fullpath))
                success = false;
        }
    } while (FindNextFileA(hFind, &fd));
    
    FindClose(hFind);
    if (success)
        success = RemoveDirectoryA(path) != 0;
    return success;
}

bool hal_directory_delete(const char *path, bool recursive, bool and_files) {
    if (!path || !hal_directory_exists(path))
        return false;
    if (recursive)
        return rmdir_recursive_win(path);
    if (and_files) {
        hal_dir_t d = {.path = path};
        const char *name = NULL;
        bool is_dir = false;
        while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
            if (is_dir) {
                hal_directory_iter_end(&d);
                return false;
            }
            const char *full_path = hal_path_join(path, name);
            if (!full_path || !hal_file_delete(full_path)) {
                if (full_path) free((void*)full_path);
                hal_directory_iter_end(&d);
                return false;
            }
            free((void*)full_path);
        }
    }
    return RemoveDirectoryA(path) != 0;
}

bool hal_directory_rename(const char *old_path, const char *new_path, bool write_over) {
    return hal_directory_copy(old_path, new_path, write_over, true);
}

bool hal_directory_copy(const char *src_path, const char *dst_path, bool write_over, bool delete_src) {
    if (!src_path || !hal_directory_exists(src_path) || !dst_path)
        return false;
    
    if (!hal_directory_exists(dst_path) && !hal_directory_create(dst_path, true))
        return false;
    
    hal_dir_t d = {.path = src_path};
    const char *name = NULL;
    bool is_dir = false;
    bool result = true;
    
    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        const char *src_full = hal_path_join(src_path, name);
        const char *dst_full = hal_path_join(dst_path, name);
        if (!src_full || !dst_full) {
            if (src_full) free((void*)src_full);
            if (dst_full) free((void*)dst_full);
            hal_directory_iter_end(&d);
            return false;
        }
        if (is_dir) {
            if (!hal_directory_copy(src_full, dst_full, write_over, false))
                result = false;
        } else {
            if (!hal_file_copy(src_full, dst_full, write_over))
                result = false;
        }
        free((void*)src_full);
        free((void*)dst_full);
        if (!result) {
            hal_directory_iter_end(&d);
            return false;
        }
    }
    
    if (delete_src && result)
        hal_directory_delete(src_path, true, true);
    return result;
}

int hal_directory_size(const char *path) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path = path};
    const char *name = NULL;
    bool is_dir = false;
    int total = 0;
    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        const char *full_path = hal_path_join(path, name);
        if (!full_path) {
            hal_directory_iter_end(&d);
            return -1;
        }
        total += is_dir ? hal_directory_size(full_path) : hal_file_size(full_path);
        free((void*)full_path);
    }
    return total;
}

int hal_directory_item_count(const char *path, bool recursive) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path = path};
    const char *name = NULL;
    bool is_dir = false;
    int total = 0;
    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        total++;
        if (is_dir && recursive) {
            const char *full_path = hal_path_join(path, name);
            if (!full_path) {
                hal_directory_iter_end(&d);
                return -1;
            }
            total += hal_directory_item_count(full_path, true);
            free((void*)full_path);
        }
    }
    return total;
}

int hal_directory_file_count(const char *path, bool recursive) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path = path};
    const char *name = NULL;
    bool is_dir = false;
    int total = 0;
    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        if (is_dir) {
            if (recursive) {
                const char *full_path = hal_path_join(path, name);
                if (!full_path) {
                    hal_directory_iter_end(&d);
                    return -1;
                }
                total += hal_directory_file_count(full_path, true);
                free((void*)full_path);
            }
        } else {
            total++;
        }
    }
    return total;
}

/* Directory iteration */
const char* hal_directory_iter(hal_dir_t *dir, bool *is_dir) {
    if (!dir || !dir->path)
        return NULL;
    
    WIN32_FIND_DATAA *fd = (WIN32_FIND_DATAA*)dir->findData;
    
    if (!dir->hFind) {
        char pattern[HAL_MAX_PATH];
        snprintf(pattern, sizeof(pattern), "%s\\*", dir->path);
        dir->hFind = FindFirstFileA(pattern, fd);
        if (dir->hFind == INVALID_HANDLE_VALUE) {
            dir->hFind = NULL;
            return NULL;
        }
    } else {
        if (!FindNextFileA((HANDLE)dir->hFind, fd)) {
            FindClose((HANDLE)dir->hFind);
            dir->hFind = NULL;
            return NULL;
        }
    }
    
    while (strcmp(fd->cFileName, ".") == 0 || strcmp(fd->cFileName, "..") == 0) {
        if (!FindNextFileA((HANDLE)dir->hFind, fd)) {
            FindClose((HANDLE)dir->hFind);
            dir->hFind = NULL;
            return NULL;
        }
    }
    
    if (is_dir)
        *is_dir = (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return fd->cFileName;
}

void hal_directory_iter_end(hal_dir_t *dir) {
    if (dir && dir->hFind) {
        FindClose((HANDLE)dir->hFind);
        dir->hFind = NULL;
    }
}

/* hal_path_* functions */
bool hal_path_exists(const char *path) {
    if (!path)
        return false;
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES;
}

const char* hal_path_cwd(void) {
    static char cwd[HAL_MAX_PATH];
    if (GetCurrentDirectoryA(sizeof(cwd), cwd))
        return cwd;
    return NULL;
}

bool hal_path_set_cwd(const char *path) {
    if (!path)
        return false;
    return SetCurrentDirectoryA(path) != 0;
}

const char* hal_path_root(void) {
    static char root[4] = "C:\\";
    char cwd[HAL_MAX_PATH];
    if (GetCurrentDirectoryA(sizeof(cwd), cwd) && cwd[0] && cwd[1] == ':') {
        root[0] = cwd[0];
    }
    return root;
}

const char* hal_path_extension(const char *path) {
    if (!path)
        return NULL;
    const char *dot = strrchr(path, '.');
    const char *slash = strrchr(path, '\\');
    const char *fslash = strrchr(path, '/');
    if (fslash && (!slash || fslash > slash))
        slash = fslash;
    if (dot && (!slash || dot > slash))
        return dot + 1;
    return NULL;
}

const char* hal_path_filename(const char *path) {
    if (!path)
        return NULL;
    const char *slash = strrchr(path, '\\');
    const char *fslash = strrchr(path, '/');
    if (fslash && (!slash || fslash > slash))
        slash = fslash;
    return slash ? slash + 1 : path;
}

const char* hal_path_filename_no_ext(const char *path) {
    if (!path)
        return NULL;
    const char *file_name = hal_path_filename(path);
    if (!file_name)
        return NULL;
    size_t length = strlen(file_name);
    const char *dot = strrchr(file_name, '.');
    if (dot) {
        length = dot - file_name;
        if (length == 0)
            return NULL;
    }
    char *result = (char*)malloc(length + 1);
    if (!result)
        return NULL;
    strncpy(result, file_name, length);
    result[length] = '\0';
    return result;
}

const char* hal_path_dirname(const char *path) {
    if (!path)
        return NULL;
    const char *slash = strrchr(path, '\\');
    const char *fslash = strrchr(path, '/');
    if (fslash && (!slash || fslash > slash))
        slash = fslash;
    if (!slash)
        return NULL;
    size_t length = slash - path;
    char *result = (char*)malloc(length + 1);
    if (!result)
        return NULL;
    strncpy(result, path, length);
    result[length] = '\0';
    return result;
}

const char* hal_path_parent(const char *path) {
    if (!path)
        return NULL;
    const char *slash = strrchr(path, '\\');
    const char *fslash = strrchr(path, '/');
    if (fslash && (!slash || fslash > slash))
        slash = fslash;
    if (!slash || slash == path)
        return strdup(hal_path_root());
    
    /* Check for drive letter case like C:\ */
    if (slash == path + 2 && path[1] == ':')
        return strdup(hal_path_root());
    
    size_t length = slash - path;
    char *result = (char*)malloc(length + 1);
    if (!result)
        return NULL;
    strncpy(result, path, length);
    result[length] = '\0';
    return result;
}

const char* hal_path_resolve(const char *path) {
    if (!path)
        return NULL;
    char resolved[HAL_MAX_PATH];
    DWORD len = GetFullPathNameA(path, sizeof(resolved), resolved, NULL);
    if (len == 0 || len >= sizeof(resolved))
        return NULL;
    return strdup(resolved);
}

const char* hal_path_join(const char *a, const char *b) {
    if (!a || !b)
        return NULL;
    size_t a_len = strlen(a);
    size_t b_len = strlen(b);
    if (!a_len || !b_len)
        return NULL;
    
    /* Remove trailing separator from a if present */
    while (a_len > 0 && (a[a_len-1] == '\\' || a[a_len-1] == '/'))
        a_len--;
    /* Remove leading separator from b if present */
    while (b_len > 0 && (*b == '\\' || *b == '/')) {
        b++;
        b_len--;
    }
    
    size_t total_len = a_len + 1 + b_len;
    char *result = (char*)malloc(total_len + 1);
    if (!result)
        return NULL;
    memcpy(result, a, a_len);
    result[a_len] = HAL_PATH_SEPARATOR;
    memcpy(result + a_len + 1, b, b_len);
    result[total_len] = '\0';
    return result;
}

const char* hal_path_join_va(int n, ...) {
    if (n <= 0)
        return NULL;
    va_list args;
    va_start(args, n);
    size_t total_length = 0;
    for (int i = 0; i < n; i++) {
        const char *part = va_arg(args, const char*);
        if (part) {
            total_length += strlen(part);
            if (i < n - 1)
                total_length += 1;
        }
    }
    va_end(args);
    if (total_length == 0)
        return NULL;
    char *result = (char*)malloc(total_length + 1);
    if (!result)
        return NULL;

    va_start(args, n);
    size_t pos = 0;
    for (int i = 0; i < n; i++) {
        const char *part = va_arg(args, const char*);
        if (part) {
            size_t part_length = strlen(part);
            memcpy(result + pos, part, part_length);
            pos += part_length;
            if (i < n - 1)
                result[pos++] = HAL_PATH_SEPARATOR;
        }
    }
    va_end(args);
    result[pos] = '\0';
    return result;
}

const char** hal_path_split(const char *path, size_t *count) {
    const char **parts = NULL;
    int parts_count = 0;
    const char *start = path;
    int index = 0;

    if (!path)
        goto BAIL;
    for (const char *p = path; *p; p++)
        if (*p == '\\' || *p == '/') {
            if (p > start)
                parts_count++;
            start = p + 1;
        }
    if (start < path + strlen(path))
        parts_count++;
    if (parts_count == 0)
        goto BAIL;

    if (!(parts = (const char**)malloc(parts_count * sizeof(char*))))
        goto BAIL;
    start = path;
    for (const char *p = path; *p; p++)
        if (*p == '\\' || *p == '/') {
            if (p > start) {
                size_t length = p - start;
                parts[index] = (char*)malloc(length + 1);
                if (!parts[index])
                    goto DEAD;
                strncpy((char*)parts[index], start, length);
                ((char*)parts[index])[length] = '\0';
                index++;
            }
            start = p + 1;
        }
    if (start < path + strlen(path)) {
        size_t length = path + strlen(path) - start;
        parts[index] = (char*)malloc(length + 1);
        if (!parts[index])
            goto DEAD;
        strncpy((char*)parts[index], start, length);
        ((char*)parts[index])[length] = '\0';
        index++;
    }
    goto BAIL;
DEAD:
    if (parts) {
        for (int i = 0; i < index; i++)
            if (parts[i])
                free((void*)parts[i]);
        free(parts);
    }
    parts = NULL;
    parts_count = -1;
BAIL:
    if (count)
        *count = parts_count;
    return parts;
}

/* Glob functions */
static int simple_match(const char *pat, const char *s) {
    if (!pat)
        return s && *s == '\0';
    if (*pat == '\0')
        return *s == '\0';
    if (*pat == '*') {
        while (*(pat + 1) == '*')
            pat++;
        if (*(pat+1) == '\0') return 1;
        for (const char *p = s; *p; ++p)
            if (simple_match(pat + 1, p))
                return 1;
        return 0;
    } else if (*pat == '?') {
        return *s && simple_match(pat+1, s+1);
    } else {
        if (*pat == *s)
            return simple_match(pat + 1, s + 1);
        return 0;
    }
}

struct collect_ctx {
    char **out;
    int n;
    int cap;
};

static void collect_add(struct collect_ctx *c, const char *full) {
    if (c->n >= c->cap) {
        int newcap = c->cap == 0 ? 8 : c->cap * 2;
        char **tmp = (char **)realloc(c->out, newcap * sizeof(char*));
        if (!tmp)
            return;
        c->out = tmp;
        c->cap = newcap;
    }
    c->out[c->n++] = strdup(full);
}

static void walk_and_match(const char *base, const char *suffix_pat, struct collect_ctx *c) {
    char pattern[HAL_MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", base);
    
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        char full[HAL_MAX_PATH];
        snprintf(full, sizeof(full), "%s\\%s", base, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            walk_and_match(full, suffix_pat, c);
        else if (simple_match(suffix_pat, fd.cFileName))
            collect_add(c, full);
    } while (FindNextFileA(hFind, &fd));
    
    FindClose(hFind);
}

const char** hal_directory_glob(const char *pattern, int *count) {
    if (!pattern) return NULL;
    struct collect_ctx c = {0};
    const char *ddstar = strstr(pattern, "**");
    if (ddstar) {
        const char *slash_before = ddstar;
        while (slash_before > pattern && *slash_before != '\\' && *slash_before != '/')
            --slash_before;
        size_t base_len = 0;
        const char *suffix_pat = NULL;
        if (*slash_before == '\\' || *slash_before == '/') {
            base_len = slash_before - pattern;
            suffix_pat = ddstar + 2;
            if (*suffix_pat == '\\' || *suffix_pat == '/')
                suffix_pat++;
        } else {
            base_len = 0;
            suffix_pat = ddstar + 2;
            if (*suffix_pat == '\\' || *suffix_pat == '/')
                suffix_pat++;
        }
        char base[HAL_MAX_PATH];
        if (base_len == 0)
            strcpy(base, ".");
        else {
            strncpy(base, pattern, base_len);
            base[base_len] = '\0';
        }
        walk_and_match(base, suffix_pat, &c);
    } else {
        const char *last = strrchr(pattern, '\\');
        const char *flast = strrchr(pattern, '/');
        if (flast && (!last || flast > last))
            last = flast;
        char dir[HAL_MAX_PATH];
        const char *filepat = NULL;
        if (last) {
            size_t dlen = last - pattern;
            if (dlen == 0)
                strcpy(dir, "\\");
            else {
                strncpy(dir, pattern, dlen);
                dir[dlen] = '\0';
            }
            filepat = last + 1;
        } else { strcpy(dir, "."); filepat = pattern; }
        
        char search[HAL_MAX_PATH];
        snprintf(search, sizeof(search), "%s\\*", dir);
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(search, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
                char full[HAL_MAX_PATH];
                snprintf(full, sizeof(full), "%s\\%s", dir, fd.cFileName);
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    if (simple_match(filepat, fd.cFileName))
                        collect_add(&c, full);
                } else {
                    walk_and_match(full, filepat, &c);
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
    if (count)
        *count = c.n;
    if (c.n == 0)
        return NULL;
    char **out = (char **)realloc(c.out, c.n * sizeof(char *));
    if (out)
        c.out = out;
    return (const char**)c.out;
}

bool hal_path_glob(const char *pattern, hal_glob_callback callback, void *userdata) {
    if (!pattern || !callback)
        return false;
    
    int count = 0;
    const char **matches = hal_directory_glob(pattern, &count);
    if (!matches)
        return true;
    
    bool result = true;
    for (int i = 0; i < count; i++) {
        const char *filename = hal_path_filename(matches[i]);
        const char *dirname = hal_path_dirname(matches[i]);
        if (callback(dirname ? dirname : ".", filename, userdata) != 0) {
            result = false;
        }
        if (dirname)
            free((void*)dirname);
        free((void*)matches[i]);
    }
    free(matches);
    return result;
}

bool hal_path_walk(const char *path, bool recursive, hal_walk_callback callback, void *userdata) {
    if (!path || !hal_directory_exists(path) || !callback)
        return false;
    
    hal_dir_t d = {.path = path};
    const char *name = NULL;
    bool is_dir = false;
    
    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        const char *full_path = hal_path_join(path, name);
        if (!full_path)
            continue;
        
        if (is_dir) {
            if (recursive) {
                if (!hal_path_walk(full_path, recursive, callback, userdata)) {
                    free((void*)full_path);
                    hal_directory_iter_end(&d);
                    return false;
                }
            }
        } else {
            if (callback(path, name, userdata) != 0) {
                free((void*)full_path);
                hal_directory_iter_end(&d);
                return false;
            }
        }
        free((void*)full_path);
    }
    return true;
}

#endif /* HAL_NO_FILESYSTEM */
