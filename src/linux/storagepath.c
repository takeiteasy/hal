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

// Linux storagepath using XDG directories

#ifndef HAL_NO_STORAGEPATH
#include "hal/storagepath.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static char home_buf[1024];
static char documents_buf[1024];
static char downloads_buf[1024];
static char desktop_buf[1024];
static char pictures_buf[1024];
static char music_buf[1024];
static char videos_buf[1024];
static char appdata_buf[1024];
static char cache_buf[1024];
static char temp_buf[1024];

static const char* get_home(void) {
    char *home = getenv("HOME");
    if (home) return home;
    struct passwd *pw = getpwuid(getuid());
    if (pw) return pw->pw_dir;
    return NULL;
}

static const char* get_xdg_path(const char *xdg_var, const char *fallback, char *buf) {
    char *path = getenv(xdg_var);
    if (path && path[0]) {
        strncpy(buf, path, 1023);
        buf[1023] = '\0';
        return buf;
    }
    const char *home = get_home();
    if (home && fallback) {
        snprintf(buf, 1024, "%s/%s", home, fallback);
        return buf;
    }
    return NULL;
}

bool hal_storagepath_available(void) { return true; }

const char* hal_path_home(void) {
    const char *home = get_home();
    if (home) {
        strncpy(home_buf, home, 1023);
        home_buf[1023] = '\0';
        return home_buf;
    }
    return NULL;
}

const char* hal_path_documents(void) {
    return get_xdg_path("XDG_DOCUMENTS_DIR", "Documents", documents_buf);
}

const char* hal_path_downloads(void) {
    return get_xdg_path("XDG_DOWNLOAD_DIR", "Downloads", downloads_buf);
}

const char* hal_path_desktop(void) {
    return get_xdg_path("XDG_DESKTOP_DIR", "Desktop", desktop_buf);
}

const char* hal_path_pictures(void) {
    return get_xdg_path("XDG_PICTURES_DIR", "Pictures", pictures_buf);
}

const char* hal_path_music(void) {
    return get_xdg_path("XDG_MUSIC_DIR", "Music", music_buf);
}

const char* hal_path_videos(void) {
    return get_xdg_path("XDG_VIDEOS_DIR", "Videos", videos_buf);
}

const char* hal_path_appdata(void) {
    return get_xdg_path("XDG_DATA_HOME", ".local/share", appdata_buf);
}

const char* hal_path_cache(void) {
    return get_xdg_path("XDG_CACHE_HOME", ".cache", cache_buf);
}

const char* hal_path_temp(void) {
    char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = getenv("TMP");
    if (!tmp) tmp = getenv("TEMP");
    if (tmp) {
        strncpy(temp_buf, tmp, 1023);
        temp_buf[1023] = '\0';
        return temp_buf;
    }
    return "/tmp";
}
#endif
