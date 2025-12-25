/* https://github.com/takeiteasy/hal
   POSIX filesystem implementation */

#ifndef HAL_NO_FILESYSTEM
#include "hal/filesystem.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

/* Constants */
const hal_file_t hal_io_invalid = {.fd=HAL_INVALID_FILE_HANDLE};
const hal_file_t hal_io_out = {.fd=HAL_IO_OUT};
const hal_file_t hal_io_err = {.fd=HAL_IO_ERR};
const hal_file_t hal_io_in  = {.fd=HAL_IO_IN};

/* hal_io_* functions */
bool hal_io_open(hal_file_t *dst, const char* path, hal_file_mode_t mode) {
    if (!dst || !path)
        return false;
    int flags = 0;
    mode_t file_permissions = 0644;
    if (mode & HAL_FILE_READ)
        flags |= mode & (HAL_FILE_WRITE | HAL_FILE_APPEND) ? O_RDWR : O_RDONLY;
    else if (mode & (HAL_FILE_WRITE | HAL_FILE_APPEND))
        flags |= O_WRONLY;
    else
        return false;

    if (mode & (HAL_FILE_WRITE | HAL_FILE_APPEND))
        flags |= O_CREAT;
    if (mode & HAL_FILE_WRITE && !(mode & HAL_FILE_APPEND))
        flags |= O_TRUNC;
    if (mode & HAL_FILE_APPEND)
        flags |= O_APPEND;
    if ((dst->fd = open(path, flags, file_permissions)) == HAL_INVALID_FILE_HANDLE)
        return false;
    return true;
}

bool hal_io_close(hal_file_t *file) {
    if (!file || !hal_io_valid(*file))
        return false;
    bool result = close(file->fd) == 0;
    file->fd = HAL_INVALID_FILE_HANDLE;
    return result;
}

size_t hal_io_read(hal_file_t file, void *buffer, size_t size) {
    if (!hal_io_valid(file) || !buffer || size <= 0)
        return 0;
    return read(file.fd, buffer, size);
}

size_t hal_io_write(hal_file_t file, const void *buffer, size_t size) {
    if (!hal_io_valid(file) || !buffer || size <= 0)
        return 0;
    return write(file.fd, buffer, size);
}

bool hal_io_seek(hal_file_t file, long offset, hal_file_seek_t whence) {
    if (!hal_io_valid(file))
        return false;
    return lseek(file.fd, offset, whence) != -1;
}

bool hal_io_advance(hal_file_t file, long offset) {
    return hal_io_seek(file, offset, HAL_FILE_CURSOR);
}

size_t hal_io_tell(hal_file_t file) {
    if (!hal_io_valid(file))
        return -1;
    return (size_t)lseek(file.fd, 0, HAL_FILE_CURSOR);
}

bool hal_io_eof(hal_file_t file) {
    if (!hal_io_valid(file))
        return true;
    off_t current_pos = lseek(file.fd, 0, HAL_FILE_CURSOR);
    if (current_pos == -1)
        return true;
    off_t end_pos = lseek(file.fd, 0, HAL_FILE_FINISH);
    if (end_pos == -1)
        return true;
    lseek(file.fd, current_pos, HAL_FILE_START);
    return current_pos >= end_pos;
}

bool hal_io_flush(hal_file_t file) {
    if (!hal_io_valid(file))
        return false;
    return fsync(file.fd) == 0;
}

bool hal_io_valid(hal_file_t file) {
    return file.fd >= 0;
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
    return ftruncate(file.fd, size) == 0;
}

/* hal_file_* functions */
bool hal_file_exists(const char *path) {
    if (!path)
        return false;
    struct stat st;
    if (stat(path, &st) == -1)
        return false;
    return S_ISREG(st.st_mode);
}

bool hal_file_delete(const char *path) {
    if (!path || !hal_file_exists(path))
        return false;
    return unlink(path) == 0;
}

bool hal_file_rename(const char *old_path, const char *new_path, bool write_over) {
    if (!old_path || !hal_file_exists(old_path) || !new_path || (!write_over && hal_file_exists(new_path)))
        return false;
    if (link(old_path, new_path) == 0)
        return unlink(old_path) == 0;
    return false;
}

