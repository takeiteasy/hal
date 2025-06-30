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

#ifndef HAL_NO_FILE_SYSTEM
#include "../file_system.h"
#include <unistd.h>
#include "../shared/file_system.c"

bool hal_file_exists(const char *path) {
    return false;
}

bool hal_dir_exists(const char *path) {
    return false;
}

const char* hal_get_root_dir(void) {
    return strdup("/");
}

const char* hal_working_dir(void) {
    return NULL;
}

const char* hal_home_dir(void) {
    return NULL;
}

const char* hal_documents_dir(void) {
    return NULL;
}

const char* hal_downloads_dir(void) {
    return NULL;
}

const char* hal_video_dir(void) {
    return NULL;
}

const char* hal_music_dir(void) {
    return NULL;
}

const char* hal_picture_dir(void) {
    return NULL;
}

const char* hal_application_dir(void) {
    return NULL;
}

const char* hal_desktop_dir(void) {
    return NULL;
}
#endif // HAL_NO_FILE_SYSTEM
