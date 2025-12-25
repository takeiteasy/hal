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

// Windows storagepath using SHGetKnownFolderPath

#ifndef HAL_NO_STORAGEPATH
#include "hal/storagepath.h"
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>

static char home_buf[MAX_PATH];
static char documents_buf[MAX_PATH];
static char downloads_buf[MAX_PATH];
static char desktop_buf[MAX_PATH];
static char pictures_buf[MAX_PATH];
static char music_buf[MAX_PATH];
static char videos_buf[MAX_PATH];
static char appdata_buf[MAX_PATH];
static char cache_buf[MAX_PATH];
static char temp_buf[MAX_PATH];

static const char* get_known_folder(const KNOWNFOLDERID *folderId, char *buf) {
    PWSTR path = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, NULL, &path))) {
        WideCharToMultiByte(CP_UTF8, 0, path, -1, buf, MAX_PATH, NULL, NULL);
        CoTaskMemFree(path);
        return buf;
    }
    return NULL;
}

bool hal_storagepath_available(void) { return true; }

const char* hal_path_home(void) {
    return get_known_folder(&FOLDERID_Profile, home_buf);
}

const char* hal_path_documents(void) {
    return get_known_folder(&FOLDERID_Documents, documents_buf);
}

const char* hal_path_downloads(void) {
    return get_known_folder(&FOLDERID_Downloads, downloads_buf);
}

const char* hal_path_desktop(void) {
    return get_known_folder(&FOLDERID_Desktop, desktop_buf);
}

const char* hal_path_pictures(void) {
    return get_known_folder(&FOLDERID_Pictures, pictures_buf);
}

const char* hal_path_music(void) {
    return get_known_folder(&FOLDERID_Music, music_buf);
}

const char* hal_path_videos(void) {
    return get_known_folder(&FOLDERID_Videos, videos_buf);
}

const char* hal_path_appdata(void) {
    return get_known_folder(&FOLDERID_RoamingAppData, appdata_buf);
}

const char* hal_path_cache(void) {
    return get_known_folder(&FOLDERID_LocalAppData, cache_buf);
}

const char* hal_path_temp(void) {
    GetTempPathA(MAX_PATH, temp_buf);
    return temp_buf;
}
#endif
