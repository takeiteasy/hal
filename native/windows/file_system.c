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

#ifndef PAUL_NO_FILE_SYSTEM
#include "../file_system.h"
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#define chdir _chdir
#include "../shared/file_system.c"

bool paul_file_exists(const char *path) {
    return false;
}

bool paul_dir_exists(const char *path) {
    return false;
}

const char* paul_get_root_dir(void) {
    return NULL;
}

const char* paul_working_dir(void) {
    return NULL;
}

const char* paul_home_dir(void) {
    return NULL;
}

const char* paul_documents_dir(void) {
    return NULL;
}

const char* paul_downloads_dir(void) {
    return NULL;
}

const char* paul_video_dir(void) {
    return NULL;
}

const char* paul_music_dir(void) {
    return NULL;
}

const char* paul_picture_dir(void) {
    return NULL;
}

const char* paul_application_dir(void) {
    return NULL;
}

const char* paul_desktop_dir(void) {
    return NULL;
}
#endif // PAUL_NO_FILE_SYSTEM
