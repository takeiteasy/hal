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

#ifndef HAL_FILE_SYSTEM_HEAD
#define HAL_FILE_SYSTEM_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_FILE_SYSTEM
#include "../hal.h"
#include <stdarg.h>
#include <string.h>

bool hal_path_exists(const char *path);
bool hal_file_exists(const char *path);
bool hal_dir_exists(const char *path);
bool hal_dir_create(const char *path);
// WARNING: This must be released
const char* hal_read_file(const char *path, size_t *file_size);
bool hal_write_file(const char *path, const char *data, size_t data_size, bool overwrite_existing);
size_t hal_file_mod_time(const char *path);
bool hal_set_working_dir(const char *path);

// WARNING: The following must be released
const char* hal_get_root_dir(void);
const char* hal_working_dir(void);
const char* hal_home_dir(void);
const char* hal_documents_dir(void);
const char* hal_downloads_dir(void);
const char* hal_video_dir(void);
const char* hal_music_dir(void);
const char* hal_picture_dir(void);
const char* hal_application_dir(void);
const char* hal_desktop_dir(void);

const char* hal_file_extension(const char *path);
size_t hal_file_size(const char *path);
const char* hal_file_name(const char *path);
// WARNING: The following must be released
const char* hal_file_name_no_extension(const char *path);
const char* hal_path_directory(const char *path);
const char* hal_path_parent(const char *path);
const char* hal_resolve_path(const char *path);
const char* hal_join_path(const char *a, const char *b);
const char* hal_join_paths(int n, ...);

#ifdef __cplusplus
}
#endif
#endif // HAL_FILE_SYSTEM_HEAD
