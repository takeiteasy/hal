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

// Platform-independent path utilities

#ifndef HAL_NO_PATH_UTILS
#include "hal/path_utils.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#define IS_SEP(c) ((c) == '/' || (c) == '\\')
#else
#define IS_SEP(c) ((c) == '/')
#endif

const char* hal_path_extension(const char *path) {
    if (!path) return NULL;
    const char *last_dot = NULL;
    const char *last_sep = NULL;
    
    for (const char *p = path; *p; p++) {
        if (*p == '.') last_dot = p;
        if (IS_SEP(*p)) last_sep = p;
    }
    
    if (!last_dot) return NULL;
    if (last_sep && last_dot < last_sep) return NULL;
    if (last_dot == path || (last_sep && last_dot == last_sep + 1)) return NULL;
    
    return last_dot + 1;
}

const char* hal_path_filename(const char *path) {
    if (!path) return NULL;
    const char *last_sep = NULL;
    
    for (const char *p = path; *p; p++) {
        if (IS_SEP(*p)) last_sep = p;
    }
    
    return last_sep ? last_sep + 1 : path;
}

char* hal_path_basename(const char *path) {
    const char *filename = hal_path_filename(path);
    if (!filename) return NULL;
    
    const char *ext = hal_path_extension(path);
    size_t len;
    if (ext && ext > filename)
        len = (ext - 1) - filename; // -1 for the dot
    else
        len = strlen(filename);
    
    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, filename, len);
    result[len] = '\0';
    return result;
}

char* hal_path_dirname(const char *path) {
    if (!path) return NULL;
    const char *last_sep = NULL;
    
    for (const char *p = path; *p; p++) {
        if (IS_SEP(*p)) last_sep = p;
    }
    
    if (!last_sep) {
        char *result = (char*)malloc(2);
        if (result) { result[0] = '.'; result[1] = '\0'; }
        return result;
    }
    
    size_t len = last_sep - path;
    if (len == 0) len = 1; // Root directory
    
    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, path, len);
    result[len] = '\0';
    return result;
}

char* hal_path_parent(const char *path) {
    return hal_path_dirname(path);
}

char* hal_path_join(const char *a, const char *b) {
    if (!a && !b) return NULL;
    if (!a) return strdup(b);
    if (!b) return strdup(a);
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    bool need_sep = len_a > 0 && !IS_SEP(a[len_a - 1]) && !IS_SEP(b[0]);
    
    size_t total = len_a + len_b + (need_sep ? 1 : 0) + 1;
    char *result = (char*)malloc(total);
    if (!result) return NULL;
    
    memcpy(result, a, len_a);
    if (need_sep) {
#if defined(_WIN32) || defined(_WIN64)
        result[len_a] = '\\';
#else
        result[len_a] = '/';
#endif
        memcpy(result + len_a + 1, b, len_b + 1);
    } else {
        memcpy(result + len_a, b, len_b + 1);
    }
    return result;
}

char* hal_path_normalize(const char *path) {
    if (!path) return NULL;
    
    size_t len = strlen(path);
    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;
    
    size_t j = 0;
    bool last_was_sep = false;
    
    for (size_t i = 0; i < len; i++) {
        if (IS_SEP(path[i])) {
            if (!last_was_sep) {
#if defined(_WIN32) || defined(_WIN64)
                result[j++] = '\\';
#else
                result[j++] = '/';
#endif
                last_was_sep = true;
            }
        } else {
            result[j++] = path[i];
            last_was_sep = false;
        }
    }
    result[j] = '\0';
    
    // Remove trailing separator (unless it's the root)
    if (j > 1 && IS_SEP(result[j-1]))
        result[j-1] = '\0';
    
    return result;
}

bool hal_path_is_absolute(const char *path) {
    if (!path || !*path) return false;
#if defined(_WIN32) || defined(_WIN64)
    // Windows: C:\ or \\ or /
    if (path[0] && path[1] == ':') return true;
    if (IS_SEP(path[0])) return true;
#else
    if (path[0] == '/') return true;
#endif
    return false;
}

char** hal_path_split(const char *path, size_t *count) {
    if (!path || !count) return NULL;
    
    *count = 0;
    size_t capacity = 8;
    char **parts = (char**)malloc(capacity * sizeof(char*));
    if (!parts) return NULL;
    
    const char *start = path;
    const char *p = path;
    
    while (*p) {
        if (IS_SEP(*p)) {
            if (p > start) {
                size_t len = p - start;
                if (*count >= capacity) {
                    capacity *= 2;
                    char **new_parts = (char**)realloc(parts, capacity * sizeof(char*));
                    if (!new_parts) { hal_path_free_split(parts, *count); return NULL; }
                    parts = new_parts;
                }
                parts[*count] = (char*)malloc(len + 1);
                if (!parts[*count]) { hal_path_free_split(parts, *count); return NULL; }
                memcpy(parts[*count], start, len);
                parts[*count][len] = '\0';
                (*count)++;
            }
            start = p + 1;
        }
        p++;
    }
    
    if (p > start) {
        size_t len = p - start;
        if (*count >= capacity) {
            capacity *= 2;
            char **new_parts = (char**)realloc(parts, capacity * sizeof(char*));
            if (!new_parts) { hal_path_free_split(parts, *count); return NULL; }
            parts = new_parts;
        }
        parts[*count] = (char*)malloc(len + 1);
        if (!parts[*count]) { hal_path_free_split(parts, *count); return NULL; }
        memcpy(parts[*count], start, len);
        parts[*count][len] = '\0';
        (*count)++;
    }
    
    return parts;
}

void hal_path_free_split(char **parts, size_t count) {
    if (!parts) return;
    for (size_t i = 0; i < count; i++)
        free(parts[i]);
    free(parts);
}
#endif
