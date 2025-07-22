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

#ifndef HAL_NO_STORAGE_PATH
#include "../storage_path.h"
#include <stdio.h>
#include <unistd.h>
#import <Cocoa/Cocoa.h>

bool hal_path_exists(const char *path) {
    return [[NSFileManager defaultManager] fileExistsAtPath:@(path)];
}

bool hal_file_exists(const char *path) {
    bool is_dir = false;
    bool exists = [[NSFileManager defaultManager] fileExistsAtPath:@(path)
                                                       isDirectory:&is_dir];
    return exists && !is_dir;
}

bool hal_dir_exists(const char *path) {
    bool is_dir = false;
    [[NSFileManager defaultManager] fileExistsAtPath:@(path)
                                         isDirectory:&is_dir];
    return is_dir;
}

const char* hal_get_working_dir(void) {
    static char buffer[MAX_PATH] = {'\0'};
    if (buffer[0] == '\0')
        if (!getcwd(buffer, MAX_PATH))
            return NULL;
    return buffer;
}

bool hal_set_working_dir(const char *path) {
    return chdir(path) == 0;
}

// WARNING: These must be released
const char* hal_working_dir(void) {
    NSString *url = [[NSFileManager defaultManager] currentDirectoryPath];
    const char *result = strdup([url UTF8String]);
    [url release];
    return result;
}

const char* hal_home_dir(void) {
    NSURL *url = [[NSFileManager defaultManager] homeDirectoryForCurrentUser];
    const char *result = strdup([[url absoluteString] UTF8String]);
    [url release];
    return result;
}

const char* hal_get_root_dir(void) {
    return strdup("/");
}

static const char* _storage_path(NSSearchPathDirectory dir) {
    NSURL *url = [[[NSFileManager defaultManager] URLsForDirectory:dir
                                                         inDomains:1] firstObject];
    const char *result = strdup([[url absoluteString] UTF8String]);
    [url release];
    return result;
}

const char* hal_documents_dir(void) {
    return _storage_path(NSDocumentDirectory);
}

const char* hal_downloads_dir(void) {
    return _storage_path(NSDownloadsDirectory);
}

const char* hal_video_dir(void) {
    return _storage_path(NSMoviesDirectory);
}

const char* hal_music_dir(void) {
    return _storage_path(NSMusicDirectory);
}

const char* hal_picture_dir(void) {
    return _storage_path(NSPicturesDirectory);
}

const char* hal_application_dir(void) {
    return _storage_path(NSApplicationDirectory);
}

const char* hal_desktop_dir(void) {
    return _storage_path(NSDesktopDirectory);
}

#endif // HAL_NO_STORAGE_PATH