int hal_file_size(const char *path) {
    if (!path)
        return -1;
    struct stat st;
    if (stat(path, &st) == -1)
        return -1;
    return (int)st.st_size;
}

bool hal_file_copy(const char *src_path, const char *dst_path, bool write_over) {
    if (!src_path || !dst_path || !hal_file_exists(src_path) || (hal_path_exists(dst_path) && !write_over))
        return false;

    static const size_t buffer_size = 4096;
    hal_file_t src_fd = {-1}, dst_fd = {-1};
    ssize_t bytes_read, bytes_written;
    char buffer[4096];
    bool result = false;

    if (!hal_io_open(&src_fd, src_path, HAL_FILE_READ))
        goto BAIL;
    if (!hal_io_open(&dst_fd, dst_path, HAL_FILE_WRITE))
        goto BAIL;

    while ((bytes_read = hal_io_read(src_fd, buffer, buffer_size)) > 0)
        if ((bytes_written = hal_io_write(dst_fd, buffer, bytes_read)) != bytes_read)
            goto BAIL;
    if (bytes_read == -1)
        goto BAIL;
    result = true;

BAIL:
    if (hal_io_valid(src_fd))
        hal_io_close(&src_fd);
    if (hal_io_valid(dst_fd))
        hal_io_close(&dst_fd);
    return result;
}

const char* hal_file_read(const char *path, size_t *size) {
    char *result = NULL;
    long _size = -1;
    hal_file_t file = hal_io_invalid;
    if (!path || !hal_file_exists(path) || !hal_io_open(&file, path, HAL_FILE_READ))
        goto BAIL;
    hal_io_seek(file, 0, HAL_FILE_START);
    hal_io_seek(file, 0, HAL_FILE_FINISH);
    if ((_size = hal_io_tell(file)) == -1)
        goto BAIL;
    hal_io_seek(file, 0, HAL_FILE_START);
    if (!(result = (char*)malloc(_size * sizeof(char))))
        goto BAIL;
    if (!hal_io_read(file, result, _size)) {
        free(result);
        goto BAIL;
    }
BAIL:
    if (hal_io_valid(file))
        hal_io_close(&file);
    if (size)
        *size = _size;
    return result;
}

/* hal_directory_* functions */
bool hal_directory_exists(const char *path) {
    if (!path)
        return false;
    struct stat st;
    if (stat(path, &st) == -1)
        return false;
    return S_ISDIR(st.st_mode);
}

bool hal_directory_create(const char *path, bool recursive) {
    if (!path)
        return false;
    if (hal_directory_exists(path))
        return true;
    if (recursive) {
        const char *resolved = hal_path_resolve(path);
        if (!resolved)
            return false;
        while (true) {
            const char *parent = hal_path_parent(resolved);
            if (!parent) {
                free((void*)resolved);
                return false;
            }
            if (hal_directory_exists(resolved)) {
                free((void*)resolved);
                free((void*)parent);
                return true;
            } else {
                if (mkdir(resolved, 0755) != 0) {
                    free((void*)resolved);
                    free((void*)parent);
                    return false;
                }
            }
            free((void*)resolved);
            resolved = parent;
        }
        free((void*)resolved);
        return true;
    } else
        return mkdir(path, 0755) == 0;
}

