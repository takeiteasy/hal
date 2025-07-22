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

#ifndef HAL_STORAGE_PATH_HEAD
#define HAL_STORAGE_PATH_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_STORAGE_PATH
#include "../hal.h"
#include <stdarg.h>
#include <string.h>

#if defined(PLATFORM_MACOS)
#define MAX_PATH 255
#elif defined(PLATFORM_WINDOWS)
#define MAX_PATH 256
#elif defined(PLATFORM_LINUX)
#define MAX_PATH 4096
#else
#define MAX_PATH 1024
#endif

#ifdef PLATFORM_WINDOWS
#define PATH_SEPERATOR '\\'
#define PATH_SEPERATOR_STR "\\"
#else
#define PATH_SEPERATOR '/'
#define PATH_SEPERATOR_STR "/"
#endif

bool hal_path_exists(const char *path);
bool hal_file_exists(const char *path);
bool hal_dir_exists(const char *path);
const char* hal_get_working_dir(void);
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

#ifdef __cplusplus
}
#endif
#endif // HAL_STORAGE_PATH_HEAD
