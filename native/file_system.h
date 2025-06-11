/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

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

#ifndef PAUL_FILE_SYSTEM_HEAD
#define PAUL_FILE_SYSTEM_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define PAUL_ONLY_FILE_SYSTEM
#include "../paul.h"
#include <stdarg.h>
#include <string.h>

bool paul_path_exists(const char *path);
bool paul_file_exists(const char *path);
bool paul_dir_exists(const char *path);
bool paul_dir_create(const char *path);
// WARNING: This must be released
const char* paul_read_file(const char *path, size_t *file_size);
bool paul_write_file(const char *path, const char *data, size_t data_size, bool overwrite_existing);
size_t paul_file_mod_time(const char *path);
bool paul_set_working_dir(const char *path);

// WARNING: The following must be released
const char* paul_get_root_dir(void);
const char* paul_working_dir(void);
const char* paul_home_dir(void);
const char* paul_documents_dir(void);
const char* paul_downloads_dir(void);
const char* paul_video_dir(void);
const char* paul_music_dir(void);
const char* paul_picture_dir(void);
const char* paul_application_dir(void);
const char* paul_desktop_dir(void);

const char* paul_file_extension(const char *path);
size_t paul_file_size(const char *path);
const char* paul_file_name(const char *path);
// WARNING: The following must be released
const char* paul_file_name_no_extension(const char *path);
const char* paul_path_directory(const char *path);
const char* paul_path_parent(const char *path);
const char* paul_resolve_path(const char *path);
const char* paul_join_path(const char *a, const char *b);
const char* paul_join_paths(int n, ...);

#ifdef __cplusplus
}
#endif
#endif // PAUL_FILE_SYSTEM_HEAD