bool hal_directory_delete(const char *path, bool recursive, bool and_files) {
    if (!path || !hal_directory_exists(path))
        return false;

    if (recursive) {
        hal_dir_t d = {.path=path};
        const char *name = NULL;
        bool is_dir = false;
        while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
            const char *full_path = hal_path_join(path, name);
            if (is_dir) {
                if (!hal_directory_delete(full_path, true, and_files)) {
                    hal_directory_iter_end(&d);
                    free((void*)full_path);
                    return false;
                }
            } else {
                if (!hal_file_delete(full_path)) {
                    hal_directory_iter_end(&d);
                    free((void*)full_path);
                    return false;
                }
            }
            free((void *)full_path);
        }
    } else {
        if (and_files) {
            hal_dir_t d = {.path=path};
            const char *name = NULL;
            bool is_dir = false;
            const char **files = (const char**)malloc(0);
            size_t files_size = 0;
            if (!files)
                return false;
            while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
                if (is_dir) {
                    for (int i = 0; i < (int)files_size; i++)
                        free((void *)files[i]);
                    free((void*)files);
                    hal_directory_iter_end(&d);
                    return false;
                }
                const char *full_path = hal_path_join(path, name);
                if (!full_path) {
                    hal_directory_iter_end(&d);
                    return false;
                }
                if (!(files = (const char**)realloc(files, ++files_size * sizeof(char *)))) {
                    free((void *)full_path);
                    hal_directory_iter_end(&d);
                    return false;
                }
                files[files_size - 1] = full_path;
            }
            bool success = true;
            if (files_size > 0) {
                for (size_t i = 0; i < files_size; i++) {
                    if (!hal_file_delete(files[i])) {
                        success = false;
                        break;
                    }
                }
            }
            for (size_t i = 0; i < files_size; i++)
                free((void *)files[i]);
            free((void *)files);
            if (!success)
                return false;
        }
    }
    return rmdir(path) == 0;
}

bool hal_directory_rename(const char *old_path, const char *new_path, bool write_over) {
    return hal_directory_copy(old_path, new_path, write_over, true);
}

bool hal_directory_copy(const char *src_path, const char *dst_path, bool write_over, bool delete_src) {
    if (!src_path || !hal_directory_exists(src_path) || !dst_path)
        return false;

    bool result = false;
    const char *parent = hal_path_parent(dst_path);
    const char *dir_name = NULL, *dst = NULL;
    hal_dir_t d = {.path=src_path};
    const char *name = NULL;
    bool is_dir = false;
    if (!parent || !hal_directory_exists(parent))
        goto BAIL;
    if (!(dir_name = hal_path_dirname(src_path)))
        goto BAIL;
    if (!(dst = hal_path_join(dst_path, dir_name)) || !hal_directory_create(dst, false))
        goto BAIL;

    while ((name = hal_directory_iter(&d, &is_dir)) != NULL) {
        const char *src_full = hal_path_join(src_path, name);
        const char *dst_full = hal_path_join(dst_path, name);
        bool die = false;
        if (is_dir)
            if (hal_directory_create(dst_full, false)) {
                if (!hal_directory_copy(src_full, dst_full, write_over, delete_src))
                    die = true;
            } else
                die = true;
        else
            if (hal_file_copy(src_full, dst_full, write_over))
                if (delete_src)
                    hal_file_delete(src_full);
        free((void *)src_full);
        free((void *)dst_full);
        if (die) {
            hal_directory_iter_end(&d);
            goto BAIL;
        }
    }

    if (delete_src)
        hal_directory_delete(src_path, true, true);
    result = true;
BAIL:
    if (parent)
        free((void*)parent);
    if (dir_name)
        free((void*)dir_name);
    if (dst)
        free((void*)dst);
    return result;
}

int hal_directory_size(const char *path) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path=path};
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
        free((void *)full_path);
    }
    return total;
}

int hal_directory_item_count(const char *path, bool recursive) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path=path};
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
            total += hal_directory_file_count(full_path, true);
            free((void *)full_path);
        }
    }
    return total;
}

int hal_directory_file_count(const char *path, bool recursive) {
    if (!path || !hal_directory_exists(path))
        return -1;
    hal_dir_t d = {.path=path};
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
        } else
            total++;
    }
    return total;
}

/* directory iteration */
const char* hal_directory_iter(hal_dir_t *dir, bool *is_dir) {
    if (!dir || !dir->path)
        return NULL;
    
    if (!dir->dir) {
        dir->dir = opendir(dir->path);
        if (!dir->dir)
            return NULL;
    }
    
    struct dirent *entry;
    while ((entry = readdir((DIR*)dir->dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (is_dir)
            *is_dir = (entry->d_type == DT_DIR);
        return entry->d_name;
    }
    
    closedir((DIR*)dir->dir);
    dir->dir = NULL;
    return NULL;
}

void hal_directory_iter_end(hal_dir_t *dir) {
    if (dir && dir->dir) {
        closedir((DIR*)dir->dir);
        dir->dir = NULL;
    }
}

/* hal_path_* functions */
bool hal_path_exists(const char *path) {
    if (!path)
        return false;
    struct stat st;
    return stat(path, &st) == 0;
}

const char* hal_path_cwd(void) {
    static char cwd[HAL_MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)))
        return cwd;
    return NULL;
}

