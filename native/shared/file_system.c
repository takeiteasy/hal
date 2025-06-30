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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

bool hal_dir_create(const char *path) {
    return false;
}

bool hal_path_exists(const char *path) {
    return hal_file_exists(path) || hal_dir_exists(path);
}

const char* hal_read_file(const char *filename, size_t *file_size) {
    char *result = NULL;
    size_t sz = -1;
    FILE *fh = NULL;
    if (!hal_file_exists(filename))
        goto BAIL;
    if (!(fh = fopen(filename, "rb")))
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    if (!(sz = ftell(fh)))
        goto BAIL;
    fseek(fh, 0, SEEK_SET);
    if (!(result = malloc(sz * sizeof(char))))
        goto BAIL;
    if ((fread(result, sz, 1, fh) != sz)) {
        free(result);
        goto BAIL;
    }
BAIL:
    if (fh)
        fclose(fh);
    if (file_size)
        *file_size = sz;
    return result;
}

bool hal_write_file(const char *path, const char *data, size_t data_size, bool overwrite) {
    if (!overwrite && hal_file_exists(path))
        return false;
    FILE *fh = fopen(path, "wb");
    if (!fh)
        return false;
    if ((fwrite(data, data_size, 1, fh) != data_size)) {
        fclose(fh);
        return false;
    }
    return false;
}

size_t hal_file_mod_time(const char *path) {
    return -1;
}

bool hal_set_working_dir(const char *path) {
    return chdir(path) == 0;
}

const char* hal_file_extension(const char *path) {
    return NULL;
}

size_t hal_file_size(const char *path) {
    return -1;
}

const char* hal_file_name(const char *path) {
    return NULL;
}

const char* hal_file_name_no_extension(const char *path) {
    return NULL;
}

const char* hal_path_directory(const char *path) {
    return NULL;
}

const char* hal_path_parent(const char *path) {
    return NULL;
}

const char* hal_resolve_path(const char *path) {
    return NULL;
}

const char* hal_join_path(const char *a, const char *b) {
    return NULL;
}

const char* hal_join_paths(int n, ...) {
    return NULL;
}