bool hal_path_set_cwd(const char *path) {
    if (!path)
        return false;
    return chdir(path) == 0;
}

const char* hal_path_root(void) {
    return "/";
}

const char* hal_path_extension(const char *path) {
    if (!path)
        return NULL;
    char *dot = strrchr((char*)path, '.');
    return dot ? dot + 1 : NULL;
}

const char* hal_path_filename(const char *path) {
    if (!path)
        return NULL;
    const char *last_slash = strrchr(path, HAL_PATH_SEPARATOR);
    return last_slash ? last_slash + 1 : path;
}

const char* hal_path_filename_no_ext(const char *path) {
    if (!path)
        return NULL;
    const char *file_name = hal_path_filename(path);
    if (!file_name)
        return NULL;
    size_t length = strlen(file_name);
    char *dot = strrchr((char*)file_name, '.');
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
    const char *last_slash = strrchr(path, HAL_PATH_SEPARATOR);
    if (!last_slash)
        return NULL;
    size_t length = last_slash - path;
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
    const char *last_slash = strrchr(path, HAL_PATH_SEPARATOR);
    if (!last_slash || last_slash == path)
        return hal_path_root();
    size_t length = last_slash - path;
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
    if (realpath(path, resolved))
        return strdup(resolved);
    return NULL;
}

const char* hal_path_join(const char *a, const char *b) {
    if (!a || !b)
        return NULL;
    size_t a_len = strlen(a);
    size_t b_len = strlen(b);
    if (!a_len || !b_len)
        return NULL;
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
        if (*p == HAL_PATH_SEPARATOR) {
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
        if (*p == HAL_PATH_SEPARATOR) {
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
    DIR *d = opendir(base);
    if (!d)
        return;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", base, ent->d_name);
        struct stat st;
        if (stat(full, &st) == -1)
            continue;
        if (S_ISDIR(st.st_mode))
            walk_and_match(full, suffix_pat, c);
        else if (S_ISREG(st.st_mode) && simple_match(suffix_pat, ent->d_name))
            collect_add(c, full);
    }
    closedir(d);
}

const char** hal_directory_glob(const char *pattern, int *count) {
    if (!pattern) return NULL;
    struct collect_ctx c = {0};
    const char *ddstar = strstr(pattern, "**");
    if (ddstar) {
        const char *slash_before = ddstar;
        while (slash_before > pattern && *slash_before != '/')
            --slash_before;
        size_t base_len = 0;
        const char *suffix_pat = NULL;
        if (*slash_before == '/') {
            base_len = slash_before - pattern;
            suffix_pat = ddstar + 2;
            if (*suffix_pat == '/')
                suffix_pat++;
        } else {
            base_len = 0;
            suffix_pat = ddstar + 2;
            if (*suffix_pat == '/')
                suffix_pat++;
        }
        char base[4096];
        if (base_len == 0)
            strcpy(base, ".");
        else {
            strncpy(base, pattern, base_len);
            base[base_len] = '\0';
        }
        walk_and_match(base, suffix_pat, &c);
    } else {
        const char *last = strrchr(pattern, '/');
        char dir[4096];
        const char *filepat = NULL;
        if (last) {
            size_t dlen = last - pattern;
            if (dlen == 0)
                strcpy(dir, "/");
            else {
                strncpy(dir, pattern, dlen);
                dir[dlen] = '\0';
            }
            filepat = last + 1;
        } else { strcpy(dir, "."); filepat = pattern; }
        DIR *d = opendir(dir);
        if (d) {
            struct dirent *ent;
            while ((ent = readdir(d)) != NULL) {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
                char full[4096];
                snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
                struct stat st;
                if (stat(full, &st) == -1)
                    continue;
                if (S_ISREG(st.st_mode)) {
                    if (simple_match(filepat, ent->d_name))
                        collect_add(&c, full);
                } else if (S_ISDIR(st.st_mode))
                    walk_and_match(full, filepat, &c);
            }
            closedir(d);
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

/* hal_path_glob and hal_path_walk */
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
